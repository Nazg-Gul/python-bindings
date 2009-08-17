/**
 * Tracing functionality of Python bindings
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "iface.h"

static PyObject *o_stdout = NULL, *s_stdout = NULL;
static PyObject *o_stderr = NULL, *s_stderr = NULL;

/**
 * Initialize tracing stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
py_tracer_init (void)
{
  PyObject *mod_sys, *mod_stringio;
  PyObject *dict_sys, *dict_stringio;
  PyObject *stringio_class;

  mod_sys = PyImport_ImportModule ("sys");
  mod_stringio = PyImport_ImportModule ("StringIO");

  dict_sys = PyModule_GetDict (mod_sys);
  dict_stringio = PyModule_GetDict (mod_stringio);

  stringio_class = PyDict_GetItemString (dict_stringio, "StringIO");

  o_stdout = PyInstance_New (stringio_class, NULL, NULL);
  s_stdout = PyDict_GetItemString (dict_sys, "stdout");
  PyDict_SetItemString (dict_sys, "stdout", o_stdout);

  o_stderr = PyInstance_New (stringio_class, NULL, NULL);
  s_stderr = PyDict_GetItemString (dict_sys, "stderr");
  PyDict_SetItemString (dict_sys, "stderr", o_stderr);

  Py_DECREF (mod_sys);
  Py_DECREF (mod_stringio);

  return 0;
}

/**
 * Uninitialize tracing stuff
 */
void
py_tracer_done (void)
{
  PyObject *mod_sys, *dict_sys;

  mod_sys = PyImport_ImportModule ("sys");
  dict_sys = PyModule_GetDict (mod_sys);

  if (s_stdout)
    {
      PyDict_SetItemString (dict_sys, "stdout", s_stderr);
      Py_DECREF (o_stdout);
      s_stdout = NULL;
    }

  if (s_stderr)
    {
      PyDict_SetItemString (dict_sys, "stderr", s_stderr);
      Py_DECREF (o_stderr);
      s_stderr = NULL;
    }

  Py_DECREF (mod_sys);
}

/**
 * Truncate specified buffer
 *
 * @param type - type of buffer (PY_STDOUT or PY_STDERR)
 * @return buffer from stream
 * @sideeffect allocate memory for output value
 */
void
py_tracer_truncate_buffer (int type)
{
  PyObject *mod_sys, *dict_sys, *stream, *truncate;
  PyObject *result;

  mod_sys = PyImport_ImportModule ("sys");
  dict_sys = PyModule_GetDict (mod_sys);

  stream =
    PyDict_GetItemString (dict_sys,
                          (type == PY_STDOUT ? "stdout" : "stderr"));

  truncate = PyObject_GetAttrString (stream, "truncate");

  if (!truncate)
    {
      Py_DECREF (mod_sys);
      return;
    }

  EXTPY_VOIDCALL_OBJECT (truncate, "(i)", 0);

  Py_DECREF (mod_sys);
  Py_DECREF (truncate);
}

/**
 * Get specified buffer
 *
 * @param type - type of buffer (PY_STDOUT or PY_STDERR)
 * @return buffer from stream
 * @sideeffect allocate memory for output value
 */
wchar_t*
py_tracer_get_buffer (int type)
{
  PyObject *mod_sys, *dict_sys, *stream, *getvalue;
  PyObject *result;
  wchar_t *wcs = NULL;

  mod_sys = PyImport_ImportModule ("sys");
  dict_sys = PyModule_GetDict (mod_sys);

  stream =
    PyDict_GetItemString (dict_sys,
                          (type == PY_STDOUT ? "stdout" : "stderr"));

  getvalue = PyObject_GetAttrString (stream, "getvalue");

  if (!getvalue)
    {
      Py_DECREF (mod_sys);
      return NULL;
    }

  EXTPY_CALL_OBJECT (result, getvalue, "()");

  if (result)
    {
      MBS2WCS (wcs, PyString_AsString (result));
      Py_DECREF (result);
    }

  Py_DECREF (mod_sys);
  Py_DECREF (getvalue);

  return wcs;
}
