/**
 *******************************************************************************
 * @file bss.c
 * @author Keidan
 * @date 03/01/2013
 * @par Project
 * bss
 *
 * @par Copyright
 * Copyright 2011-2013 Keidan, all right reserved
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY.
 *
 * Licence summary : 
 *    You can modify and redistribute the sources code and binaries.
 *    You can send me the bug-fix
 *
 * Term of the licence in in the file licence.txt.
 *
 *******************************************************************************
 */
#include "bss_common.h"
#include "bss_config.h"
#include <getopt.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <tk/text/string.h>
#include <tk/text/stringtoken.h>

static FILE* output = NULL;
static FILE* input = NULL;
static int packets = 0;

static const struct option long_options[] = { 
    { "help"   , 0, NULL, 'h' },
    { "iface"  , 1, NULL, '0' },
    { "output" , 1, NULL, '1' },
    { "input"  , 1, NULL, '2' },
    { "filter" , 1, NULL, '3' },
    { "payload", 0, NULL, '4' },
    { "raw"    , 0, NULL, '5' },
    { "size"   , 1, NULL, '6' },
    { "count"  , 1, NULL, '7' },
    { "link"   , 1, NULL, '8' },
    { NULL     , 0, NULL, 0   } 
};

  
#ifndef _POSIX_HOST_NAME_MAX
  #define _POSIX_HOST_NAME_MAX 255
#endif

#define blogger(...) ({				\
    logger(LOG_ERR, __VA_ARGS__);		\
    fprintf(stderr, __VA_ARGS__);		\
  })

static void bss_sig_catch(int s);
static void bss_cleanup(void);

void usage(int err) {
  fprintf(stdout, "usage: bss options\n");
  fprintf(stdout, "\t--help, -h: Print this help.\n");
  exit(err);
}


int main(int argc, char** argv) {
struct sigaction sa;
  fprintf(stdout, "Basic serial sniffer is a FREE software v%d.%d.\nCopyright 2013 By kei\nLicense GPL.\n\n", BSS_VERSION_MAJOR, BSS_VERSION_MINOR);


  atexit(bss_cleanup);
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = bss_sig_catch;
  (void)sigaction(SIGINT, &sa, NULL);
  (void)sigaction(SIGTERM, &sa, NULL);

  int opt;
  while ((opt = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {
    switch (opt) {
      case 'h': usage(0); break;
      default: /* '?' */
	blogger("Unknown option '%c'\n", opt);
	usage(EXIT_FAILURE);
	break;
    }
  }

  return ret;
}


static void bss_sig_catch(int s) {
  printf("\n"); /* skip the ^C on the console... */
  exit(0); /* call atexit */
}

static void bss_cleanup(void) {

}
