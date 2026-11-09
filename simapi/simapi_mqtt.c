#define _POSIX_C_SOURCE 200809L

#include "simapi_mqtt.h"

#include <mosquitto.h>
#include <json-c/json.h>

#include <simdata.h>

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    FIELD_DOUBLE,
    FIELD_INT,
    FIELD_BOOL,
    FIELD_STRING
} MQTTFieldType;

typedef struct {
    const char *source;
    size_t offset;
    MQTTFieldType type;
} MQTTFieldDefinition;

typedef struct {
    char *name;                 /* Owned JSON output key */
    size_t offset;             /* Offset into SimData */
    MQTTFieldType type;
} MQTTField;

#define DOUBLE_FIELD(member) { #member, offsetof(SimData, member), FIELD_DOUBLE }
#define INT_FIELD(member)    { #member, offsetof(SimData, member), FIELD_INT }
#define BOOL_FIELD(member)   { #member, offsetof(SimData, member), FIELD_BOOL }
#define STRING_FIELD(member) { #member, offsetof(SimData, member), FIELD_STRING }

static const MQTTFieldDefinition definitions[] = {
    DOUBLE_FIELD(clutch),
    DOUBLE_FIELD(brake),
    //DOUBLE_FIELD(throttle),
    //DOUBLE_FIELD(handbrake),
    //DOUBLE_FIELD(steering_angle),
    DOUBLE_FIELD(rpms),
    INT_FIELD(gear),
    //DOUBLE_FIELD(speed_ms),
    //DOUBLE_FIELD(distance_round_track),
    //DOUBLE_FIELD(world_x),
    //DOUBLE_FIELD(world_y),
    //DOUBLE_FIELD(world_z),
    //INT_FIELD(current_lap),
    //DOUBLE_FIELD(current_lap_time),
    //DOUBLE_FIELD(lap_time_previous),
    //BOOL_FIELD(current_lap_is_valid),
    //BOOL_FIELD(previous_lap_was_valid),
    //STRING_FIELD(car_class)
};

#define DEFINITION_COUNT (sizeof(definitions) / sizeof(definitions[0]))

struct MQTTContext {
    struct mosquitto *mosq;
    bool connected;
    MQTTField *fields;
    size_t field_count;
    char *topic; /* Configured default publishing topic */
};

/* All mqtt_* calls, including open/close, must run on one event-loop thread.
 * Global library init/cleanup are reference-counted for multiple contexts. */
static unsigned int library_users = 0;

static int library_acquire(void)
{
    int rc = MOSQ_ERR_SUCCESS;
    if (library_users == 0)
        rc = mosquitto_lib_init();
    if (rc == MOSQ_ERR_SUCCESS)
        library_users++;
    return rc;
}

static void library_release(void)
{
    if (library_users > 0 && --library_users == 0)
        mosquitto_lib_cleanup();
}

static void free_fields(MQTTContext *ctx)
{
    if (!ctx)
        return;
    for (size_t i = 0; i < ctx->field_count; ++i)
        free(ctx->fields[i].name);
    free(ctx->fields);
    ctx->fields = NULL;
    ctx->field_count = 0;
}

static const MQTTFieldDefinition *find_definition(const char *source)
{
    for (size_t i = 0; i < DEFINITION_COUNT; ++i) {
        if (strcmp(source, definitions[i].source) == 0)
            return &definitions[i];
    }
    return NULL;
}

static int load_fields(MQTTContext *ctx, config_t *cfg)
{
    config_setting_t *entries = config_lookup(cfg, "mqtt.fields");
    if (!entries || !config_setting_is_list(entries)) {
        fprintf(stderr, "MQTT: expected list mqtt.fields in config\n");
        return -1;
    }

    int count = config_setting_length(entries);
    if (count <= 0) {
        fprintf(stderr, "MQTT: mqtt.fields cannot be empty\n");
        return -1;
    }

    MQTTField *fields = calloc((size_t)count, sizeof(*fields));
    if (!fields)
        return -1;

    size_t loaded = 0;
    for (int i = 0; i < count; ++i) {
        config_setting_t *entry = config_setting_get_elem(entries, i);
        const char *source = NULL;
        const char *name = NULL;

        if (!entry ||
            !config_setting_lookup_string(entry, "source", &source) ||
            !config_setting_lookup_string(entry, "name", &name) ||
            !source || !*source || !name || !*name) {
            fprintf(stderr, "MQTT: fields[%d] needs nonempty source and name\n", i);
            goto error;
        }

        const MQTTFieldDefinition *def = find_definition(source);
        if (!def) {
            fprintf(stderr, "MQTT: unknown telemetry source '%s'\n", source);
            goto error;
        }
        for (size_t j = 0; j < loaded; ++j) {
            if (strcmp(fields[j].name, name) == 0) {
                fprintf(stderr, "MQTT: duplicate JSON key '%s'\n", name);
                goto error;
            }
        }

        fields[loaded].name = strdup(name);
        if (!fields[loaded].name)
            goto error;
        fields[loaded].offset = def->offset;
        fields[loaded].type = def->type;
        ++loaded;
    }

    ctx->fields = fields;
    ctx->field_count = loaded;
    return 0;

error:
    for (size_t j = 0; j < loaded; ++j)
        free(fields[j].name);
    free(fields);
    return -1;
}

static struct json_object *packet_to_json(const MQTTContext *ctx, const SimData *packet)
{
    struct json_object *obj = json_object_new_object();
    if (!obj)
        return NULL;

    const unsigned char *base = (const unsigned char *)packet;
    for (size_t i = 0; i < ctx->field_count; ++i) {
        const MQTTField *f = &ctx->fields[i];
        const void *ptr = base + f->offset;
        struct json_object *value = NULL;

        switch (f->type) {
        case FIELD_DOUBLE:
            value = json_object_new_double(*(const double *)ptr);
            break;
        case FIELD_INT:
            value = json_object_new_int(*(const int *)ptr);
            break;
        case FIELD_BOOL:
            value = json_object_new_boolean(*(const bool *)ptr);
            break;
        case FIELD_STRING: {
            const char *str = *(const char *const *)ptr;
            value = json_object_new_string(str ? str : "");
            break;
        }
        }

        if (!value) {
            json_object_put(obj);
            return NULL;
        }
        if (json_object_object_add(obj, f->name, value) != 0) {
            /* json-c takes ownership of value only on success. */
            json_object_put(value);
            json_object_put(obj);
            return NULL;
        }
    }
    return obj;
}

static void on_connect(struct mosquitto *mosq, void *userdata, int rc)
{
    (void)mosq;
    MQTTContext *ctx = userdata;
    ctx->connected = (rc == 0);
    if (rc != 0)
        fprintf(stderr, "MQTT: broker rejected connection: %d\n", rc);
}

static void on_disconnect(struct mosquitto *mosq, void *userdata, int rc)
{
    (void)mosq;
    MQTTContext *ctx = userdata;
    ctx->connected = false;
    if (rc != 0)
        fprintf(stderr, "MQTT: disconnected unexpectedly: %d\n", rc);
}

MQTTContext *mqtt_open(config_t *cfg, const char *client_id)
{
    if (!cfg)
        return NULL;

    const char *host = NULL;
    const char *topic = NULL;
    const char *login = NULL;
    const char *password = NULL;
    const char *ca_file = NULL;
    const char *ca_path = NULL;
    int port = 0;
    int use_tls = 0;

    if (!config_lookup_string(cfg, "mqtt.host", &host) || !host || !*host ||
        !config_lookup_int(cfg, "mqtt.port", &port) ||
        port < 1 || port > 65535 ||
        !config_lookup_string(cfg, "mqtt.topic", &topic) || !topic || !*topic) {
        fprintf(stderr, "MQTT: mqtt.host, mqtt.port and mqtt.topic are required\n");
        return NULL;
    }

    config_lookup_string(cfg, "mqtt.login", &login);
    config_lookup_string(cfg, "mqtt.password", &password);
    config_lookup_bool(cfg, "mqtt.use_tls", &use_tls);
    config_lookup_string(cfg, "mqtt.ca_file", &ca_file);
    config_lookup_string(cfg, "mqtt.ca_path", &ca_path);

    if (password && *password && (!login || !*login)) {
        fprintf(stderr, "MQTT: password requires mqtt.login\n");
        return NULL;
    }
    if (!use_tls && ((ca_file && *ca_file) || (ca_path && *ca_path))) {
        fprintf(stderr, "MQTT: CA options require mqtt.use_tls=true\n");
        return NULL;
    }


    MQTTContext *ctx = calloc(1, sizeof(*ctx));
    if (!ctx)
        return NULL;
    ctx->connected = false;
    ctx->topic = strdup(topic);
    if (!ctx->topic)
        goto error;

    if (load_fields(ctx, cfg) != 0)
        goto error;

    int rc = library_acquire();
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "MQTT: library initialization failed: %s\n",
                mosquitto_strerror(rc));
        goto error;
    }

    ctx->mosq = mosquitto_new(client_id, true, ctx);
    if (!ctx->mosq) {
        fprintf(stderr, "MQTT: failed to allocate Mosquitto client\n");
        goto error_library;
    }

    mosquitto_connect_callback_set(ctx->mosq, on_connect);
    mosquitto_disconnect_callback_set(ctx->mosq, on_disconnect);

    if (login && *login) {
        rc = mosquitto_username_pw_set(ctx->mosq, login,
                                       password ? password : NULL);
        if (rc != MOSQ_ERR_SUCCESS)
            goto error_mosq;
    }

    if (use_tls) {
        /* /etc/ssl/certs is provided by ca-certificates on Debian,
         * Arch and Fedora. Override with mqtt.ca_file or mqtt.ca_path. */
        const char *trusted_path = (ca_path && *ca_path) ? ca_path : NULL;
        const char *trusted_file = (ca_file && *ca_file) ? ca_file : NULL;
        if (!trusted_file && !trusted_path)
            trusted_path = "/etc/ssl/certs";

        rc = mosquitto_tls_set(ctx->mosq, trusted_file, trusted_path,
                               NULL, NULL, NULL);
        if (rc != MOSQ_ERR_SUCCESS)
            goto error_mosq;
        /* Default: validate the broker certificate and hostname. */
    }

    rc = mosquitto_connect(ctx->mosq, host, port, 60);
    if (rc != MOSQ_ERR_SUCCESS)
        goto error_mosq;

    return ctx;

error_mosq:
    fprintf(stderr, "MQTT: startup error: %s\n", mosquitto_strerror(rc));
    mosquitto_destroy(ctx->mosq);
error_library:
    library_release();
error:
    free(ctx->topic);
    free_fields(ctx);
    free(ctx);
    return NULL;
}

bool mqtt_is_connected(const MQTTContext *ctx)
{
    return ctx && ctx->connected;
}

int mqtt_publish_packet(MQTTContext *ctx, const SimData *packet)
{
    if (!ctx || !packet)
        return MOSQ_ERR_INVAL;


    if (!mqtt_is_connected(ctx))
        return MOSQ_ERR_NO_CONN;

    struct json_object *obj = packet_to_json(ctx, packet);

    if (!obj)
        return MOSQ_ERR_NOMEM;

    const char *json = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PLAIN);

    if (!json)
    {
        json_object_put(obj);
        return MOSQ_ERR_NOMEM;
    }

    int rc = mosquitto_publish(ctx->mosq, NULL, ctx->topic, (int)strlen(json), json, 0, false);

    json_object_put(obj);

    return rc;
}


/* Call regularly from the owning libuv loop (e.g. uv_timer_t). */
int mqtt_poll(MQTTContext *ctx)
{
    if (!ctx || !ctx->mosq)
        return MOSQ_ERR_INVAL;

    return mosquitto_loop(ctx->mosq, 0, 1);
}

void mqtt_close(MQTTContext *ctx)
{
    if (!ctx)
        return;

    if (ctx->mosq) {
        mosquitto_disconnect(ctx->mosq);
        mosquitto_destroy(ctx->mosq);
        library_release();
    }

    free_fields(ctx);
    free(ctx->topic);
    free(ctx);
}
