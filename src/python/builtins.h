/**
 *
 * Builtin python functions
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef PYTHON_IFACE_H
#  error "Do not include this file directly. Include iface.h instead."
#endif

/* Initialize builtins stuff */
int
py_builtins_init (void);

/* Uninitialize builtins stuff */
void
py_builtins_done (void);

/* Return a global dictionary of the builtins */
PyObject*
py_builtins_get_global (void);

/* Return a local dictionary of the builtins */
PyObject*
py_builtins_get_local (void);
