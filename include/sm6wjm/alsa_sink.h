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

#ifndef INCLUDED_SM6WJM_ALSA_SINK_H
#define INCLUDED_SM6WJM_ALSA_SINK_H

#include <sm6wjm/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
    namespace sm6wjm {
        
        /*!
         * \brief <+description of block+>
         * \ingroup sm6wjm
         *
         */
        class SM6WJM_API alsa_sink : virtual public gr::sync_block
        {
        public:
            typedef boost::shared_ptr<alsa_sink> sptr;
            
            /*!
             * \brief Return a shared_ptr to a new instance of sm6wjm::alsa_sink.
             *
             * To avoid accidental use of raw pointers, sm6wjm::alsa_sink's
             * constructor is in a private implementation
             * class. sm6wjm::alsa_sink::make is the public interface for
             * creating new instances.
             */
            static sptr make(int samplerate, const std::string &device);
        };
        
    } // namespace sm6wjm
} // namespace gr

#endif /* INCLUDED_SM6WJM_ALSA_SINK_H */

