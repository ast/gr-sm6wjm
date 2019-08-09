/* -*- c++ -*- */

#define SM6WJM_API

%include "gnuradio.i"           // the common stuff

//load generated python docstrings
%include "sm6wjm_swig_doc.i"

%{
#include "sm6wjm/alsa_sink.h"
%}

%include "sm6wjm/alsa_sink.h"
GR_SWIG_BLOCK_MAGIC2(sm6wjm, alsa_sink);
