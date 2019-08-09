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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "alsa_sink_impl.h"
#include <stdexcept>
#include <gnuradio/filter/firdes.h>

namespace gr {
    namespace sm6wjm {
        
        alsa_sink::sptr
        alsa_sink::make(int samplerate, const std::string &device)
        {
            return gnuradio::get_initial_sptr
            (new alsa_sink_impl(samplerate, device));
        }
        
        /*
         * The private constructor
         */
        alsa_sink_impl::alsa_sink_impl(int samplerate, const std::string &device)
        : d_pcm_name(device),
        d_samplerate(samplerate),
        d_periods(4),
        d_frames(1024),
        d_format(SND_PCM_FORMAT_S16),
        d_channels(2),
        d_stream(SND_PCM_STREAM_PLAYBACK),
        d_pid(0.70, 0.01, 0.0001),
        gr::sync_block("alsa_sink",
                       gr::io_signature::make(1, 1, sizeof(float)),
                       gr::io_signature::make(0, 0, 0))
        {
            // Some wigg
            d_buf_resampled.resize(2 * d_frames);
            d_buf_complex_float.resize(d_frames);
            d_buf_complex_int16.resize(d_frames);
            open_alsa_pcm_handle();
            
            // it's indeed possible to use normalized frequencies here,
            // even if the documentation doesn't say it.
            std::vector<float>
            taps = gr::filter::firdes::low_pass_2(32, 32, 0.5, 0.1, 50);
            
            d_resamp = new gr::filter::kernel::pfb_arb_resampler_fff(1.0, taps, 32);
            // ALSA status struct
            snd_pcm_status_malloc(&d_pcm_status);
            
            // It's more efficient, I think, and less jittery
            set_output_multiple(int(d_frames));
            set_max_noutput_items(int(d_frames));
        }
        
        /*
         * Our virtual destructor.
         */
        alsa_sink_impl::~alsa_sink_impl()
        {
            snd_pcm_close(d_pcm_handle);
        }
        
        void
        alsa_sink_impl::open_alsa_pcm_handle() {
            snd_pcm_t *pcm_handle = nullptr;
            snd_pcm_hw_params_t *hwparams;
            snd_pcm_sw_params_t *swparams;
            
            int err = 0;
            
            snd_pcm_hw_params_alloca(&hwparams);
            snd_pcm_sw_params_alloca(&swparams);
            
            /* Open normal blocking */
            if ((err = snd_pcm_open(&pcm_handle, d_pcm_name.c_str(), d_stream, 0)) < 0) {
                // snd_strerror(err)
                throw std::runtime_error("snd_pcm_open() failed");
            }
            
            /* Init hwparams with full configuration space */
            if ((err = snd_pcm_hw_params_any(pcm_handle, hwparams)) < 0) {
                throw std::runtime_error("snd_pcm_hw_params_any() failed");
            }
            
            /* Interleaved access. (IQ interleaved). */
            if ((err = snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
                throw std::runtime_error("snd_pcm_hw_params_set_access() failed");
            }
            
            // Set number of channels
            if ((err = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, d_channels)) < 0) {
                throw std::runtime_error("snd_pcm_hw_params_set_channels() failed");
            }
            
            /* Set sample format */
            /* Use native format of device to avoid costly conversions */
            if ((err = snd_pcm_hw_params_set_format(pcm_handle, hwparams, d_format)) < 0) {
                throw std::runtime_error("snd_pcm_hw_params_set_format() failed");
            }
            
            /* Set sample rate. If the exact rate is not supported exit */
            if ((err = snd_pcm_hw_params_set_rate(pcm_handle, hwparams, d_samplerate, 0)) < 0) {
                throw std::runtime_error("snd_pcm_hw_params_set_rate() failed");
            }
            
            // Period size
            if ((err = snd_pcm_hw_params_set_period_size(pcm_handle, hwparams, d_frames, 0)) < 0) {
                throw std::runtime_error("snd_pcm_hw_params_set_period_size() failed");
            }
            
            /* Set number of periods. Periods used to be called fragments. */
            if ((err = snd_pcm_hw_params_set_periods(pcm_handle, hwparams, d_periods, 0)) < 0) {
                throw std::runtime_error("snd_pcm_hw_params_set_periods() failed");
            }
            
            /* Apply HW parameter settings to */
            /* PCM device and prepare device  */
            if ((err = snd_pcm_hw_params(pcm_handle, hwparams)) < 0) {
                throw std::runtime_error("snd_pcm_hw_params() failed");
            }
            
            // SW params
            if ((err = snd_pcm_sw_params_current(pcm_handle, swparams)) < 0)
            {
                throw std::runtime_error("snd_pcm_sw_params_current() failed");
            }
            
            // Autostart when the ringbuffer is full
            err = snd_pcm_sw_params_set_start_threshold(pcm_handle, swparams, (d_periods) * d_frames);
            if (err < 0) {
                throw std::runtime_error("snd_pcm_sw_params_set_start_threshold() failed");
            }
            
            // Apply SW params
            if((err = snd_pcm_sw_params(pcm_handle, swparams)) < 0) {
                throw std::runtime_error("snd_pcm_sw_params() failed");
            }
            
            d_pcm_handle = pcm_handle;
        }
        
        bool
        alsa_sink_impl::start() {
            snd_pcm_prepare(d_pcm_handle);
            return true;
        }
        
        bool
        alsa_sink_impl::stop() {
            snd_pcm_drain(d_pcm_handle);
            return true;
        }
        
        int
        alsa_sink_impl::work(int noutput_items,
                             gr_vector_const_void_star &input_items,
                             gr_vector_void_star &output_items)
        {
            const float *in = (const float *) input_items[0];
            // get status of pcm
            snd_pcm_status(d_pcm_handle, d_pcm_status);
            snd_pcm_uframes_t avail = snd_pcm_status_get_avail(d_pcm_status);
            snd_pcm_sframes_t delay = snd_pcm_status_get_delay(d_pcm_status);
            snd_pcm_state_t pcm_state = snd_pcm_status_get_state(d_pcm_status);
            
            float resamp = 1.0;
            
            switch (pcm_state) {
                case SND_PCM_STATE_OPEN: break;
                case SND_PCM_STATE_SETUP: break;
                case SND_PCM_STATE_PREPARED:
                    //printf("prep\n");
                    break;
                case SND_PCM_STATE_RUNNING: {
                    float fill = delay/float(d_frames*d_periods);
                    resamp = 1 + d_pid.update(fill);
                    resamp = clamp(resamp, 1.1, 0.9);
                    
                    //printf("fill: %f\n", fill);
                } break;
                case SND_PCM_STATE_XRUN: break;
                case SND_PCM_STATE_DRAINING: break;
                case SND_PCM_STATE_PAUSED: break;
                case SND_PCM_STATE_SUSPENDED: break;
                case SND_PCM_STATE_DISCONNECTED: break;
                default: break;
            }
            
            int n_read = 0;
            int n_written = 0;
            d_resamp->set_rate(resamp);
            n_written = d_resamp->filter(d_buf_resampled.data(),
                                         (float*)in,
                                         noutput_items,
                                         n_read);
            //printf("n_written: %d, n_read: %d\n", n_written, n_read);
            
            // Copy data to both channels
            volk_32f_x2_interleave_32fc(d_buf_complex_float.data(),
                                        d_buf_resampled.data(), //in,
                                        d_buf_resampled.data(), //in,
                                        n_written);
            // x2, data is "complex"
            volk_32f_s32f_convert_16i((int16_t*) d_buf_complex_int16.data(),
                                      (const float*) d_buf_complex_float.data(),
                                      float(INT16_MAX-1),
                                      n_written * 2);
            
            snd_pcm_sframes_t n_err;
            n_err = snd_pcm_writei(d_pcm_handle, d_buf_complex_int16.data(), n_written);
            if (n_err < 0) {
                int err = snd_pcm_recover(d_pcm_handle, (int)n_err, 0);
                if (err < 0) {
                    throw std::runtime_error("could not recover from xrun");
                }
            }
            
            // Tell runtime system how many output items we consumed.
            return n_read;
        }
        
    } /* namespace sm6wjm */
} /* namespace gr */
