/* 
# libmpeg2ts, a MPEG2 transport stream reading/writing library
# 
# Copyright (C) 2012 Alexander Izvorski <aizvorski@gmail.com>
# 
# This file is part of libmpeg2ts.
# 
# libmpeg2ts is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# libmpeg2ts is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with libmpeg2ts.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _MPEG2TS_H
#define _MPEG2TS_H 1

#include <stdint.h>

#include "bs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int adaptation_field_length;
        int discontinuity_indicator;
        int random_access_indicator;
        int elementary_stream_priority_indicator;
        int PCR_flag;
        int OPCR_flag;
        int splicing_point_flag;
        int transport_private_data_flag;
        int adaptation_field_extension_flag;
            int program_clock_reference_base;
            int program_clock_reference_extension;
            int original_program_clock_reference_base;
            int original_program_clock_reference_extension;
            int splice_countdown;
            int transport_private_data_length;
            uint8_t* private_data;
            int private_data_len;
            int adaptation_field_extension_length;
            int ltw_flag;
            int piecewise_rate_flag;
            int seamless_splice_flag;
                int ltw_valid_flag;
                int ltw_offset;
                int piecewise_rate;
                int splice_type;
                int DTS_next_AU;
} adaptation_field_t;

typedef struct {
    int transport_error_indicator;
    int payload_unit_start_indicator;
    int transport_priority;
    int PID;
    int transport_scrambling_control;
    int adaptation_field_control;
    int continuity_counter;
        adaptation_field_t* adaptation_field;
        uint8_t* data;
        int data_len;
} transport_packet_t;

#define pes_stream_id_program_stream_map 0
#define pes_stream_id_padding_stream 0
#define pes_stream_id_private_stream_2 0
#define pes_stream_id_ECM 0
#define pes_stream_id_EMM 0
#define pes_stream_id_program_stream_directory 0
#define pes_stream_id_DSMCC_stream 0
#define pes_stream_id_H_222_1 0
#define trick_mode_control_fast_forward 0
#define trick_mode_control_slow_motion 0
#define trick_mode_control_freeze_frame 0
#define trick_mode_control_fast_reverse 0
#define trick_mode_control_slow_reverse 0
typedef struct {
    int packet_start_code_prefix;
    int stream_id;
    int PES_packet_length;
        int PES_scrambling_control;
        int PES_priority;
        int data_alignment_indicator;
        int copyright;
        int original_or_copy;
        int PTS_DTS_flags;
        int ESCR_flag;
        int ES_rate_flag;
        int DSM_trick_mode_flag;
        int additional_copy_info_flag;
        int PES_CRC_flag;
        int PES_extension_flag;
        int PES_header_data_length;
            int PTS;
            int DTS;
            int ESCR_base;
            int ESCR_extension;
            int ES_rate;
            int trick_mode_control;
                int field_id;
                int intra_slice_refresh;
                int frequency_truncation;
                int rep_cntrl;
            int additional_copy_info;
            int previous_PES_packet_CRC;
            int PES_private_data_flag;
            int pack_header_field_flag;
            int program_packet_sequence_counter_flag;
            int P_STD_buffer_flag;
            int reserved;
            int PES_extension_flag_2;
                int PES_private_data;
                int pack_field_length;
                uint8_t* pack_header;
                int pack_header_len;
                int program_packet_sequence_counter;
                int MPEG1_MPEG2_identifier;
                int original_stuff_length;
                int P_STD_buffer_scale;
                int P_STD_buffer_size;
                int PES_extension_field_length;
                uint8_t* PES_extension_field;
                int PES_extension_field_len;
            uint8_t* PES_packet_data;
            int PES_packet_data_len;
} PES_packet_t;

int adaptation_field_write(adaptation_field_t* p, bs_t* bs);
int adaptation_field_read(adaptation_field_t* p, bs_t* bs);
int adaptation_field_len(adaptation_field_t* p);
int transport_packet_write(transport_packet_t* p, bs_t* bs);
int transport_packet_read(transport_packet_t* p, bs_t* bs);
int transport_packet_len(transport_packet_t* p);
int PES_packet_write(PES_packet_t* p, bs_t* bs);
int PES_packet_read(PES_packet_t* p, bs_t* bs);
int PES_packet_len(PES_packet_t* p);

#ifdef __cplusplus
}
#endif

#endif
