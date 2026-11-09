#ifndef MQTT_TELEMETRY_H
#define MQTT_TELEMETRY_H

#include <stdbool.h>
#include <libconfig.h>
#include <simdata.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MQTTContext MQTTContext;

/*
 * Reads mqtt broker settings and mqtt.fields once, then uses mosquitto_connect().
 * cfg may be destroyed after this function returns.
 * A non-NULL return means the TCP connection was established; call mqtt_poll()
 * regularly to process CONNACK and maintain network traffic.
 * mqtt_open() may block during the initial TCP connection.
 */
MQTTContext *mqtt_open(config_t *cfg, const char *client_id);

/* Process MQTT network traffic without waiting. Call on your libuv loop
 * (e.g. every 50-100 ms). Returns a MOSQ_ERR_* code.
 * This simple version does not automatically reconnect. */
int mqtt_poll(MQTTContext *ctx);

/* True only after a successful broker CONNACK. */
bool mqtt_is_connected(const MQTTContext *ctx);

/*
 * Returns a MOSQ_ERR_* code (MOSQ_ERR_SUCCESS on locally accepted publish).
 * QoS 0, non-retained. No packet is buffered by this wrapper while offline.
 * Pass topic=NULL to use mqtt.topic from the configuration.
 * Call all mqtt_* functions on the same event-loop thread.
 */
int mqtt_publish_packet(MQTTContext *ctx, const SimData *packet);

/* Disconnect and release the context. */
void mqtt_close(MQTTContext *ctx);

#ifdef __cplusplus
}
#endif

#endif
