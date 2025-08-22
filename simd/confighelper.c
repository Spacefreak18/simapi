#include <stdio.h>
#include <stdbool.h>
#include <string.h>


#include "confighelper.h"


//int strcicmp(char const *a, char const *b)
//{
//    for (;; a++, b++) {
//        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
//        if (d != 0 || !*a)
//            return d;
//    }
//}


int getNumberOfConfigs(const char* config_file_str)
{
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, config_file_str))
    {
        config_destroy(&cfg);
        return -1;
    }

    config_setting_t* config = NULL;
    config_setting_t* config_sims = NULL;

    config_sims = config_lookup(&cfg, "sims");
    int configs = config_setting_length(config_sims);

    config_destroy(&cfg);
    return configs;
}


int loadconfig(SimdSettings simds, int compat_info_size, GameCompatInfo* gc)
{
    int numdevices = 0;
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, simds.configfile))
    {
        config_destroy(&cfg);
        return -1;
    }
    else
    {
        config_setting_t* config = NULL;
        config = config_lookup(&cfg, "sims");

        int i = 0;
        while (i<compat_info_size)
        {
            config_setting_t* config_sim = config_setting_get_elem(config, i);

            const char* sim_name = NULL;
            const char* launch_exe = NULL;
            const char* live_exe = NULL;

            // these first two are the two that are required at this point
            int launch_found = 0;
            launch_found = config_setting_lookup_string(config_sim, "launchexe", &launch_exe);

            if(launch_found == 0)
            {
                i++;
                continue;
            }


            int live_found = 0;
            live_found = config_setting_lookup_string(config_sim, "liveexe", &live_exe);

            int name_found = 0;
            name_found = config_setting_lookup_string(config_sim, "name", &sim_name);
            if(name_found == 0)
            {
                sim_name = "NoName";
            }

            int simapi_found = 0;
            int simapi = 0;
            simapi_found = config_setting_lookup_int(config_sim, "simapi", &simapi);

            int gameid_found = 0;
            int gameid = 0;
            gameid_found = config_setting_lookup_int(config_sim, "gameid", &gameid);

            int bridgedelay_found = 0;
            int bridgedelay = 5;
            bridgedelay_found = config_setting_lookup_int(config_sim, "bridgedelay", &bridgedelay);

            gc[i].LaunchExe = strdup(launch_exe);
            gc[i].LiveExe = strdup(live_exe);
            gc[i].Name = strdup(sim_name);
            gc[i].BridgeDelay = bridgedelay;
            gc[i].simapi = simapi;
            gc[i].GameId = gameid;

            i++;
        }
    }


    config_destroy(&cfg);

    return 0;
}
