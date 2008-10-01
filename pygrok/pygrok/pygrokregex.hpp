#include <boost/xpressive/xpressive.hpp>
#include "../../grokregex.hpp"
#include "../../grokpatternset.hpp"

#include <Python.h>
#include <structmember.h>

static PyObject *
MatchToDict(GrokMatch<sregex> &gm) {
  GrokMatch<sregex>::match_map_type::const_iterator iter;
  GrokMatch<sregex>::match_map_type m;
  PyObject *match_dict = PyDict_New();

  m = gm.GetMatches();

  for (iter = m.begin(); iter != m.end(); iter++) {
    PyObject *key, *val;
    key = PyString_FromString((*iter).first.c_str());
    val = PyString_FromString((*iter).second.c_str());
    PyDict_SetItem(match_dict, key, val);
    Py_DECREF(key);
    Py_DECREF(val);
  }

  return match_dict;
}

typedef struct {
  PyObject_HEAD
  GrokRegex<sregex> *gre;
  GrokPatternSet<sregex> *pattern_set;
} pyGrokRegex;

static PyObject *
pyGrokRegex_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  pyGrokRegex *self = NULL;

  self = (pyGrokRegex *)type->tp_alloc(type, 0);

  if (!PyArg_UnpackTuple(args, "GrokRegex.__init__", 0, 0))
    return NULL;

  if (self != NULL) {
    self->gre = new GrokRegex<sregex>;
    self->pattern_set = new GrokPatternSet<sregex>;
  }

  return (PyObject *)self;
}

static void
pyGrokRegex_dealloc(pyGrokRegex *self) {
  delete self->gre;
  delete self->pattern_set;
  self->ob_type->tp_free((PyObject*) self);
}

static PyObject *
pyGrokRegex_search(pyGrokRegex *self, PyObject *args) {
  PyObject* search_pystr = NULL;
  char *search_pchar = NULL;
  bool result;
  GrokMatch<sregex> gm;

  if (!PyArg_UnpackTuple(args, "GrokRegex.search", 1, 1, &search_pystr))
    return NULL;

  search_pchar = PyString_AsString(search_pystr);
  string search_string(search_pchar);
  result = self->gre->Search(search_string, gm);

  if (!result)
    Py_RETURN_NONE;

  return MatchToDict(gm);
}

static PyObject* pyGrokRegex_set_regex(pyGrokRegex *self, PyObject *args) {
  PyObject* regex_pystr = NULL;
  char *regex_pchar = NULL;

  if (!PyArg_UnpackTuple(args, "GroKRegex.set_regex", 1, 1, &regex_pystr))
    return NULL;

  regex_pchar = PyString_AsString(regex_pystr);
  try {
    self->gre->SetRegex((const char *)regex_pchar);
  } catch (boost::xpressive::regex_error e) {
    PyErr_Format(PyExc_ValueError, "Invalid regexp: '%s'", regex_pchar);
    return NULL;
  }

  Py_RETURN_NONE;
}

static PyObject*
pyGrokRegex_add_patterns(pyGrokRegex *self, PyObject *args) {
  PyObject *pattern_dict = NULL;
  PyObject *item_list = NULL;

  if (!PyArg_UnpackTuple(args, "GrokRegex.add_patterns", 1, 1, &pattern_dict))
    return NULL;

  // python 2.5 uses Py_ssize_t
  //Py_ssize_t dict_len;

  // python 2.4 uses int
  int dict_len;
  item_list = PyDict_Items(pattern_dict);

  dict_len = PyList_Size(item_list);
  GrokPatternSet<sregex> gps;
  for (int i = 0; i < dict_len; i++) {
    PyObject *item = PyList_GetItem(item_list, i);
    PyObject *name = PyTuple_GetItem(item, 0);
    PyObject *pattern = PyTuple_GetItem(item, 1);
    string name_str(PyString_AsString(name));
    string pattern_str(PyString_AsString(pattern));
    //cout << "Adding: " << name_str << " => " << pattern_str << endl;
    gps.AddPattern(name_str, pattern_str);
  }

  self->gre->AddPatternSet(gps);

  Py_RETURN_NONE;
}

static PyObject*
pyGrokRegex_get_expanded_regex(pyGrokRegex *self, PyObject *args) {
  if (!PyArg_UnpackTuple(args, "GrokRegex.get_expanded_regex", 0, 0))
    return NULL;

  PyObject *result;
  string tmp(self->gre->GetExpandedPattern());
  result = PyString_FromString(tmp.c_str());

  return result;
}

static PyMethodDef pyGrokRegex_methods[] = {
  {"search", (PyCFunction)pyGrokRegex_search, METH_VARARGS},
  {"set_regex", (PyCFunction)pyGrokRegex_set_regex, METH_VARARGS},
  {"add_patterns", (PyCFunction)pyGrokRegex_add_patterns, METH_VARARGS},
  {"get_expanded_regex", (PyCFunction)pyGrokRegex_get_expanded_regex, METH_VARARGS},
  {NULL, NULL, 0, NULL},
};

static PyMemberDef pyGrokRegex_members[] = {
  {NULL},
};

static PyTypeObject pyGrokRegexType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pygrok.GrokRegex",             /*tp_name*/
    sizeof(pyGrokRegex), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)pyGrokRegex_dealloc,  /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "GrokRegex objects",      /* tp_doc */
    0,                   /* tp_traverse */
    0,                   /* tp_clear */
    0,                   /* tp_richcompare */
    0,                   /* tp_weaklistoffset */
    0,                   /* tp_iter */
    0,                   /* tp_iternext */
    pyGrokRegex_methods,             /* tp_methods */
    pyGrokRegex_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,
    //(initproc)pyGrokRegex_init,      /* tp_init */
    0,                         /* tp_alloc */
    pyGrokRegex_new,                 /* tp_new */
};

