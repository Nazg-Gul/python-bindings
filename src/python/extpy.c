/**
 * Extended functionality of Python bindings
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "iface.h"
#include <wchar.h>

/**
 * Create error object for return
 *
 * @param error_msg - error message
 * @return error object to return
 */
PyObject*
extpy_return_pyobj_error (PyObject *type, wchar_t *error_msg)
{
  char *mbmsg;

  WCS2MBS (mbmsg, error_msg);

  PyErr_SetString (type, mbmsg);

  free (mbmsg);

  return NULL;
}

/*
 * Set dictionary key's value without causing memory leaks.
 * When some object is just created to be added to the dictionary,
 * its reference count needs to be decremented so it can be reclaimed.
 *
 * @param dict - dictionary to operate on
 * @param key - dictionary's key
 * @param value - value to set
 */
int
extpy_dict_set_item_str (PyObject *dict, wchar_t *key, PyObject *value)
{
  char *mbkey;

  WCS2MBS (mbkey, key);

  /* Add value to dictionary */
  int ret = PyDict_SetItemString (dict, mbkey, value);

  free (mbkey);

  /* Delete original */
  Py_DECREF (value);

  return ret;
}
