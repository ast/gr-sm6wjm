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

#ifndef INCLUDED_SM6WJM_PID_H
#define INCLUDED_SM6WJM_PID_H

class pid {
    float d_sp;
    float d_Kp;
    float d_Ki;
    float d_sumedt;
    
public:
    pid(float sp, float Kp, float Ki)
    : d_sp(sp), d_Kp(Kp), d_Ki(Ki), d_sumedt(0) {
        //d_Kp = 0.01;
        //d_Ki = 0.0001;
    }
    
    inline float update(float pv) {
        const float e = d_sp - pv;
        d_sumedt += e;
        return d_Kp * e + d_Ki * d_sumedt;
    }
};

#endif /* INCLUDED_SM6WJM_PID_H */
