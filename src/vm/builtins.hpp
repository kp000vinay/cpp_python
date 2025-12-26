#pragma once

#include "pyobject.hpp"
#include <iostream>
#include <memory>
#include <functional>

namespace cpython_cpp {
namespace vm {

/**
 * Built-in print() function
 * 
 * Prints objects to stdout, separated by spaces, ending with newline.
 */
inline PyObject builtin_print(const std::vector<PyObject>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        print_object(args[i]);
    }
    std::cout << "\n";
    return std::monostate{};  // None
}

/**
 * Built-in len() function
 * 
 * Returns the length of a sequence or collection.
 */
inline PyObject builtin_len(const std::vector<PyObject>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("len() takes exactly one argument");
    }
    
    const PyObject& obj = args[0];
    
    if (is_string(obj)) {
        return static_cast<int64_t>(std::get<std::string>(obj).size());
    } else if (is_list(obj)) {
        return static_cast<int64_t>(std::get<std::shared_ptr<PyList>>(obj)->size());
    } else if (is_tuple(obj)) {
        return static_cast<int64_t>(std::get<std::shared_ptr<PyTuple>>(obj)->size());
    } else if (is_dict(obj)) {
        return static_cast<int64_t>(std::get<std::shared_ptr<PyDict>>(obj)->size());
    } else if (is_set(obj)) {
        return static_cast<int64_t>(std::get<std::shared_ptr<PySet>>(obj)->size());
    }
    
    throw std::runtime_error("object has no len()");
}

/**
 * Built-in type() function
 * 
 * Returns the type name of an object.
 */
inline PyObject builtin_type(const std::vector<PyObject>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("type() takes exactly one argument");
    }
    
    const PyObject& obj = args[0];
    
    if (is_none(obj)) return std::string("<class 'NoneType'>");
    if (is_bool(obj)) return std::string("<class 'bool'>");
    if (is_int(obj)) return std::string("<class 'int'>");
    if (is_float(obj)) return std::string("<class 'float'>");
    if (is_string(obj)) return std::string("<class 'str'>");
    if (is_list(obj)) return std::string("<class 'list'>");
    if (is_tuple(obj)) return std::string("<class 'tuple'>");
    if (is_dict(obj)) return std::string("<class 'dict'>");
    if (is_set(obj)) return std::string("<class 'set'>");
    
    return std::string("<class 'object'>");
}

/**
 * Built-in int() function
 * 
 * Converts a value to an integer.
 */
inline PyObject builtin_int(const std::vector<PyObject>& args) {
    if (args.empty() || args.size() > 2) {
        throw std::runtime_error("int() takes 1 or 2 arguments");
    }
    
    return to_int(args[0]);
}

/**
 * Built-in float() function
 * 
 * Converts a value to a float.
 */
inline PyObject builtin_float(const std::vector<PyObject>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("float() takes exactly one argument");
    }
    
    return to_float(args[0]);
}

/**
 * Built-in str() function
 * 
 * Converts a value to a string.
 */
inline PyObject builtin_str(const std::vector<PyObject>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("str() takes exactly one argument");
    }
    
    return to_string(args[0]);
}

/**
 * Built-in bool() function
 * 
 * Converts a value to a boolean.
 */
inline PyObject builtin_bool(const std::vector<PyObject>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("bool() takes exactly one argument");
    }
    
    return to_bool(args[0]);
}

/**
 * Built-in range() function
 * 
 * Returns a list of integers (simplified implementation).
 */
inline PyObject builtin_range(const std::vector<PyObject>& args) {
    int64_t start = 0, stop = 0, step = 1;
    
    if (args.size() == 1) {
        stop = to_int(args[0]);
    } else if (args.size() == 2) {
        start = to_int(args[0]);
        stop = to_int(args[1]);
    } else if (args.size() == 3) {
        start = to_int(args[0]);
        stop = to_int(args[1]);
        step = to_int(args[2]);
    } else {
        throw std::runtime_error("range() takes 1 to 3 arguments");
    }
    
    if (step == 0) {
        throw std::runtime_error("range() step argument must not be zero");
    }
    
    auto list = std::make_shared<PyList>();
    
    if (step > 0) {
        for (int64_t i = start; i < stop; i += step) {
            list->append(i);
        }
    } else {
        for (int64_t i = start; i > stop; i += step) {
            list->append(i);
        }
    }
    
    return list;
}

} // namespace vm
} // namespace cpython_cpp
