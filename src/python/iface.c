/**
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "iface.h"

#include <wchar.h>
#include <stdio.h>

#ifdef PACKAGE
#  define PROGRAM_NAME PACKAGE
#else
#  define PROGRAM_NAME "RootProgram"
#endif

/* Program's name (argv[0]) */
static wchar_t *progname = NULL;

/* List of created modules */
static py_module_t **modules = NULL;
static long modules_count = 0;

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
 * Reutrn string from last slash
 *
 * @param string - string to get substring from
 * @return pointer to a string frmo the last slash
 */
static wchar_t*
last_slash (const wchar_t *string)
{
  wchar_t *lfslash, *lbslash;

  lfslash = wcsrchr (string, '/');
  lbslash = wcsrchr (string, '\\');

  if (!lfslash)
    {
      return lbslash;
    }
  else if (!lbslash)
    {
      return lfslash;
    }

  if ((intptr_t)lfslash < (intptr_t)lbslash)
    {
      return lbslash;
    }
  else
    {
      return lfslash;
    }
}


/**
 * Append entry to sis directory
 *
 * @param dirname - name of directory to append
 */
static void
syspath_append (wchar_t *dirname)
{
  PyObject *mod_sys = NULL, *dict = NULL, *path = NULL, *dir = NULL;
  short ok = 1;
  char *mbdirname;

  WCS2MBS (mbdirname, dirname);

  mod_sys = PyImport_ImportModule ("sys"); /* new ref */

  if (mod_sys)
    {
      dict = PyModule_GetDict (mod_sys); /* borrowed ref */
      path = PyDict_GetItemString (dict, "path"); /* borrowed ref */
      if (!PyList_Check (path))
        {
          ok = 0;
        }
    }
  else
    {
      /* cant get the sys module */
      /* PyErr_Clear(); is called below */
      ok = 0;
    }


  dir = PyString_FromString (mbdirname);
  free (mbdirname);

  if (ok && PySequence_Contains (path, dir) == 0)
    {
      /* Only add if we need to */
      if (PyList_Append (path, dir) != 0)
        {
          ok = 0; /* append failed */
        }
    }

    if ((ok == 0) || PyErr_Occurred ())
      {
        fprintf (stderr, "Warning: could import or build sys.path\n");
      }

  PyErr_Clear ();
  Py_DECREF (dir);
  Py_XDECREF (mod_sys);
}

/**
 * Initialize system paths
 *
 * @param first_time - is functions calls for the first time?
 */
static void
init_syspath (int first_time)
{
  PyObject *mod;
  wchar_t *last;
  long n;

  /* looks for the last dir separator */
  last = last_slash (progname);

  n = last - progname;
  if (n > 0)
    {
      wchar_t execdir[4096];

      wcsncpy (execdir, progname, MIN (n, BUF_LEN (execdir)));
      if (execdir[n - 1] == '.')
        {
          n--; /*fix for when run as ./program */
        }
      execdir[n] = '\0';

      /* append to module search path */
      syspath_append (execdir);
    }
  else
    {
      printf( "Warning: could not determine argv[0] path\n" );
    }

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

/**
 * Create a new Python dictionary object
 *
 * @return Python dictionary object
 * @sideeffect allocate memory for output value
 */
static PyObject*
create_global_dictionary (void)
{
  PyObject *dict = PyDict_New ();

  PyDict_SetItemString (dict, "__builtins__", PyEval_GetBuiltins ());
  extpy_dict_set_item_str (dict, L"__name__",
                           PyString_FromString ( "__main__" ));

  return dict;
}

/**
 * Delete a given Python dictionary object
 *
 * @param dict - dictionary to b ereleased
 */
static void
release_global_dictionary (PyObject *dict)
{
  PyDict_Clear (dict);

  /* Release dictionary */
  Py_DECREF (dict);
}

/**
 * Register module in list
 *
 * @param module - module to be registered
 */
static void
register_module (py_module_t *module)
{
  modules = realloc (modules, (modules_count + 1) * sizeof (py_module_t*));
  modules[modules_count++] = module;
}

/**
 * Unregister module from list
 *
 * @param module - module to be unregistered
 */
static void
unregister_module (py_module_t *module)
{
  long i;
  int found = 0;

  for (i = 0; i < modules_count; ++i)
    {
      if (modules[i] == module)
        {
          found = 1;
          break;
        }
    }

  if (found)
    {
      /* Shift modules */
      if (i < modules_count - 1)
        {
          modules[i] = modules[modules_count - 1];
        }

      --modules_count;
      if (!modules_count)
        {
          free (modules);
          modules = NULL;
        }
      else
        {
          modules = realloc (modules, modules_count * sizeof (py_module_t*));
        }
    }
}

/**
 * Unregister all modules from list
 */
static void
unregister_all_modules (void)
{
  while (modules)
    {
      py_module_free (modules[0]);
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

      MBS2WCS (progname, argv[0]);
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

  init_syspath (first_time);

  if (py_tracer_init ())
    {
      return -1;
    }

  return 0;
}

/**
 * Finalize Python sub-system
 */
void
python_done (void)
{
  py_tracer_done ();

  unregister_all_modules ();

  /* End python */
  Py_Finalize ();

  SAFE_FREE (progname);
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

  module->name = wcsdup (name);
  module->descr = wcsdup (descr);
  module->handle = Py_InitModule3 (mbname, methods_list, mbdescr);
  module->dict = PyModule_GetDict (module->handle);
  module->conv_methods = methods_list;

  SAFE_FREE (mbname);
  SAFE_FREE (mbdescr);

  register_module (module);

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

  unregister_module (module);

  free_methods_list (module->conv_methods);
  Py_DECREF (module->handle);

  SAFE_FREE (module->name);
  SAFE_FREE (module->descr);

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
      free (mbfn);
      return NULL;
    }

  file = fopen (mbfn, "rb");

  if (!file)
    {
      free (mbfn);
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
      free (mbfn);
      return NULL;
    }

  script = py_script_new_buffer (wcbuf);
  script->file_name = wcsdup (file_name);

  free (buffer);
  free (wcbuf);
  free (mbfn);

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
py_run_script_at_dict (py_script_t *script, PyObject *dict)
{
  char *mbfn = NULL;
  PyObject *result;
  char *filename = "";

  if (!script)
    {
      return NULL;
    }

  if (script->file_name)
    {
      WCS2MBS (mbfn, script->file_name);
      filename = mbfn;
    }

  /* Compile script */
  if (!script->compiled)
    {
      char *mbscript;

      WCS2MBS (mbscript, script->script);

      script->compiled = Py_CompileString (mbscript, filename, Py_file_input);
      free (mbscript);

      if (PyErr_Occurred ())
        {
          /* Compilation error occurred */
          PyErr_Print ();
          SAFE_FREE (mbfn);
          py_script_free_compiled (script);
          return NULL;
        }
    }

  extpy_dict_set_item_str (dict, L"__file__",
                           PyString_FromString (filename));

  SAFE_FREE (mbfn);

  PyErr_Clear ();
  result = PyEval_EvalCode ((PyCodeObject*)script->compiled, dict, dict);

  if (PyErr_Occurred ())
    {
      PyErr_Print ();
    }

  return result;
}

/**
 * Run Python script
 *
 * @param script - script to be executed
 * @return Python eval's result
 */
PyObject*
py_run_script (py_script_t *script)
{
  PyObject *dict = create_global_dictionary ();
  PyObject *result;

  result = py_run_script_at_dict (script, dict);

  release_global_dictionary (dict);

  return result;
}

/**
 * Run specified file at specified dictionary
 *
 * @param file_name - name of file to run
 * @param dict - dictionary to run on
 * @return Python eval's result
 */
PyObject*
py_run_file_at_dict (const wchar_t *file_name, PyObject *dict)
{
  py_script_t *script;
  PyObject *result;

  script = py_script_new_file (file_name);

  if (!script)
    {
      return NULL;
    }

  result = py_run_script (script);
  py_script_free (script);

  return result;
}

/**
 * Run specified file
 *
 * @param file_name - name of file to run
 * @return Python eval's result
 */
PyObject*
py_run_file (const wchar_t *file_name)
{
  PyObject *dict = create_global_dictionary ();
  PyObject *result;

  result = py_run_file_at_dict (file_name, dict);

  release_global_dictionary (dict);

  return result;
}

/****
 * Other helpers
 */

/**
 * Add integer constant to module
 *
 * @param module - module to add constant to
 * @param name - constant's name
 * @param value - constant's value
 * @return -1 on error, 0 on success.
 */
int
py_module_add_int_constant (py_module_t *module, wchar_t *name, long value)
{
  char *mbname;
  int res;

  WCS2MBS (mbname, name);

  if (!name)
    {
      return -1;
    }

  res = PyModule_AddIntConstant (module->handle, mbname, value);

  free (mbname);

  return res;
}
