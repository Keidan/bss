/**
 *******************************************************************************
 * @file bss_utils.c
 * @author Keidan
 * @date 22/08/2013
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

#include "bss_utils.h"

/**
 * @fn void bss_urils_send_frame(sr_t isr, char* cmd)
 * @brief Send the first frame.
 * @param isr Serial pointer.
 * @param cmd The command.
 */
void bss_urils_send_frame(sr_t isr, char* cmd) {
  uint32_t n;
  unsigned char* buffer = bss_utils_hex_to_buffer_c(cmd, &n);
  printf("Send trame (%d bytes):\n", n);
  ntools_print_hex(stdout, buffer, n, 0);
  sr_write(isr, buffer, n);
  free(buffer);
}

/**
 * @fn void bss_send_first_frame(sr_t isr, htable_t t, uint32_t *tidx)
 * @brief Send the first frame.
 * @param isr Serial pointer.
 * @param t table.
 * @param tidx Table key.
 */
void bss_utils_send_table_frame(sr_t isr, htable_t t, uint32_t *tidx) {
  printf("tidx:%d\n", *tidx);
  printf("tidx:%d, %s\n", *tidx, string_convert((*tidx), 10));
  char* cmd = (char*)htable_lookup(t, (char*)string_convert(*tidx, 10));
  if(!cmd) {
    logger(LOG_ERR, "Null frame cmd\n");
    return;
  }
  bss_urils_send_frame(isr, cmd);
  (*tidx)++;
}

/**
 * @fn void bss_utils_parse_simul(FILE** simul, htable_t *tsnd, htable_t *trcv)
 * @brief Parse the silul file.
 * @param simul The simul file.
 * @param tsnd SND table.
 * @param trcv RCV table.
 */
void bss_utils_parse_simul(FILE** simul, htable_t *tsnd, htable_t *trcv) {
  _Bool stag = 0, rtag = 0, add = 0;
  char* line = NULL;
  size_t sz = 0, llen, nrframe = 0, nsframe = 0, *idx;
  stringbuffer_t buffer = stringbuffer_new();
  htable_t tmp = NULL;
  if(!*tsnd) {
    *tsnd = htable_new();
    if(!*tsnd) exit(EXIT_FAILURE);
  }
  if(!*trcv) {
    *trcv = htable_new();
    if(!*trcv) exit(EXIT_FAILURE);
  }
  stringbuffer_copy(buffer, "");
  while (getline(&line, &sz, *simul) != -1) {
    llen = strlen(line);
    if(line[llen - 1] == '\n') line[llen - 1] = ' ';
    if(!strcmp(SND_TAG" ", line)) {
      stag = 1;
      rtag = !stag;
      if(stringbuffer_length(buffer)) {
	add=1;
	tmp=*trcv;
	idx=&nrframe;
      } else continue;
    } else if(!strcmp(RCV_TAG" ", line)) {
      rtag = 1;
      stag = !rtag;
      if(stringbuffer_length(buffer)) {
	add=1;
	tmp=*tsnd;
	idx=&nsframe;
      } else continue;
    }
    if(add) {
      add = 0;
      htable_add(tmp, (char*)string_convert(*idx, 10), stringbuffer_to_str(buffer), stringbuffer_length(buffer));
      (*idx)++;
      stringbuffer_clear(buffer);
      continue;
    }
    stringbuffer_append(buffer, line);
  }
  if(stringbuffer_length(buffer)) {
    htable_t tmp = *tsnd;
    uint32_t idx = nsframe;
    if(rtag) {
      tmp = *trcv;
      idx = nrframe;
    }
    htable_add(tmp, (char*)string_convert(idx, 10), stringbuffer_to_str(buffer), stringbuffer_length(buffer));
    idx++;
    stringbuffer_erase2(buffer, 0);
  }
  if(line) free(line);
  fclose(*simul), *simul = NULL;
  stringbuffer_delete(buffer);
}

/**
 * @fn unsigned char* bss_utils_hex_to_buffer(const stringbuffer_t input)
 * @brief Convert input hex string in binary datas.
 * @param input The input string.
 * @param alen Allocated lenth.
 * @return The binary datas (free required).
 */
unsigned char* bss_utils_hex_to_buffer(const stringbuffer_t input, uint32_t *alen) {
  return bss_utils_hex_to_buffer(stringbuffer_to_str(input), alen);
}

/**
 * @fn unsigned char* bss_utils_hex_to_buffer_c(const char* input)
 * @brief Convert input hex string in binary datas.
 * @param input The input string.
 * @param alen Allocated lenth.
 * @return The binary datas (free required).
 */
unsigned char* bss_utils_hex_to_buffer_c(const char* input, uint32_t *alen) {
  int n = 0, len = 1;
  unsigned char *tmp, *decoded = malloc(20);
  *alen = 0;
  if(!decoded) return NULL;
  stringtoken_t tok = stringtoken_init(input, " ");
  while(stringtoken_has_more_tokens(tok)) {
    char* c = stringtoken_next_token(tok);
    if((strlen(c) + n) >= len) {
      len += strlen(c);
      tmp = realloc(decoded, len);
      if(!tmp) {
	free(decoded);
	return NULL;
      }
      decoded = tmp;
    }
    decoded[n++] = (unsigned char)strtol(c, NULL, 16);
  }
  decoded[n] = 0;
  *alen = n;
  stringtoken_release(tok);
  return decoded;
}
