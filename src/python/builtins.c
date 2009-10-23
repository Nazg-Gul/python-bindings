/**
 *
 * Builtin python functions
 *
 * Copyright 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "iface.h"

static PyObject *global_builtins = NULL;
static PyObject *mod_corebuilt = NULL;

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
  /* CoreBuiltins module initialization */
  builtins_init ();
  mod_corebuilt = PyImport_ImportModule ("CoreBuiltins");

  global_builtins = PyEval_GetBuiltins ();

  return 0;
}

/**
 * Uninitialize builtins stuff
 */
void
py_builtins_done (void)
{
  Py_DECREF (mod_corebuilt);
}

/**
 * Return a global dictionary of the builtins
 *
 * @return dictionary of builtins
 */
PyObject*
py_builtins_get_global (void)
{
  return global_builtins;
}

/**
 * Return a local dictionary of the builtins
 *
 * @return dictionary of builtins
 */
PyObject*
py_builtins_get_local (void)
{
  return PyModule_GetDict (mod_corebuilt);
}
