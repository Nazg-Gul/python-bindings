/**
 * Smart including of headers
 *
 * Copyright 2008, 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef SMARTINCLUDE_H
#define SMARTINCLUDE_H

/*
  This macroses are used in header files to ensure that the declarations
  within are properly encapsulated in an `extern "C" { .. }` block when
  included from a  C++ compiler.
 */
#ifdef __cplusplus
#  define BEGIN_HEADER extern "C" {
#  define END_HEADER }
#else
#  define BEGIN_HEADER
#  define END_HEADER
#endif

BEGIN_HEADER

/* Attribute for unused parameter to avoid */
/* compilator's waringns */
#ifdef HAVE__ATTRIBUTE__
#  define ATTR_UNUSED __attribute__((unused))
#else
#  define ATTR_UNUSED
#endif

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "macrodef.h"

/* Some definitions to tell compiler use specified stuff */
#define __USE_GNU
#define __USE_ISOC95
#define __USE_ISOC99
#define __USE_FILE_OFFSET64 1
#define USE_WIDEC_SUPPORT

END_HEADER

#endif
