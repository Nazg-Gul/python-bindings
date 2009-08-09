/**
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "iface.h"
#include "import.h"

#include <wchar.h>
#include <stdio.h>

#ifdef PACKAGE
#  define PROGRAM_NAME PACKAGE
#else
#  define PROGRAM_NAME "RootProgram"
#endif

/**
 * Converts internal method's definition to Python method definition
 *
 * @param def - definitions to be converted
 * @return Python method's definitions
 */
static void
convert_method_def (py_method_def_t def, PyMethodDef *result)
{
  WCS2MBS (result->ml_name, def.name);
  result->ml_meth  = def.meth;
  result->ml_flags = def.flags;
  WCS2MBS (result->ml_doc, def.doc);
}

/**
 * Convert list of internal method's definition to Python method definition
 *
 * @param list - list of definitions to be converted
 * @return list of converted definitions
 * @sideeffect allocate memory for output value. Use free_methods_list to free
 */
static PyMethodDef*
convert_methods_list (py_method_def_t *list)
{
  long i, count = 0;
  PyMethodDef *out_list;

  /* Get count of entries */
  while (list[count].name)
    {
      ++count;
    }

  MALLOC_ZERO (out_list, sizeof (PyMethodDef) * (count + 1));

  for (i = 0; i < count; ++i)
    {
      convert_method_def (list[i], &out_list[i]);
    }

  return out_list;
}

/**
 * Free list of converted methods' definitions
 */
static void
free_methods_list (PyMethodDef *list)
{
  long i = 0;

  while (list[i].ml_name)
    {
      free ((char*)list[i].ml_name);

      if (list[i].ml_doc)
        {
          free ((char*)list[i].ml_doc);
        }

      ++i;
      }

  SAFE_FREE (list);
}

/**
 * Initialize importing stuff
 */
static void
init_import (void)
{
  PyObject *module, *dict;
  PyObject *import = PyCFunction_New (py_import_meth, NULL);

  module = PyImport_AddModule ("__builtin__");
  dict = PyModule_GetDict (module);

  extpy_dict_set_item_str (dict, L"__import__", import);
}

/**
 * Initialize system paths
 */
static void
init_syspath (first_time)
{
  PyObject *mod;

  printf ("Checking for installed Python... ");
  mod = PyImport_ImportModule ("site");

  if (mod)
    {
      printf("got it!\n");
      Py_DECREF (mod);
    }
  else
    {
      /* import 'site' failed */
      PyErr_Clear (  );

      if (first_time)
        {
          printf ("No installed Python found.\n");
          printf ("Only built-in modules are available. "
                  "Some scripts may not run.\n");
          printf ("Continuing happily.\n");
        }
    }

  mod = PyImport_ImportModule( "sys" );	/* new ref */

  if (mod)
    {
      Py_DECREF (mod);
    }
  else
    {
      PyErr_Clear ();
      printf("Warning: import of sys module failed\n");
    }
}

/****
 * Common Python methods
 */

/**
 * Initialize Python sub-system
 *
 * @param argc - count of arguments
 * @param argv - argument values
 * @param inittab_module - list of init-tab modules
 * @return zero on success, non-zero otherwise
 */
int
python_init (int argc, char **argv, struct _inittab *inittab_modules)
{
  int count = 3;
  const char *version;
  char *blank_ptr;

  static int argc_copy = 0;
  static char **argv_copy = NULL;
  int first_time = argc;

  if (first_time)
    {
      argc_copy = argc;
      argv_copy = argv;
    }

  Py_SetProgramName (PROGRAM_NAME);

  /* Print python version */
  version = Py_GetVersion ();
  blank_ptr = strchr (version, ' ');

  if (blank_ptr)
    {
      count = blank_ptr - version;
    }

  printf ("Compiled with Python version %.*s.\n", count, version);


  /* Initialize the TOP-LEVEL modules */
  PyImport_ExtendInittab (inittab_modules);

  Py_Initialize ();

  PySys_SetArgv (argc_copy, argv_copy);

  /* Initialize thread support */
  PyEval_InitThreads ();

  init_import ();

  init_syspath (first_time);

  return 0;
}

/**
 * Finalize Python sub-system
 */
void
python_done (void)
{
  /* End python */
  Py_Finalize ();
}

/****
 * Python's modules
 */

/**
 * Create new Python module
 *
 * @param name - name of module
 * @param descr - module's description
 * @param methods - module's methods
 * @return descriptor of new module
 * @sideeffect allocate memory for output value. Use py_module_free to free
 */
py_module_t*
py_module_new (const wchar_t *name, const wchar_t *descr,
               py_method_def_t *methods)
{
  py_module_t *module;
  char *mbname, *mbdescr;
  PyMethodDef *methods_list;

  methods_list = convert_methods_list (methods);

  MALLOC_ZERO (module, sizeof (py_module_t));

  WCS2MBS (mbname,  name);
  WCS2MBS (mbdescr, descr);

  module->handle = Py_InitModule3 (mbname, methods_list, mbdescr);
  module->dict = PyModule_GetDict (module->handle);
  module->conv_methods = methods_list;

  SAFE_FREE (mbname);
  SAFE_FREE (mbdescr);

  return module;
}

/**
 * Free Python module
 *
 * @param nodule - module to be freed
 */
void
py_module_free (py_module_t *module)
{
  if (!module)
    {
      return;
    }

  free_methods_list (module->conv_methods);
  Py_DECREF (module->handle);

  SAFE_FREE (module);
}

/**
 * Create script from buffer
 *
 * @param buffer - buffer where script's code is stored
 * @return new script's descriptor
 * @sideeffect allocate memory for return value. Use py_script_free() to free
 */
py_script_t*
py_script_new_buffer (const wchar_t *buffer)
{
  py_script_t *script;

  MALLOC_ZERO (script, sizeof (py_script_t));

  script->script = wcsdup (buffer);

  return script;
}

/**
 * Create script from file
 *
 * @param file_name - name of file to load as script
 * @return new script's descriptor
 * @sideeffect allocate memory for return value. Use py_script_free() to free
 */
py_script_t*
py_script_new_file (const wchar_t *file_name)
{
  char *mbfn, *buffer;
  struct stat st;
  py_script_t *script;
  wchar_t *wcbuf;
  FILE *file;

  WCS2MBS (mbfn, file_name);

  if (!mbfn)
    {
      return NULL;
    }

  if (stat (mbfn, &st))
    {
      return NULL;
    }

  file = fopen (mbfn, "rb");

  if (!file)
    {
      return NULL;
    }

  buffer = malloc (sizeof (char) * (st.st_size + 1));
  fread (buffer, sizeof (char), st.st_size / sizeof (char), file);
  buffer[st.st_size] = '\0';
  fclose (file);

  MBS2WCS (wcbuf, buffer);

  if (!wcbuf)
    {
      free (buffer);
      return NULL;
    }

  script = py_script_new_buffer (wcbuf);
  script->file_name = wcsdup (file_name);

  free (buffer);
  free (wcbuf);

  return script;
}

/**
 * Free Python script
 *
 * @param script - script to be freed
 */
void
py_script_free (py_script_t *script)
{
  if (!script)
    {
      return;
    }

  py_script_free_compiled (script);

  SAFE_FREE (script->file_name);
  SAFE_FREE (script->script);
  SAFE_FREE (script);
}

/**
 * Free compiled Python script
 *
 * @param script - script which compiled text fill be freed
 */
void
py_script_free_compiled (py_script_t *script)
{
  if (script && script->compiled)
    {
      Py_DECREF (script->compiled);
      script->compiled = NULL;
    }
}

/**
 * Run Python script
 *
 * @param script - script to be executed
 * @param dict - global dictionary
 * @return Python eval's result
 */
PyObject*
py_run_script (py_script_t *script, PyObject *dict)
{
  char *mbfn = NULL;

  if (!script)
    {
      return NULL;
    }

  /* Compile script */
  if (!script->compiled)
    {
      char *mbscript;

      WCS2MBS (mbscript, script->script);

      script->compiled = Py_CompileString (mbscript, "", Py_file_input);
      free (mbscript);

      if (PyErr_Occurred ())
        {
          /* Compilation error occurred */
          PyErr_Print ();
          py_script_free_compiled (script);
          return NULL;
        }
    }

  if (script->file_name)
    {
      WCS2MBS (mbfn, script->file_name);
    }

  PyDict_SetItemString (dict, "__file__",
                        PyString_FromString (mbfn ? mbfn : ""));

  SAFE_FREE (mbfn);

  return PyEval_EvalCode ((PyCodeObject*)script->compiled, dict, dict);
}