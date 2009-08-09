/**
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * Interface header for Python sub-system
 *
 * Some modules and functions implemented with
 * using source code of Blender-3D project
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef PYTHON_IFACE_H
#define PYTHON_IFACE_H

#include <smartinclude.h>

BEGIN_HEADER

#include <Python.h>
#include <wchar.h>

/****
 * Common Python methods
 */

#define PY_BEGIN_METHMAP(map_name) \
  static py_method_def_t map_name[] = {

#define PY_METHMAP_DEF(name, meth, flag, doc) \
  {name, meth, flag, doc},

#define PY_END_METHMAP \
  {NULL, NULL, 0, NULL} \
};

#define PY_METHOD_FORWARD(name) \
  static PyObject* \
  name (PyObject *__self, PyObject *__args);

#define PY_METHOD(name) \
  static PyObject* \
  name (PyObject *__self, PyObject *__args) \
  {

#define PY_METH_END \
    Py_RETURN_NONE; \
  }

#define PY_PARSE_TUPLE(mask, err_msg, args...)   \
  if (!PyArg_ParseTuple (__args, mask, ##args)) \
    { \
      return extpy_return_pyobj_error (PyExc_ValueError, err_msg); \
    }

/* Initialize Python sub-system */
int
python_init (int argc, char **argv, struct _inittab *inittab_modules);

/* Finalize Python sub-system */
void
python_done (void);

/****
 * Python's modules
 */

typedef struct {
  const wchar_t *name;  /* The name of the built-in function/method */
  PyCFunction    meth;  /* The C function that implements it */
  int            flags; /* Combination of METH_xxx flags, which mostly
                              describe the args expected by the C func */
  const wchar_t *doc;   /* The __doc__ attribute, or NULL */
} py_method_def_t;

typedef struct {
  wchar_t *name;  /* Module's name */
  wchar_t *descr; /* Module's description */

  PyObject *handle; /* Python handle of module */
  PyObject *dict;   /* Module's dictionary */

  PyMethodDef *conv_methods; /* Module's initial methods */
} py_module_t;

/* Create new Python module */
py_module_t*
py_module_new (const wchar_t *name, const wchar_t *descr,
               py_method_def_t *methods);

/* Free Python module */
void
py_module_free (py_module_t *module);

/****
 * Scripts
 */

typedef struct {
  wchar_t *script;
  PyObject *compiled;
  wchar_t *file_name;
} py_script_t;

/* Create script from buffer */
py_script_t*
py_script_new_buffer (const wchar_t *buffer);

/* Create script from file */
py_script_t*
py_script_new_file (const wchar_t *file_name);

/* Free Python script */
void
py_script_free (py_script_t *scrint);

/* Free compiled Python script */
void
py_script_free_compiled (py_script_t *script);

/****
 * Executions
 */

/* Run Python script */
PyObject*
py_run_script_at_dict (py_script_t *script, PyObject *dict);

/* Run Python script */
PyObject*
py_run_script (py_script_t *script);

/****
 * Extensions
 */

#include "extpy.h"

END_HEADER

#endif
