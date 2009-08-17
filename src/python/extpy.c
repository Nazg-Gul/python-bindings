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
 * Fill stdout and stderr fields of running result
 */
static inline void
fill_result_outputs (extpy_run_result_t *result)
{
  result->stdout = py_tracer_get_buffer (PY_TRACER_STDOUT);
  result->stderr = py_tracer_get_buffer (PY_TRACER_STDERR);
}

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

/**
 * Run python file
 *
 * @param filename - name of file to run
 * @return run result
 * @sideeffect allocate memory for output value. USe extpy_run_free() to free
 */
extpy_run_result_t*
extpy_run_file (wchar_t *filename)
{
  extpy_run_result_t *result;

  MALLOC_ZERO (result, sizeof (extpy_run_result_t));

  result->result = py_run_file (filename);
  fill_result_outputs (result);

  return result;
}

/**
 * Run python script
 *
 * @param script - script to run
 * @return run result
 * @sideeffect allocate memory for output value. USe extpy_run_free() to free
 */
extpy_run_result_t*
extpy_run_script (py_script_t *script)
{
  extpy_run_result_t *result;

  MALLOC_ZERO (result, sizeof (extpy_run_result_t));

  result->result = py_run_script (script);
  fill_result_outputs (result);

  return result;
}

/**
 * Free running results
 *
 * @param result - results to be freed
 */
void
extpy_run_free (extpy_run_result_t* result)
{
  if (!result)
    {
      return;
    }

  if (result->result)
    {
      Py_DECREF (result->result);
    }

  SAFE_FREE (result->stdout);
  SAFE_FREE (result->stderr);
}
