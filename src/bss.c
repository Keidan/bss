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
#include <stdio.h>
#include <stdlib.h>
#include "bss_config.h"
#include <getopt.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <tk/text/string.h>
#include <tk/text/stringtoken.h>
#include <tk/io/sr.h>
#include <tk/sys/log.h>
#include <tk/io/net/netutils.h>

#define MAX_CMD_SIZE 255

static FILE* dump = NULL;
static sr_t isr = NULL;
static sr_t osr = NULL;
static _Bool raw = 0;
static _Bool hexa = 0;
static char cmd[MAX_CMD_SIZE];

static const struct option long_options[] = { 
    { "help"   , 0, NULL, 'h' },
    { "input"  , 1, NULL, 'i' },
    { "output" , 1, NULL, 'o' },
    { "dump"   , 1, NULL, 'd' },
    { "raw"    , 0, NULL, 'r' },
    { "command", 1, NULL, 'c' },
    { "hexa"   , 0, NULL, '0' },
    { NULL     , 0, NULL, 0   } 
};

  
#define blogger(...) ({				\
    logger(LOG_ERR, __VA_ARGS__);		\
    fprintf(stderr, __VA_ARGS__);		\
  })

static void bss_sig_catch(int s);
static void bss_cleanup(void);
static void bss_sr_read(sr_t sr, unsigned char* buffer, uint32_t length);

void usage(int err) {
  fprintf(stdout, "usage: bss options\n");
  fprintf(stdout, "\t--help, -h: Print this help.\n");
  fprintf(stdout, "\t--input, -i: Input mode and config: dev=device:b=baud:d=data_bits:s=stop_bits:c=flowcontrol:p=parity\n");
  fprintf(stdout, "\t\t dev: serial device (eg: dev=/dev/ttyS0).\n");
  fprintf(stdout, "\t\t b: Nb bauds (eg: b=9600).\n");
  fprintf(stdout, "\t\t d: Data bits, possible values: 5, 6, 7 or 8 (eg: d=8).\n");
  fprintf(stdout, "\t\t s: Stop bits, possible values: 1 or 2 (eg: s=1).\n");
  fprintf(stdout, "\t\t c: Flow control, possible values: none, xonxoff or rtscts (eg: c:none).\n");
  fprintf(stdout, "\t\t p: Parity, possible values: none,odd or even (eg: p=none).\n");
  fprintf(stdout, "\t--output, -o: This mode reinjects all input data into a new device.\n");
  fprintf(stdout, "\t\tSee the input mode description for the format pattern.\n");
  fprintf(stdout, "\t--dump, -d: Dump input datas in a scpecified file (eg: -d file).\n");
  fprintf(stdout, "\t--raw, -r: Dump all datas in raw mode.\n");
  fprintf(stdout, "\t--command, -c: Input command.\n");
  fprintf(stdout, "\t--hexa: Input command in hexa (by 2, eg for -c '00 00 10').\n");

  exit(err);
}


int main(int argc, char** argv) {
  struct sigaction sa;

  fprintf(stdout, "Basic serial sniffer is a FREE software v%d.%d.\nCopyright 2013 By kei\nLicense GPL.\n\n", BSS_VERSION_MAJOR, BSS_VERSION_MINOR);

  memset(cmd, 0, MAX_CMD_SIZE);

  atexit(bss_cleanup);
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = bss_sig_catch;
  (void)sigaction(SIGINT, &sa, NULL);
  (void)sigaction(SIGTERM, &sa, NULL);

  int opt;
  while ((opt = getopt_long(argc, argv, "hi:o:d:rc:0", long_options, NULL)) != -1) {
    switch (opt) {
      case 'h': usage(0); break;
      case 'i': /* input */
	if((isr = sr_open_from_string(optarg)) == NULL) {
	  blogger("Unable to open the input serial (see the log for more details)!\n");
	  usage(EXIT_FAILURE);
	}
	break;
      case 'o': /* output */
	if((osr = sr_open_from_string(optarg)) == NULL) {
	  blogger("Unable to open the output serial (see the log for more details)!\n");
	  usage(EXIT_FAILURE);
	}
	break;
      case 'd': /* dump */
	dump = fopen(optarg, "w+");
	if(!dump) {
	  blogger("Unable to open file '%s': (%d) %s\n", optarg, errno, strerror(errno));
	  usage(EXIT_FAILURE);
	}
	break;
      case '0': /* hexa */
	hexa = 1;
	break;
      case 'c': /* command */
	strncpy(cmd, optarg, MAX_CMD_SIZE);
	break;
      case 'r': /* raw */
	raw = 1;
	break;
      default: /* '?' */
	blogger("Unknown option '%c'\n", opt);
	usage(EXIT_FAILURE);
	break;
    }
  }

  if(!isr) {
    blogger("Input mode is mandatory!\n");
    usage(EXIT_FAILURE);
  }
  string_t buf;
  sr_get_info(isr, buf);
  blogger("%s\n", buf);

  sr_start_read(isr, bss_sr_read);

  if(strlen(cmd)) {
    if(!hexa)
      sr_write(isr, cmd, strlen(cmd));
    else {
      int n = 0;
      unsigned char full_cmd[MAX_CMD_SIZE+1];
      bzero(full_cmd, MAX_CMD_SIZE);
      stringtoken_t tok = stringtoken_init(cmd, " ");
      while(stringtoken_has_more_tokens(tok)) {
	char* c = stringtoken_next_token(tok);
	full_cmd[n++] = (unsigned char)strtol(c, NULL, 16);
      }
      printf("Send trame (%d bytes):\n", n);
      netutils_print_hex(stdout, full_cmd, n, 0);
      stringtoken_release(tok);
      sr_write(isr, full_cmd, n);
    }
  }

  while(1) sleep(1);
  return EXIT_SUCCESS;
}


static void bss_sig_catch(int s) {
  printf("\n"); /* skip the ^C on the console... */
  exit(0); /* call atexit */
}

static void bss_cleanup(void) {
  log_close();
  if(dump) fclose(dump), dump = NULL;
  if(osr) sr_close(osr), osr = NULL;
  if(isr) sr_close(isr), isr = NULL;
}

static void bss_sr_read(sr_t sr, unsigned char* buffer, uint32_t length) {
  printf("Buffer size: %d\n", length);
  netutils_print_hex(dump == NULL ? stdout : dump, buffer, length, raw);
  /* forward this data */
  if(osr) sr_write(osr, buffer, length);
}
