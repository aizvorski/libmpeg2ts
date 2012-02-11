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


#include "mpeg2ts.h"

int adaptation_field_read(adaptation_field_t* p, bs_t* bs) {
    p->adaptation_field_length = bs_read_uimsbf(bs, 8);
    if (p->adaptation_field_length > 0) {
        p->discontinuity_indicator = bs_read_bslbf(bs, 1);
        p->random_access_indicator = bs_read_bslbf(bs, 1);
        p->elementary_stream_priority_indicator = bs_read_bslbf(bs, 1);
        p->PCR_flag = bs_read_bslbf(bs, 1);
        p->OPCR_flag = bs_read_bslbf(bs, 1);
        p->splicing_point_flag = bs_read_bslbf(bs, 1);
        p->transport_private_data_flag = bs_read_bslbf(bs, 1);
        p->adaptation_field_extension_flag = bs_read_bslbf(bs, 1);
        if (p->PCR_flag == 1) {
            p->program_clock_reference_base = bs_read_uimsbf(bs, 33);
            bs_skip_bslbf(bs, 6);
            p->program_clock_reference_extension = bs_read_uimsbf(bs, 9);
        };
        if (p->OPCR_flag == 1) {
            p->original_program_clock_reference_base = bs_read_uimsbf(bs, 33);
            bs_skip_bslbf(bs, 6);
            p->original_program_clock_reference_extension = bs_read_uimsbf(bs, 9);
        };
        if (p->splicing_point_flag == 1) {
            p->splice_countdown = bs_read_tcimsbf(bs, 8);
        };
        if (p->transport_private_data_flag == 1) {
            p->transport_private_data_length = bs_read_uimsbf(bs, 8);
            p->private_data_len = bs_read_bytes(bs, p->private_data, p->transport_private_data_length);
        };
        if (p->adaptation_field_extension_flag == 1) {
            p->adaptation_field_extension_length = bs_read_uimsbf(bs, 8);
            p->ltw_flag = bs_read_bslbf(bs, 1);
            p->piecewise_rate_flag = bs_read_bslbf(bs, 1);
            p->seamless_splice_flag = bs_read_bslbf(bs, 1);
            bs_skip_bslbf(bs, 5);
            if (p->ltw_flag == 1) {
                p->ltw_valid_flag = bs_read_bslbf(bs, 1);
                p->ltw_offset = bs_read_uimsbf(bs, 15);
            };
            if (p->piecewise_rate_flag == 1) {
                bs_skip_bslbf(bs, 2);
                p->piecewise_rate = bs_read_uimsbf(bs, 22);
            };
            if (p->seamless_splice_flag == 1) {
                p->splice_type = bs_read_bslbf(bs, 4);
                p->DTS_next_AU = bs_read_timestamp_with_markers(bs, 36);
            };
            bs_skip_bytes_all(bs);
        };
        bs_skip_bytes_all(bs);
    };
};

int transport_packet_read(transport_packet_t* p, bs_t* bs) {
    bs_skip_bslbf(bs, 8);
    p->transport_error_indicator = bs_read_bslbf(bs, 1);
    p->payload_unit_start_indicator = bs_read_bslbf(bs, 1);
    p->transport_priority = bs_read_bslbf(bs, 1);
    p->PID = bs_read_uimsbf(bs, 13);
    p->transport_scrambling_control = bs_read_bslbf(bs, 2);
    p->adaptation_field_control = bs_read_bslbf(bs, 2);
    p->continuity_counter = bs_read_uimsbf(bs, 4);
    if (p->adaptation_field_control == 0b10 || p->adaptation_field_control ==  0b11) {
        adaptation_field_read(p->adaptation_field, bs);
    };
    if (p->adaptation_field_control == 0b01 || p->adaptation_field_control ==  0b11) {
        p->data_len = bs_read_bytes_all(bs, p->data);
    };
};

int PES_packet_read(PES_packet_t* p, bs_t* bs) {
    p->packet_start_code_prefix = bs_read_bslbf(bs, 24);
    p->stream_id = bs_read_uimsbf(bs, 8);
    p->PES_packet_length = bs_read_uimsbf(bs, 16);
    if (p->stream_id != pes_stream_id_program_stream_map && p->stream_id !=                                            pes_stream_id_padding_stream && p->stream_id !=                                            pes_stream_id_private_stream_2 && p->stream_id !=                                            pes_stream_id_ECM && p->stream_id !=                                            pes_stream_id_EMM && p->stream_id !=                                            pes_stream_id_program_stream_directory && p->stream_id !=                                            pes_stream_id_DSMCC_stream && p->stream_id !=                                           pes_stream_id_H_222_1) {
        bs_skip_bslbf(bs, 2);
        p->PES_scrambling_control = bs_read_bslbf(bs, 2);
        p->PES_priority = bs_read_bslbf(bs, 1);
        p->data_alignment_indicator = bs_read_bslbf(bs, 1);
        p->copyright = bs_read_bslbf(bs, 1);
        p->original_or_copy = bs_read_bslbf(bs, 1);
        p->PTS_DTS_flags = bs_read_bslbf(bs, 2);
        p->ESCR_flag = bs_read_bslbf(bs, 1);
        p->ES_rate_flag = bs_read_bslbf(bs, 1);
        p->DSM_trick_mode_flag = bs_read_bslbf(bs, 1);
        p->additional_copy_info_flag = bs_read_bslbf(bs, 1);
        p->PES_CRC_flag = bs_read_bslbf(bs, 1);
        p->PES_extension_flag = bs_read_bslbf(bs, 1);
        p->PES_header_data_length = bs_read_uimsbf(bs, 8);
        if (p->PTS_DTS_flags == 0b10) {
            bs_skip_bslbf(bs, 4);
            p->PTS = bs_read_timestamp_with_markers(bs, 36);
        };
        if (p->PTS_DTS_flags == 0b11) {
            bs_skip_bslbf(bs, 4);
            p->PTS = bs_read_timestamp_with_markers(bs, 36);
            bs_skip_bslbf(bs, 4);
            p->DTS = bs_read_timestamp_with_markers(bs, 36);
        };
        if (p->ESCR_flag == 1) {
            bs_skip_bslbf(bs, 2);
            p->ESCR_base = bs_read_timestamp_with_markers(bs, 36);
            p->ESCR_extension = bs_read_uimsbf(bs, 9);
            bs_skip_bslbf(bs, 1);
        };
        if (p->ES_rate_flag == 1) {
            bs_skip_bslbf(bs, 1);
            p->ES_rate = bs_read_uimsbf(bs, 22);
            bs_skip_bslbf(bs, 1);
        };
        if (p->DSM_trick_mode_flag == 1) {
            p->trick_mode_control = bs_read_uimsbf(bs, 3);
            if (p->trick_mode_control == trick_mode_control_fast_forward) {
                p->field_id = bs_read_bslbf(bs, 2);
                p->intra_slice_refresh = bs_read_bslbf(bs, 1);
                p->frequency_truncation = bs_read_bslbf(bs, 2);
            };
            if (p->trick_mode_control == trick_mode_control_slow_motion) {
                p->rep_cntrl = bs_read_uimsbf(bs, 5);
            };
            if (p->trick_mode_control == trick_mode_control_freeze_frame) {
                p->field_id = bs_read_uimsbf(bs, 2);
                bs_skip_bslbf(bs, 3);
            };
            if (p->trick_mode_control == trick_mode_control_fast_reverse) {
                p->field_id = bs_read_bslbf(bs, 2);
                p->intra_slice_refresh = bs_read_bslbf(bs, 1);
                p->frequency_truncation = bs_read_bslbf(bs, 2);
            };
            if (p->trick_mode_control == trick_mode_control_slow_reverse) {
                p->rep_cntrl = bs_read_uimsbf(bs, 5);
            };
            if (p->trick_mode_control != trick_mode_control_fast_forward && p->trick_mode_control !=                                                        trick_mode_control_slow_motion && p->trick_mode_control !=                                                       trick_mode_control_freeze_frame && p->trick_mode_control !=                                                       trick_mode_control_fast_reverse && p->trick_mode_control !=                                                       trick_mode_control_slow_reverse) {
                bs_skip_bslbf(bs, 5);
            };
        };
        if (p->additional_copy_info_flag == 1) {
            bs_skip_bslbf(bs, 1);
            p->additional_copy_info = bs_read_bslbf(bs, 7);
        };
        if (p->PES_CRC_flag == 1) {
            p->previous_PES_packet_CRC = bs_read_bslbf(bs, 16);
        };
        if (p->PES_extension_flag == 1) {
            p->PES_private_data_flag = bs_read_bslbf(bs, 1);
            p->pack_header_field_flag = bs_read_bslbf(bs, 1);
            p->program_packet_sequence_counter_flag = bs_read_bslbf(bs, 1);
            p->P_STD_buffer_flag = bs_read_bslbf(bs, 1);
            p->reserved = bs_read_bslbf(bs, 3);
            p->PES_extension_flag_2 = bs_read_bslbf(bs, 1);
            if (p->PES_private_data_flag == 1) {
                p->PES_private_data = bs_read_bslbf(bs, 128);
            };
            if (p->pack_header_field_flag == 1) {
                p->pack_field_length = bs_read_uimsbf(bs, 8);
                p->pack_header_len = bs_read_bytes(bs, p->pack_header, p->pack_field_length);
            };
            if (p->program_packet_sequence_counter_flag == 1) {
                bs_skip_bslbf(bs, 1);
                p->program_packet_sequence_counter = bs_read_uimsbf(bs, 7);
                bs_skip_bslbf(bs, 1);
                p->MPEG1_MPEG2_identifier = bs_read_bslbf(bs, 1);
                p->original_stuff_length = bs_read_uimsbf(bs, 6);
            };
            if (p->P_STD_buffer_flag == 1) {
                bs_skip_bslbf(bs, 2);
                p->P_STD_buffer_scale = bs_read_bslbf(bs, 1);
                p->P_STD_buffer_size = bs_read_uimsbf(bs, 13);
            };
            if (p->PES_extension_flag_2 == 1) {
                bs_skip_bslbf(bs, 1);
                p->PES_extension_field_length = bs_read_uimsbf(bs, 7);
                p->PES_extension_field_len = bs_read_bytes(bs, p->PES_extension_field, p->PES_extension_field_length);
            };
            p->PES_packet_data_len = bs_read_bytes_all(bs, p->PES_packet_data);
        };
    };
    if (p->stream_id != pes_stream_id_program_stream_map && p->stream_id !=                                          pes_stream_id_padding_stream && p->stream_id !=                                          pes_stream_id_private_stream_2 && p->stream_id !=                                          pes_stream_id_ECM && p->stream_id !=                                          pes_stream_id_EMM && p->stream_id !=                                          pes_stream_id_program_stream_directory && p->stream_id !=                                          pes_stream_id_DSMCC_stream && p->stream_id !=                                          pes_stream_id_H_222_1) {
        p->PES_packet_data_len = bs_read_bytes_all(bs, p->PES_packet_data);
    };
};

int adaptation_field_write(adaptation_field_t* p, bs_t* bs) {
    bs_write_uimsbf(bs, p->adaptation_field_length, 8);
    if (p->adaptation_field_length > 0) {
        bs_write_bslbf(bs, p->discontinuity_indicator, 1);
        bs_write_bslbf(bs, p->random_access_indicator, 1);
        bs_write_bslbf(bs, p->elementary_stream_priority_indicator, 1);
        bs_write_bslbf(bs, p->PCR_flag, 1);
        bs_write_bslbf(bs, p->OPCR_flag, 1);
        bs_write_bslbf(bs, p->splicing_point_flag, 1);
        bs_write_bslbf(bs, p->transport_private_data_flag, 1);
        bs_write_bslbf(bs, p->adaptation_field_extension_flag, 1);
        if (p->PCR_flag == 1) {
            bs_write_uimsbf(bs, p->program_clock_reference_base, 33);
            bs_write_bslbf(bs, 0, 6);
            bs_write_uimsbf(bs, p->program_clock_reference_extension, 9);
        };
        if (p->OPCR_flag == 1) {
            bs_write_uimsbf(bs, p->original_program_clock_reference_base, 33);
            bs_write_bslbf(bs, 0, 6);
            bs_write_uimsbf(bs, p->original_program_clock_reference_extension, 9);
        };
        if (p->splicing_point_flag == 1) {
            bs_write_tcimsbf(bs, p->splice_countdown, 8);
        };
        if (p->transport_private_data_flag == 1) {
            bs_write_uimsbf(bs, p->transport_private_data_length, 8);
            bs_write_bytes(bs, p->private_data, p->private_data_len);
        };
        if (p->adaptation_field_extension_flag == 1) {
            bs_write_uimsbf(bs, p->adaptation_field_extension_length, 8);
            bs_write_bslbf(bs, p->ltw_flag, 1);
            bs_write_bslbf(bs, p->piecewise_rate_flag, 1);
            bs_write_bslbf(bs, p->seamless_splice_flag, 1);
            bs_write_bslbf(bs, 0, 5);
            if (p->ltw_flag == 1) {
                bs_write_bslbf(bs, p->ltw_valid_flag, 1);
                bs_write_uimsbf(bs, p->ltw_offset, 15);
            };
            if (p->piecewise_rate_flag == 1) {
                bs_write_bslbf(bs, 0, 2);
                bs_write_uimsbf(bs, p->piecewise_rate, 22);
            };
            if (p->seamless_splice_flag == 1) {
                bs_write_bslbf(bs, p->splice_type, 4);
                bs_write_timestamp_with_markers(bs, p->DTS_next_AU, 36);
            };
            bs_fill_bytes_all(bs, 0);
        };
        bs_fill_bytes_all(bs, 0xFF);
    };
};

int transport_packet_write(transport_packet_t* p, bs_t* bs) {
    bs_write_bslbf(bs, 0x47, 8);
    bs_write_bslbf(bs, p->transport_error_indicator, 1);
    bs_write_bslbf(bs, p->payload_unit_start_indicator, 1);
    bs_write_bslbf(bs, p->transport_priority, 1);
    bs_write_uimsbf(bs, p->PID, 13);
    bs_write_bslbf(bs, p->transport_scrambling_control, 2);
    bs_write_bslbf(bs, p->adaptation_field_control, 2);
    bs_write_uimsbf(bs, p->continuity_counter, 4);
    if (p->adaptation_field_control == 0b10 || p->adaptation_field_control ==  0b11) {
        adaptation_field_write(p->adaptation_field, bs);
    };
    if (p->adaptation_field_control == 0b01 || p->adaptation_field_control ==  0b11) {
        bs_write_bytes(bs, p->data, p->data_len);
    };
};

int PES_packet_write(PES_packet_t* p, bs_t* bs) {
    bs_write_bslbf(bs, p->packet_start_code_prefix, 24);
    bs_write_uimsbf(bs, p->stream_id, 8);
    bs_write_uimsbf(bs, p->PES_packet_length, 16);
    if (p->stream_id != pes_stream_id_program_stream_map && p->stream_id !=                                            pes_stream_id_padding_stream && p->stream_id !=                                            pes_stream_id_private_stream_2 && p->stream_id !=                                            pes_stream_id_ECM && p->stream_id !=                                            pes_stream_id_EMM && p->stream_id !=                                            pes_stream_id_program_stream_directory && p->stream_id !=                                            pes_stream_id_DSMCC_stream && p->stream_id !=                                           pes_stream_id_H_222_1) {
        bs_write_bslbf(bs, 0b10, 2);
        bs_write_bslbf(bs, p->PES_scrambling_control, 2);
        bs_write_bslbf(bs, p->PES_priority, 1);
        bs_write_bslbf(bs, p->data_alignment_indicator, 1);
        bs_write_bslbf(bs, p->copyright, 1);
        bs_write_bslbf(bs, p->original_or_copy, 1);
        bs_write_bslbf(bs, p->PTS_DTS_flags, 2);
        bs_write_bslbf(bs, p->ESCR_flag, 1);
        bs_write_bslbf(bs, p->ES_rate_flag, 1);
        bs_write_bslbf(bs, p->DSM_trick_mode_flag, 1);
        bs_write_bslbf(bs, p->additional_copy_info_flag, 1);
        bs_write_bslbf(bs, p->PES_CRC_flag, 1);
        bs_write_bslbf(bs, p->PES_extension_flag, 1);
        bs_write_uimsbf(bs, p->PES_header_data_length, 8);
        if (p->PTS_DTS_flags == 0b10) {
            bs_write_bslbf(bs, 0b0010, 4);
            bs_write_timestamp_with_markers(bs, p->PTS, 36);
        };
        if (p->PTS_DTS_flags == 0b11) {
            bs_write_bslbf(bs, 0b0011, 4);
            bs_write_timestamp_with_markers(bs, p->PTS, 36);
            bs_write_bslbf(bs, 0b0001, 4);
            bs_write_timestamp_with_markers(bs, p->DTS, 36);
        };
        if (p->ESCR_flag == 1) {
            bs_write_bslbf(bs, 0, 2);
            bs_write_timestamp_with_markers(bs, p->ESCR_base, 36);
            bs_write_uimsbf(bs, p->ESCR_extension, 9);
            bs_write_bslbf(bs, 1, 1);
        };
        if (p->ES_rate_flag == 1) {
            bs_write_bslbf(bs, 1, 1);
            bs_write_uimsbf(bs, p->ES_rate, 22);
            bs_write_bslbf(bs, 1, 1);
        };
        if (p->DSM_trick_mode_flag == 1) {
            bs_write_uimsbf(bs, p->trick_mode_control, 3);
            if (p->trick_mode_control == trick_mode_control_fast_forward) {
                bs_write_bslbf(bs, p->field_id, 2);
                bs_write_bslbf(bs, p->intra_slice_refresh, 1);
                bs_write_bslbf(bs, p->frequency_truncation, 2);
            };
            if (p->trick_mode_control == trick_mode_control_slow_motion) {
                bs_write_uimsbf(bs, p->rep_cntrl, 5);
            };
            if (p->trick_mode_control == trick_mode_control_freeze_frame) {
                bs_write_uimsbf(bs, p->field_id, 2);
                bs_write_bslbf(bs, 0, 3);
            };
            if (p->trick_mode_control == trick_mode_control_fast_reverse) {
                bs_write_bslbf(bs, p->field_id, 2);
                bs_write_bslbf(bs, p->intra_slice_refresh, 1);
                bs_write_bslbf(bs, p->frequency_truncation, 2);
            };
            if (p->trick_mode_control == trick_mode_control_slow_reverse) {
                bs_write_uimsbf(bs, p->rep_cntrl, 5);
            };
            if (p->trick_mode_control != trick_mode_control_fast_forward && p->trick_mode_control !=                                                        trick_mode_control_slow_motion && p->trick_mode_control !=                                                       trick_mode_control_freeze_frame && p->trick_mode_control !=                                                       trick_mode_control_fast_reverse && p->trick_mode_control !=                                                       trick_mode_control_slow_reverse) {
                bs_write_bslbf(bs, 0, 5);
            };
        };
        if (p->additional_copy_info_flag == 1) {
            bs_write_bslbf(bs, 1, 1);
            bs_write_bslbf(bs, p->additional_copy_info, 7);
        };
        if (p->PES_CRC_flag == 1) {
            bs_write_bslbf(bs, p->previous_PES_packet_CRC, 16);
        };
        if (p->PES_extension_flag == 1) {
            bs_write_bslbf(bs, p->PES_private_data_flag, 1);
            bs_write_bslbf(bs, p->pack_header_field_flag, 1);
            bs_write_bslbf(bs, p->program_packet_sequence_counter_flag, 1);
            bs_write_bslbf(bs, p->P_STD_buffer_flag, 1);
            bs_write_bslbf(bs, p->reserved, 3);
            bs_write_bslbf(bs, p->PES_extension_flag_2, 1);
            if (p->PES_private_data_flag == 1) {
                bs_write_bslbf(bs, p->PES_private_data, 128);
            };
            if (p->pack_header_field_flag == 1) {
                bs_write_uimsbf(bs, p->pack_field_length, 8);
                bs_write_bytes(bs, p->pack_header, p->pack_header_len);
            };
            if (p->program_packet_sequence_counter_flag == 1) {
                bs_write_bslbf(bs, 1, 1);
                bs_write_uimsbf(bs, p->program_packet_sequence_counter, 7);
                bs_write_bslbf(bs, 1, 1);
                bs_write_bslbf(bs, p->MPEG1_MPEG2_identifier, 1);
                bs_write_uimsbf(bs, p->original_stuff_length, 6);
            };
            if (p->P_STD_buffer_flag == 1) {
                bs_write_bslbf(bs, 0b10, 2);
                bs_write_bslbf(bs, p->P_STD_buffer_scale, 1);
                bs_write_uimsbf(bs, p->P_STD_buffer_size, 13);
            };
            if (p->PES_extension_flag_2 == 1) {
                bs_write_bslbf(bs, 1, 1);
                bs_write_uimsbf(bs, p->PES_extension_field_length, 7);
                bs_write_bytes(bs, p->PES_extension_field, p->PES_extension_field_len);
            };
            bs_write_bytes(bs, p->PES_packet_data, p->PES_packet_data_len);
        };
    };
    if (p->stream_id != pes_stream_id_program_stream_map && p->stream_id !=                                          pes_stream_id_padding_stream && p->stream_id !=                                          pes_stream_id_private_stream_2 && p->stream_id !=                                          pes_stream_id_ECM && p->stream_id !=                                          pes_stream_id_EMM && p->stream_id !=                                          pes_stream_id_program_stream_directory && p->stream_id !=                                          pes_stream_id_DSMCC_stream && p->stream_id !=                                          pes_stream_id_H_222_1) {
        bs_write_bytes(bs, p->PES_packet_data, p->PES_packet_data_len);
    };
};

int adaptation_field_len(adaptation_field_t* p) {
    int len;
    len = 0;
    len += 1;
    len += 1;
    len += 1;
    len += 5;
    if (p->ltw_flag == 1) {
        len += 1;
        len += 15;
    };
    if (p->piecewise_rate_flag == 1) {
        len += 2;
        len += 22;
    };
    if (p->seamless_splice_flag == 1) {
        len += 4;
        len += 36;
    };
    if (p->adaptation_field_extension_length < len /8) { p->adaptation_field_extension_length = len /8; };

    len = 0;
    if (p->adaptation_field_length > 0) {
        len += 1;
        len += 1;
        len += 1;
        len += 1;
        len += 1;
        len += 1;
        len += 1;
        len += 1;
        if (p->PCR_flag == 1) {
            len += 33;
            len += 6;
            len += 9;
        };
        if (p->OPCR_flag == 1) {
            len += 33;
            len += 6;
            len += 9;
        };
        if (p->splicing_point_flag == 1) {
            len += 8;
        };
        if (p->transport_private_data_flag == 1) {
            len += 8;
            len += p->private_data_len *8;
            p->transport_private_data_length = p->private_data_len;
        };
        if (p->adaptation_field_extension_flag == 1) {
            len += 8;
            len += p->adaptation_field_extension_length *8;
        };
    };
    if (p->adaptation_field_length < len /8) { p->adaptation_field_length = len /8; };

    len = 0;
    len += 8;
    len += p->adaptation_field_length *8;
    return len /8;
};

int transport_packet_len(transport_packet_t* p) {
    int len;
    len = 0;
    len += 8;
    len += 1;
    len += 1;
    len += 1;
    len += 13;
    len += 2;
    len += 2;
    len += 4;
    if (p->adaptation_field_control == 0b10 || p->adaptation_field_control ==  0b11) {
        len += adaptation_field_len(p->adaptation_field);
    };
    if (p->adaptation_field_control == 0b01 || p->adaptation_field_control ==  0b11) {
        len += p->data_len *8;
    };
    return len /8;
};

int PES_packet_len(PES_packet_t* p) {
    int len;
    len = 0;
    if (p->stream_id != pes_stream_id_program_stream_map && p->stream_id !=                                            pes_stream_id_padding_stream && p->stream_id !=                                            pes_stream_id_private_stream_2 && p->stream_id !=                                            pes_stream_id_ECM && p->stream_id !=                                            pes_stream_id_EMM && p->stream_id !=                                            pes_stream_id_program_stream_directory && p->stream_id !=                                            pes_stream_id_DSMCC_stream && p->stream_id !=                                           pes_stream_id_H_222_1) {
        len += 2;
        len += 2;
        len += 1;
        len += 1;
        len += 1;
        len += 1;
        len += 2;
        len += 1;
        len += 1;
        len += 1;
        len += 1;
        len += 1;
        len += 1;
        len += 8;
        if (p->PTS_DTS_flags == 0b10) {
            len += 4;
            len += 36;
        };
        if (p->PTS_DTS_flags == 0b11) {
            len += 4;
            len += 36;
            len += 4;
            len += 36;
        };
        if (p->ESCR_flag == 1) {
            len += 2;
            len += 36;
            len += 9;
            len += 1;
        };
        if (p->ES_rate_flag == 1) {
            len += 1;
            len += 22;
            len += 1;
        };
        if (p->DSM_trick_mode_flag == 1) {
            len += 3;
            if (p->trick_mode_control == trick_mode_control_fast_forward) {
                len += 2;
                len += 1;
                len += 2;
            };
            if (p->trick_mode_control == trick_mode_control_slow_motion) {
                len += 5;
            };
            if (p->trick_mode_control == trick_mode_control_freeze_frame) {
                len += 2;
                len += 3;
            };
            if (p->trick_mode_control == trick_mode_control_fast_reverse) {
                len += 2;
                len += 1;
                len += 2;
            };
            if (p->trick_mode_control == trick_mode_control_slow_reverse) {
                len += 5;
            };
            if (p->trick_mode_control != trick_mode_control_fast_forward && p->trick_mode_control !=                                                        trick_mode_control_slow_motion && p->trick_mode_control !=                                                       trick_mode_control_freeze_frame && p->trick_mode_control !=                                                       trick_mode_control_fast_reverse && p->trick_mode_control !=                                                       trick_mode_control_slow_reverse) {
                len += 5;
            };
        };
        if (p->additional_copy_info_flag == 1) {
            len += 1;
            len += 7;
        };
        if (p->PES_CRC_flag == 1) {
            len += 16;
        };
        if (p->PES_extension_flag == 1) {
            len += 1;
            len += 1;
            len += 1;
            len += 1;
            len += 3;
            len += 1;
            if (p->PES_private_data_flag == 1) {
                len += 128;
            };
            if (p->pack_header_field_flag == 1) {
                len += 8;
                len += p->pack_header_len *8;
                p->pack_field_length = p->pack_header_len;
            };
            if (p->program_packet_sequence_counter_flag == 1) {
                len += 1;
                len += 7;
                len += 1;
                len += 1;
                len += 6;
            };
            if (p->P_STD_buffer_flag == 1) {
                len += 2;
                len += 1;
                len += 13;
            };
            if (p->PES_extension_flag_2 == 1) {
                len += 1;
                len += 7;
                len += p->PES_extension_field_len *8;
                p->PES_extension_field_length = p->PES_extension_field_len;
            };
            len += p->PES_packet_data_len *8;
        };
    };
    if (p->stream_id != pes_stream_id_program_stream_map && p->stream_id !=                                          pes_stream_id_padding_stream && p->stream_id !=                                          pes_stream_id_private_stream_2 && p->stream_id !=                                          pes_stream_id_ECM && p->stream_id !=                                          pes_stream_id_EMM && p->stream_id !=                                          pes_stream_id_program_stream_directory && p->stream_id !=                                          pes_stream_id_DSMCC_stream && p->stream_id !=                                          pes_stream_id_H_222_1) {
        len += p->PES_packet_data_len *8;
    };
    if (p->PES_packet_length < len /8) { p->PES_packet_length = len /8; };

    len = 0;
    len += 24;
    len += 8;
    len += 16;
    len += p->PES_packet_length *8;
    return len /8;
};

