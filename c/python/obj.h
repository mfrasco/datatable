//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// © H2O.ai 2018
//------------------------------------------------------------------------------
#ifndef dt_PYTHON_OBJ_h
#define dt_PYTHON_OBJ_h
#include <string>
#include <vector>
#include <Python.h>
#include "types.h"             // CString
#include "utils/exceptions.h"  // Error

class Column;
class DataTable;
class Groupby;
class RowIndex;

namespace py {

// Forward declarations
class Arg;
class odict;
class ofloat;
class oint;
class oiter;
class olist;
class ostring;
class orange;
class otuple;
class robj;
class rdict;
class oobj;
using strvec = std::vector<std::string>;


/**
 * This class is a C++ wrapper around pythonic C struct `PyObject*`. Its main
 * purpose is to provide type checks and conversions into native C++
 * primitives / objects.
 *
 * `py::_obj` by itself is not usable: instead, you should use one of the two
 * derived classes:
 *   - `py::robj` contains a *borrowed PyObject reference*. This class is used
 *     most commonly for objects that have a very small lifespan. DO NOT use
 *     this class to store `PyObject*`s for an extended period of time.
 *   - `py::oobj` contains an *owned PyObject reference*. The value that it
 *     wraps will not be garbage-collected as long as the `py::oobj` object
 *     remains alive. This class bears extra performance cost compared to
 *     `py::robj` (however this extra cost is very small).
 *
 *
 * Conversion methods
 * ==================
 *
 * to_bool:
 *    Converts None -> NA, True or 1 -> true, False or 0 -> false, or raises
 *    an error otherwise.
 *
 * to_bool_strict:
 *    Converts True -> true, False -> false, otherwise raises an error.
 *
 * to_bool_force:  [noexcept]
 *    Converts None into NA, otherwise converts into true / false using the
 *    python `bool(value)` call (if it raises an exception, then the value
 *    is converted into an NA).
 *
 *
 * to_int32, to_int64:
 *    Convert None into NA, or an integer value into either int32_t or int64_t
 *    C++ primitive. True and False are treated as integers 1 and 0. All other
 *    value types raise an exception. If the pythonic integer is too large
 *    to fit into 32/64 bits, then it will be replaced with the largest 32/64
 *    bit integer (taking the sign into account).
 *
 * to_int32_strict, to_int64_strict:
 *    Convert a pythonic integer into either an int32_t or int64_t C++ value.
 *    All non-ints (including None, True and False) cause an exception. If the
 *    conversion causes an integer overflow, it also causes an exception.
 *
 * to_pyint:
 *    Convert into a py::Int instance, or raise an exception if the object is
 *    not a python int or None. A `py::Int` object offers many additional ways
 *    to convert the value into C++ primitives.
 *
 * to_pyint_force:  [noexcept]
 *    Similar to `to_pyint`, but it will also attempt to convert its argument
 *    into an int using python `int(x)` function. If this function raises an
 *    error, the value will be converted into an NA.
 *
 *
 * to_double:
 *    Convert None into NA, or python float or int into a C++ double value. An
 *    overflow may occur if the underlying value is a large integer (i.e.
 *    10**500). If the object is neither python float nor int nor None, an
 *    exception is raised.
 *
 * to_pyfloat_force:  [noexcept]
 *    Convert into a py::ofloat instance, possibly applying pythonic `float(x)`
 *    function. If the function raises an exception, the value will be
 *    converted into NA.
 *
 *
 * to_cstring:
 *    Convert a string or bytes object into a `CString` struct. Python None is
 *    converted into an NA string, which is encoded as {.size=0, .ch=nullptr}.
 *    All other objects cause an exception to be thrown.
 *    The pointer in the returned `CString` struct is borrowed: its lifespan is
 *    controlled by the lifespan of the underlying PyObject.
 *
 * to_string:
 *    Similar to `to_cstring`, but the string is copied into an `std::string`
 *    object. If the value is NA, then an empty `std::string` is returned
 *    (std::string has no concept of missing strings).
 *
 * to_pystring_force:  [noexcept]
 *    Convert into a `py::ostring` object. If the object is not string/bytes,
 *    then an attempt is made to stringify it, using pythonic `str()` call.
 *    If `str()` raises an error, the object is converted into NA.
 */
class _obj {
  protected:
    PyObject* v;

  public:
    oobj get_attr(const char* attr) const;
    bool has_attr(const char* attr) const;
    oobj get_item(const py::_obj& key) const;
    oobj invoke(const char* fn, const char* format, ...) const;
    oobj call(otuple args) const;
    oobj call(otuple args, odict kws) const;
    ostring str() const;
    PyTypeObject* typeobj() const noexcept;  // borrowed ref

    explicit operator bool() const noexcept;  // opposite of is_undefined()
    bool operator==(const _obj& other) const noexcept;
    bool operator!=(const _obj& other) const noexcept;

    //--------------------------------------------------------------------------
    // Type tests
    //--------------------------------------------------------------------------
    bool is_undefined()     const noexcept;
    bool is_none()          const noexcept;
    bool is_ellipsis()      const noexcept;
    bool is_true()          const noexcept;
    bool is_false()         const noexcept;
    bool is_bool()          const noexcept;
    bool is_int()           const noexcept;
    bool is_float()         const noexcept;
    bool is_numeric()       const noexcept;
    bool is_string()        const noexcept;
    bool is_list()          const noexcept;
    bool is_tuple()         const noexcept;
    bool is_list_or_tuple() const noexcept;
    bool is_iterable()      const noexcept;
    bool is_dict()          const noexcept;
    bool is_buffer()        const noexcept;
    bool is_range()         const noexcept;
    bool is_frame()         const noexcept;
    bool is_pandas_frame()  const noexcept;
    bool is_pandas_series() const noexcept;
    bool is_numpy_array()   const noexcept;
    bool is_numpy_marray()  const noexcept;

    struct error_manager;  // see below
    int8_t      to_bool          (const error_manager& = _em0) const;
    int8_t      to_bool_strict   (const error_manager& = _em0) const;
    int8_t      to_bool_force    (const error_manager& = _em0) const noexcept;

    int32_t     to_int32         (const error_manager& = _em0) const;
    int64_t     to_int64         (const error_manager& = _em0) const;
    int32_t     to_int32_strict  (const error_manager& = _em0) const;
    int64_t     to_int64_strict  (const error_manager& = _em0) const;
    size_t      to_size_t        (const error_manager& = _em0) const;
    py::oint    to_pyint         (const error_manager& = _em0) const;
    py::oint    to_pyint_force   (const error_manager& = _em0) const noexcept;

    double      to_double        (const error_manager& = _em0) const;
    py::ofloat  to_pyfloat_force (const error_manager& = _em0) const noexcept;

    CString     to_cstring       (const error_manager& = _em0) const;
    std::string to_string        (const error_manager& = _em0) const;
    py::ostring to_pystring_force(const error_manager& = _em0) const noexcept;

    char**      to_cstringlist   (const error_manager& = _em0) const;
    strvec      to_stringlist    (const error_manager& = _em0) const;
    py::olist   to_pylist        (const error_manager& = _em0) const;
    py::otuple  to_pytuple       (const error_manager& = _em0) const;
    py::odict   to_pydict        (const error_manager& = _em0) const;
    py::rdict   to_rdict         (const error_manager& = _em0) const;
    py::orange  to_pyrange       (const error_manager& = _em0) const;
    py::oiter   to_pyiter        (const error_manager& = _em0) const;

    Column*     to_column        (const error_manager& = _em0) const;
    Groupby*    to_groupby       (const error_manager& = _em0) const;
    RowIndex    to_rowindex      (const error_manager& = _em0) const;
    DataTable*  to_frame         (const error_manager& = _em0) const;
    SType       to_stype         (const error_manager& = _em0) const;

    PyObject*   to_pyobject_newref() const noexcept;
    PyObject*   to_borrowed_ref() const { return v; }

    /**
     * `error_manager` is a factory function for different error messages. It
     * is used to customize error messages when they are thrown from an `Arg`
     * instance.
     */
    struct error_manager {
      error_manager() = default;
      error_manager(const error_manager&) = default;
      virtual ~error_manager() {}
      virtual Error error_not_boolean    (PyObject*) const;
      virtual Error error_not_integer    (PyObject*) const;
      virtual Error error_not_double     (PyObject*) const;
      virtual Error error_not_string     (PyObject*) const;
      virtual Error error_not_groupby    (PyObject*) const;
      virtual Error error_not_rowindex   (PyObject*) const;
      virtual Error error_not_frame      (PyObject*) const;
      virtual Error error_not_column     (PyObject*) const;
      virtual Error error_not_list       (PyObject*) const;
      virtual Error error_not_dict       (PyObject*) const;
      virtual Error error_not_range      (PyObject*) const;
      virtual Error error_not_stype      (PyObject*) const;
      virtual Error error_not_iterable   (PyObject*) const;
      virtual Error error_int32_overflow (PyObject*) const;
      virtual Error error_int64_overflow (PyObject*) const;
      virtual Error error_double_overflow(PyObject*) const;
      virtual Error error_int_negative   (PyObject*) const;
    };

  protected:
    static error_manager _em0;

    // `_obj` class is not directly constructible: create either `robj` or
    // `oobj` objects instead.
    _obj() = default;

    friend robj;
    friend oobj;
    friend odict;
    friend rdict;
    friend Arg;
    friend oobj get_module(const char* name);
};



class robj : public _obj {
  public:
    robj(const PyObject* p);
    robj(const Arg&);
    robj(const robj&);
    robj(const oobj&);
    robj& operator=(const robj&);
    robj& operator=(const _obj&);
};



class oobj : public _obj {
  public:
    oobj();
    oobj(PyObject* p);
    oobj(const oobj&);
    oobj(const robj&);
    oobj(oobj&&);
    oobj& operator=(const oobj&);
    oobj& operator=(oobj&&);
    ~oobj();

    static oobj from_new_reference(PyObject* p);

    // Return the underlying PyObject*, as a new ref, but invalidating this
    // object in the process. Use as:
    //    std::move(x).release();
    //
    PyObject* release() &&;
};


/**
 * Return python constants None, True, False wrapped as `oobj`s.
 */
oobj None();
oobj True();
oobj False();
oobj Ellipsis();
robj rnone();


}  // namespace py

#endif
