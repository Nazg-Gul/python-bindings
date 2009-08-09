/**
 * Extended functionality of Python bindings
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef PYTHON_IFACE_H
#  error "Do not include this file directly. Include iface.h instead."
#endif

/* Create error object for return */
PyObject*
extpy_return_pyobj_error (PyObject *type, wchar_t *error_msg);

/* Set dictionary key's value without causing memory leaks */
int
extpy_dict_set_item_str (PyObject *dict, wchar_t *key, PyObject *value);
