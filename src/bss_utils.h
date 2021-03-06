/**
 *******************************************************************************
 * @file bss_utils.h
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
#ifndef __BSS_UTILS_H__
  #define __BSS_UTILS_H__

  #include <stdio.h>
  #include <stdlib.h>
  #include <tk/utils/string.h>
  #include <tk/utils/stringbuffer.h>
  #include <tk/utils/stringtoken.h>
  #include <tk/io/sr.h>
  #include <tk/sys/log.h>
  #include <tk/io/net/nettools.h>
  #include <tk/sys/syssig.h>
  #include <tk/utils/htable.h>


  #define SND_TAG "--[SND]--"
  #define RCV_TAG "--[RCV]--"

  struct bss_frames_counter_s {
      uint32_t max_rcv;
      uint32_t max_snd;
      uint32_t cur_rcv;
      uint32_t cur_snd;
  };

  /**
   * @fn unsigned char* bss_utils_hex_to_buffer(const stringbuffer_t input)
   * @brief Convert input hex string in binary datas.
   * @param input The input string.
   * @param alen Allocated lenth.
   * @return The binary datas (free required).
   */
  unsigned char* bss_utils_hex_to_buffer(const stringbuffer_t input, uint32_t *alen);

  /**
   * @fn unsigned char* bss_utils_hex_to_buffer_c(const char* input)
   * @brief Convert input hex string in binary datas.
   * @param input The input string.
   * @param alen Allocated lenth.
   * @return The binary datas (free required).
   */
  unsigned char* bss_utils_hex_to_buffer_c(const char* input, uint32_t *alen);

  /**
   * @fn void bss_utils_parse_simul(FILE** simul, htable_t *tsnd, htable_t *trcv, struct bss_frames_counter_s* counter)
   * @brief Parse the silul file.
   * @param simul The simul file.
   * @param tsnd SND table.
   * @param trcv RCV table.
   * @param counter Frames counter.
   */
  void bss_utils_parse_simul(FILE** simul, htable_t *tsnd, htable_t *trcv, struct bss_frames_counter_s* counter);

  /**
   * @fn void bss_send_first_frame(sr_t isr, htable_t t, uint32_t *tidx)
   * @brief Send the first frame.
   * @param isr Serial pointer.
   * @param t table.
   * @param tidx Table key.
   */
  void bss_utils_send_table_frame(sr_t isr, htable_t t, uint32_t *tidx);

  /**
   * @fn void bss_utils_send_frame(sr_t isr, char* cmd)
   * @brief Send the first frame.
   * @param isr Serial pointer.
   * @param cmd The command.
   */
  void bss_utils_send_frame(sr_t isr, char* cmd);

#endif /* __BSS_UTILS_H__ */
