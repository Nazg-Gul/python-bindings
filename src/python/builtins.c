/**
 *
 * Builtin python functions
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "iface.h"

static PyObject *builtins = NULL;

PY_METHOD(syspath_append)
  char *dirname;
  wchar_t *wdirname;

  PY_PARSE_TUPLE ("s", L"Method expects one string argument", &dirname);

  MBS2WCS (wdirname, dirname);

  if (wdirname)
    {
      py_syspath_append (wdirname);
      free (wdirname);
    }
PY_METH_END

PY_BEGIN_METHMAP(methods)
  PY_METHMAP_DEF (L"syspathAppend", syspath_append,
                  METH_VARARGS, L"Append specified directory to system paths")
PY_END_METHMAP

PY_INITTAB_PROC(builtins_init, L"CoreBuiltins",
                L"Module with different core built-ins", methods)
  PY_DEF_INT_CONST (L"TRUE", 1);
  PY_DEF_INT_CONST (L"FALSE", 0);
PY_INITTAB_END_PROC

/**
 * Initialize builtins stuff
 *
 * @return zero on success, non-zero otherwise
 */
int
py_builtins_init (void)
{
  PyObject *mod;

  builtins_init ();
  mod = PyImport_ImportModule ("CoreBuiltins");

  builtins = PyDict_Copy (PyEval_GetBuiltins ());
  PyDict_Merge (builtins, PyModule_GetDict (mod), 0);

  Py_DECREF (mod);

  return 0;
}

/**
 * Uninitialize builtins stuff
 */
void
py_builtins_done (void)
{
  Py_DECREF (builtins);
}

/**
 * Return a dictionary of the builtins
 *
 * @return dictionary of builtins
 */
PyObject*
py_builtins_get (void)
{
  return builtins;
}
