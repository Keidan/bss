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
#include "bss_utils.h"
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

static htable_t tsnd = NULL;
static htable_t trcv = NULL;
static FILE* dump = NULL;
static _Bool simul_mode = 0;
static FILE* simul = NULL;
static sr_t isr = NULL;
static sr_t osr = NULL;
static _Bool raw = 0;
static _Bool hexa = 0;
static _Bool snd = 0;
static _Bool rcv = 0;
static stringbuffer_t cmd = NULL;
uint32_t sframe = 0;
uint32_t rframe = 0;

static const struct option long_options[] = { 
    { "help"   , 0, NULL, 'h' },
    { "input"  , 1, NULL, 'i' },
    { "output" , 1, NULL, 'o' },
    { "dump"   , 1, NULL, 'd' },
    { "raw"    , 0, NULL, 'r' },
    { "command", 1, NULL, 'c' },
    { "hexa"   , 0, NULL, '0' },
    { "simul"  , 1, NULL, 's' },
    { "snd"    , 0, NULL, '1' },
    { "rcv"    , 0, NULL, '2' },
    { NULL     , 0, NULL, 0   } 
};

  
static void bss_cleanup(void);
static void bss_signals(int sig);
/**
 * @fn static void bss_sr_read(sr_t sr, unsigned char* buffer, uint32_t length)
 * @brief Serial callback.
 * @param sr Serial instance.
 * @param buffer input buffer.
 * @param length input length.
 */
static void bss_sr_read(sr_t sr, unsigned char* buffer, uint32_t length);


void usage(int err) {
  fprintf(stdout, "usage: bss options\n");
  fprintf(stdout, "\t--help, -h: Print this help.\n");
  fprintf(stdout, "\t--input, -i: Input mode and config: dev=device:b=baud:d=data_bits:s=stop_bits:c=flowcontrol:p=parity:v=vmin:t=vtime\n");
  fprintf(stdout, "\t\t dev: serial device (eg: dev=/dev/ttyS0).\n");
  fprintf(stdout, "\t\t b: Nb bauds (eg: b=9600).\n");
  fprintf(stdout, "\t\t d: Data bits, possible values: 5, 6, 7 or 8 (eg: d=8).\n");
  fprintf(stdout, "\t\t s: Stop bits, possible values: 1 or 2 (eg: s=1).\n");
  fprintf(stdout, "\t\t c: Flow control, possible values: none, xonxoff or rtscts (eg: c:none).\n");
  fprintf(stdout, "\t\t p: Parity, possible values: none,odd or even (eg: p=none).\n");
  fprintf(stdout, "\t\t v: vmin, 0 <= value <= 254.\n");
  fprintf(stdout, "\t\t t: vtime, 0 <= value <= 254 (in 0.1 sec).\n");
  fprintf(stdout, "\t--output, -o: This mode reinjects all input data into a new device.\n");
  fprintf(stdout, "\t\tSee the input mode description for the format pattern.\n");
  fprintf(stdout, "\t--dump, -d: Dump input datas in a scpecified file (eg: -d file).\n");
  fprintf(stdout, "\t--raw, -r: Dump all datas in raw mode.\n");
  fprintf(stdout, "\t--command, -c: Input command.\n");
  fprintf(stdout, "\t--hexa: Input command in hexa (by 2, eg for -c '00 00 10').\n");
  fprintf(stdout, "\t--simu, -s: Simulation file (use --hexa for hexa datas).\n");
  fprintf(stdout, "\t\tFile format (sender frame AND receiver frame)\n");
  fprintf(stdout, "\t\t%s\\n\n", SND_TAG);
  fprintf(stdout, "\t\tbinary or hexa (xx[space]xx...) datas\n");
  fprintf(stdout, "\t\t%s\\n\n", RCV_TAG);
  fprintf(stdout, "\t\tbinary or hexa (xx[space]xx...) datas\n");
  fprintf(stdout, "\t--snd: Sender mode (see --simul,-s).\n");    
  fprintf(stdout, "\t--rcv: Receiver mode (see --simul,-s).\n");
  exit(err);
}


int main(int argc, char** argv) {

  fprintf(stdout, "Basic serial sniffer is a FREE software v%d.%d.\nCopyright 2013 By kei\nLicense GPL.\n\n", BSS_VERSION_MAJOR, BSS_VERSION_MINOR);

  ssig_init(log_init_cast_user("bss", LOG_PID|LOG_CONS|LOG_PERROR), bss_cleanup);
  ssig_add_signal(SIGINT, bss_signals);
  ssig_add_signal(SIGTERM, bss_signals);

  cmd = stringbuffer_new();

  int opt;
  while ((opt = getopt_long(argc, argv, "hi:o:d:rc:0s:12", long_options, NULL)) != -1) {
    switch (opt) {
      case 'h': usage(0); break;
      case 'i': /* input */
	if((isr = sr_open_from_string(optarg)) == NULL) {
	  logger(LOG_ERR, "Unable to open the input serial (see the log for more details)!\n");
	  usage(EXIT_FAILURE);
	}
	break;
      case 'o': /* output */
	if((osr = sr_open_from_string(optarg)) == NULL) {
	  logger(LOG_ERR, "Unable to open the output serial (see the log for more details)!\n");
	  usage(EXIT_FAILURE);
	}
	break;
      case 'd': /* dump */
	dump = fopen(optarg, "w+");
	if(!dump) {
	  logger(LOG_ERR, "Unable to open file '%s': (%d) %s\n", optarg, errno, strerror(errno));
	  usage(EXIT_FAILURE);
	}
	break;
      case '0': /* hexa */
	hexa = 1;
	break;
      case 'c': /* command */
	stringbuffer_copy(cmd, optarg);
	break;
      case 'r': /* raw */
	raw = 1;
	break;
      case 's': /* simul */
	simul = fopen(optarg, "r");
	if(!simul) {
	  logger(LOG_ERR, "Unable to open file '%s': (%d) %s\n", optarg, errno, strerror(errno));
	  usage(EXIT_FAILURE);
	}
	simul_mode = 1;
	break;
      case '1':
	snd = 1;
	break;
      case '2':
	rcv = 1;
	break;
      default: /* '?' */
	logger(LOG_ERR, "Unknown option '%c'\n", opt);
	usage(EXIT_FAILURE);
	break;
    }
  }

  if(!isr) {
    logger(LOG_ERR, "Input mode is mandatory!\n");
    usage(EXIT_FAILURE);
  }
  if(simul_mode && rcv && snd) {
    logger(LOG_ERR, "Invalid simul mode snd = 1 AND rcv = 1!");
    usage(EXIT_FAILURE);
  }else if(simul_mode && !rcv && !snd) {
    logger(LOG_ERR, "Invalid simul mode snd = 0 AND rcv = 0!");
    usage(EXIT_FAILURE);
  } else if(simul_mode && osr) {
    logger(LOG_ERR, "Invalid simul mode AND output mode!");
    usage(EXIT_FAILURE);
  } else if(simul_mode && stringbuffer_length(cmd)) {
    logger(LOG_ERR, "Invalid simul mode AND non null command!");
    usage(EXIT_FAILURE);
  }

  string_t buf;
  sr_get_info(isr, buf);
  logger(LOG_INFO, "%s\n", buf);

  sr_start_read(isr, bss_sr_read);

  if(stringbuffer_length(cmd)) {
    if(!hexa)
      sr_write(isr, (unsigned char*)stringbuffer_to_str(cmd), stringbuffer_length(cmd));
    else
      bss_urils_send_frame(isr, stringbuffer_to_str(cmd));
  } else if(simul_mode) {
    bss_utils_parse_simul(&simul, &tsnd, &trcv);
    if(snd)
      bss_utils_send_table_frame(isr, tsnd, &sframe);
  }

  while(1) sleep(1);
  return EXIT_SUCCESS;
}


/**
 * @fn static void bss_sr_read(sr_t sr, unsigned char* buffer, uint32_t length)
 * @brief Serial callback.
 * @param sr Serial instance.
 * @param buffer input buffer.
 * @param length input length.
 */
static void bss_sr_read(sr_t sr, unsigned char* buffer, uint32_t length) {
  printf("Buffer size: %d\n", length);
  ntools_print_hex(dump == NULL ? stdout : dump, buffer, length, raw);
  if(simul_mode) {
    if(snd)
      bss_utils_send_table_frame(isr, tsnd, &sframe);
    else
      bss_utils_send_table_frame(isr, trcv, &rframe);
  } else {
    /* forward this data */
    if(osr) sr_write(osr, buffer, length);
  }
}

static void bss_signals(int sig) {
  if(sig == SIGINT)
    printf("\n"); // for ^C
  exit(0);
}

static void bss_cleanup(void) {
  if(dump) fclose(dump), dump = NULL;
  if(osr) sr_close(osr), osr = NULL;
  if(isr) sr_close(isr), isr = NULL;
  if(simul) fclose(simul), simul = NULL;
  if(cmd) stringbuffer_delete(cmd), cmd = NULL;
  if(tsnd) htable_delete(tsnd), tsnd = NULL;
  if(trcv) htable_delete(trcv), trcv = NULL;
}

