/**
 * Tracing functionality of Python bindings
 *
 * Copyright 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef PYTHON_IFACE_H
#  error "Do not include this file directly. Include iface.h instead."
#endif

/* Initialize tracing stuff */
int
py_tracer_init (void);

/* Uninitialize tracing stuff */
void
py_tracer_done (void);

/* Truncate specified buffer */
void
py_tracer_truncate_buffer (int type);

/* Get specified buffer */
wchar_t*
py_tracer_get_buffer (int type);
