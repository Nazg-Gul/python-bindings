/**
 *
 * C implementation of somePython functions
 *
 * Copyright 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef PYTHON_IFACE_H
#  error "Do not include this file directly. Include iface.h instead."
#endif

/* Formed writing to Python stream  */
int
py_proc_write (int stream, const wchar_t *format, ...);
