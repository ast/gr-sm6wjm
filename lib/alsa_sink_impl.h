/* -*- c++ -*- */
/*
 * Copyright 2019 Albin Stigö.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_SM6WJM_ALSA_SINK_IMPL_H
#define INCLUDED_SM6WJM_ALSA_SINK_IMPL_H

#include <sm6wjm/alsa_sink.h>

#include <gnuradio/filter/pfb_arb_resampler.h>
#include <alsa/asoundlib.h>
#include <string>
#include <vector>
#include <cstdint>
#include <volk/volk.h>

#include "pid.h"

namespace gr {
    namespace sm6wjm {
        class alsa_sink_impl : public alsa_sink
        {
        private:
            snd_pcm_t *d_pcm_handle;
            snd_pcm_status_t *d_pcm_status;
            std::string d_pcm_name;
            unsigned int d_channels;
            unsigned int d_samplerate;
            unsigned int d_periods;
            snd_pcm_uframes_t d_frames;
            snd_pcm_format_t d_format;
            snd_pcm_stream_t d_stream;
            
            pid d_pid;
            
            gr::filter::kernel::pfb_arb_resampler_fff * d_resamp;
            
            std::vector<float> d_buf_resampled;
            std::vector<lv_32fc_t> d_buf_complex_float;
            std::vector<lv_16sc_t> d_buf_complex_int16;
            
            void open_alsa_pcm_handle();
            
            float clamp(float x, float upper, float lower) {
                return std::min<float>(upper, std::max<float>(x, lower));
            }
            
        public:
            alsa_sink_impl(int samplerate, const std::string &device);
            ~alsa_sink_impl();
            
            bool start() override;
            bool stop() override;
            
            // Where all the action really happens
            int work(int noutput_items,
                     gr_vector_const_void_star &input_items,
                     gr_vector_void_star &output_items
                     ) override;
        };
        
    } // namespace sm6wjm
} // namespace gr

#endif /* INCLUDED_SM6WJM_ALSA_SINK_IMPL_H */
