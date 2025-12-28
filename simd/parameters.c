#include "parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include <libconfig.h>

#include <regex.h>
#if defined(HAVE_ARGTABLE3)
#include <argtable3.h>
#elif defined(HAVE_ARGTABLE2)
#include <argtable2.h>
#else
#error "No argtable implementation available"
#endif


ConfigError getParameters(int argc, char** argv, Parameters* p)
{

    ConfigError exitcode = E_SOMETHING_BAD;

    p->verbosity_count           = 0;

    p->daemon                    = true;
    p->memmap                    = true;
    p->bridge                    = true;
    p->notify                    = true;
    p->udp                       = false;
    p->poke                      = false;
    p->targetval                 = false;

    // setup argument handling structures
    const char* progname = "simd";

    struct arg_lit* arg_verbosity    = arg_litn("v","verbose", 0, 2, "increase logging verbosity");

    struct arg_lit* arg_nodaemon     = arg_lit0("n", "nodaemon", "no daemon mode");
    struct arg_lit* arg_nomemmap     = arg_lit0("h", "nomemmap", "no automatic memory mapping for supported sim games");
    struct arg_lit* arg_nobridge     = arg_lit0("a", "nobridge", "no automatic memory bridging for supported sim games");
    struct arg_lit* arg_nonotify     = arg_lit0("s", "nonotify", "no desktop notifications");
    struct arg_str* arg_poke         = arg_str0("p", "poke", "<string>", "poke simdata");
    struct arg_str* arg_target       = arg_str0("t", "target", "<string>", "target value ofpoke simdata");

    struct arg_lit* arg_udp          = arg_lit0("u", "udp", "force udp on all sims which support udp sufficiently");
    struct arg_lit* help             = arg_litn(NULL,"help", 0, 1, "print this help and exit");
    struct arg_lit* vers             = arg_litn(NULL,"version", 0, 1, "print version information and exit");
    struct arg_end* end              = arg_end(20);
    void* argtable0[]                = {arg_nomemmap,arg_nodaemon,arg_nobridge,arg_nonotify,arg_poke,arg_target,arg_udp,arg_verbosity,help,vers,end};
    int nerrors0;

    if (arg_nullcheck(argtable0) != 0)
    {
        printf("%s: insufficient memory\n",progname);
        goto cleanup;
    }

    nerrors0 = arg_parse(argc,argv,argtable0);

    if (nerrors0==0)
    {
        p->verbosity_count = arg_verbosity->count;
        p->daemon_count = arg_nodaemon->count;
        p->notify_count = arg_nonotify->count;
        p->udp_count = arg_udp->count;

        if (arg_nodaemon->count > 0)
        {
            p->daemon = false;
        }
        if (arg_nomemmap->count > 0)
        {
            p->memmap = false;
        }
        if (arg_nonotify->count > 0)
        {
            p->notify = false;
        }
        if (arg_nobridge->count > 0)
        {
            p->bridge = false;
        }
        if (arg_udp->count > 0)
        {
            p->udp = true;
        }

        if(arg_poke->count > 0)
        {
            p->pokesetting = strdup(arg_poke->sval[0]);
            p->poke = true;
        }
        if(arg_target->count > 0)
        {
            p->targetvalue = strdup(arg_target->sval[0]);
            p->targetval = true;
        }

        exitcode = E_SUCCESS_AND_DO;
    }

    if (help->count > 0)
    {
        printf("Usage: %s\n", progname);
        arg_print_syntax(stdout,argtable0,"\n");
        printf("\nReport bugs on the github github.com/spacefreak18/simapi.\n");
        exitcode = E_SUCCESS_AND_EXIT;
        goto cleanup;
    }

    if (vers->count > 0)
    {
        printf("%s Simulator Data Mapping Daemon\n",progname);
        printf("October 2024, Paul Dino Jones\n");
        exitcode = E_SUCCESS_AND_EXIT;
        goto cleanup;
    }

cleanup:
    arg_freetable(argtable0,sizeof(argtable0)/sizeof(argtable0[0]));
    return exitcode;
}
