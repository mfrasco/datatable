//------------------------------------------------------------------------------
// Copyright 2018 H2O.ai
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//------------------------------------------------------------------------------
// See https://docs.python.org/3/c-api/dict.html
// for the details of Python API
//------------------------------------------------------------------------------
#include "python/dict.h"

namespace py {


//------------------------------------------------------------------------------
// Constructors
//------------------------------------------------------------------------------

odict::odict() {
  v = PyDict_New();
  if (!v) throw PyError();
}

// private constructors
odict::odict(const robj& src) : oobj(src) {}
odict::odict(const oobj& src) : oobj(src) {}

rdict::rdict(const robj& src) : robj(src) {}
rdict::rdict(const oobj& src) : robj(src) {}



//------------------------------------------------------------------------------
// Element accessors
//------------------------------------------------------------------------------

size_t odict::size() const {
  return static_cast<size_t>(PyDict_Size(v));
}

size_t rdict::size() const {
  return static_cast<size_t>(PyDict_Size(v));
}


bool odict::empty() const {
  return PyDict_Size(v) == 0;
}

bool rdict::empty() const {
  return PyDict_Size(v) == 0;
}


bool odict::has(_obj key) const {
  // Note: PyDict_GetItem() suppresses exceptions. It returns a borrowed ref
  //       to the value, or NULL if key is not present
  return PyDict_GetItem(v, key.v) != nullptr;
}

bool rdict::has(_obj key) const {
  // Note: PyDict_GetItem() suppresses exceptions. It returns a borrowed ref
  //       to the value, or NULL if key is not present
  return PyDict_GetItem(v, key.v) != nullptr;
}


robj odict::get(_obj key) const {
  return robj(PyDict_GetItem(v, key.v));
}

robj rdict::get(_obj key) const {
  return robj(PyDict_GetItem(v, key.v));
}


robj odict::get_or_none(_obj key) const {
  PyObject* res = PyDict_GetItem(v, key.v);
  if (!res) res = Py_None;
  return robj(res);
}

robj rdict::get_or_none(_obj key) const {
  PyObject* res = PyDict_GetItem(v, key.v);
  if (!res) res = Py_None;
  return robj(res);
}


void odict::set(_obj key, _obj val) {
  // PyDict_SetItem INCREFs both key and value internally
  int r = PyDict_SetItem(v, key.v, val.v);
  if (r) throw PyError();
}


void odict::del(_obj key) {
  int r = PyDict_DelItem(v, key.v);
  if (r) throw PyError();
}


dict_iterator odict::begin() const {
  return dict_iterator(v, 0);
}

dict_iterator odict::end() const {
  return dict_iterator(v, -1);
}

dict_iterator rdict::begin() const {
  return dict_iterator(v, 0);
}

dict_iterator rdict::end() const {
  return dict_iterator(v, -1);
}



//------------------------------------------------------------------------------
// dict_iterator
//------------------------------------------------------------------------------

dict_iterator::dict_iterator(PyObject* p, Py_ssize_t i0)
  : dict(p), pos(i0), curr_value(robj(nullptr), robj(nullptr))
{
  advance();
}

dict_iterator& dict_iterator::operator++() {
  advance();
  return *this;
}

dict_iterator::value_type dict_iterator::operator*() const {
  return curr_value;
}

bool dict_iterator::operator==(const dict_iterator& other) const {
  return (pos == other.pos);
}

bool dict_iterator::operator!=(const dict_iterator& other) const {
  return (pos != other.pos);
}

void dict_iterator::advance() {
  if (pos == -1) return;
  PyObject *key, *value;
  if (PyDict_Next(dict, &pos, &key, &value)) {
    curr_value = value_type(py::robj(key), py::robj(value));
  } else {
    pos = -1;
  }
}


}
