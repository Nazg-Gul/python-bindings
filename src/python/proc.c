/**
 *
 * C implementation of somePython functions
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "iface.h"

/**
 * Formed writing to Python stream
 *
 * @param stream - stream specification (PY_STDOUT or PY_STDERR)
 * @param format - format of string to write
 * @return zero on success, non-zero otherwise
 */
int
py_proc_write (int stream, const wchar_t *format, ...)
{
  PyObject *mod_sys, *dict_sys, *io, *write, *result = NULL;
  wchar_t buffer[4096];
  char *mbbuf;

  mod_sys = PyImport_ImportModule ("sys");
  dict_sys = PyModule_GetDict (mod_sys);

  PACK_ARGS (format, buffer, BUF_LEN (buffer));

  io = PyDict_GetItemString (dict_sys,
                             (stream == PY_STDOUT ? "stdout" : "stderr"));

  write = PyObject_GetAttrString (io, "write");

  if (!write)
    {
      Py_DECREF (mod_sys);
      return -1;
    }

  WCS2MBS (mbbuf, buffer);

  if (!mbbuf)
    {
      Py_DECREF (mod_sys);
      Py_DECREF (write);
      return -1;
    }

  EXTPY_CALL_OBJECT (result, write, "(s)", mbbuf);

  if (result)
    {
      Py_DECREF (result);
    }

  Py_DECREF (write);
  Py_DECREF (mod_sys);

  free (mbbuf);

  return result != NULL;
}
