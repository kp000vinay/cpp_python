#pragma once

#include <variant>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>
#include <cmath>

namespace cpython_cpp {
namespace vm {

// Forward declarations
class PyList;
class PyDict;
class PyTuple;
class PySet;
class PyFunction;
class PyClass;
class PyInstance;

/**
 * PyObject - Runtime representation of Python objects
 * 
 * Uses std::variant to represent different Python types.
 * This is the core value type used throughout the VM.
 */
using PyObject = std::variant<
    std::monostate,                    // None
    bool,                              // True, False
    int64_t,                           // int
    double,                            // float
    std::string,                       // str
    std::shared_ptr<PyList>,           // list
    std::shared_ptr<PyDict>,           // dict
    std::shared_ptr<PyTuple>,          // tuple
    std::shared_ptr<PySet>,            // set
    std::shared_ptr<PyFunction>,       // function
    std::shared_ptr<PyClass>,          // class
    std::shared_ptr<PyInstance>        // instance
>;

/**
 * PyList - Python list type
 */
class PyList {
public:
    std::vector<PyObject> items;
    
    PyList() = default;
    explicit PyList(std::vector<PyObject> items) : items(std::move(items)) {}
    
    size_t size() const { return items.size(); }
    void append(const PyObject& obj) { items.push_back(obj); }
    PyObject& operator[](size_t index) { return items[index]; }
    const PyObject& operator[](size_t index) const { return items[index]; }
};

/**
 * PyDict - Python dictionary type
 */
class PyDict {
public:
    std::map<std::string, PyObject> items;
    
    PyDict() = default;
    
    void set(const std::string& key, const PyObject& value) {
        items[key] = value;
    }
    
    PyObject get(const std::string& key, const PyObject& default_value = std::monostate{}) const {
        auto it = items.find(key);
        return (it != items.end()) ? it->second : default_value;
    }
    
    bool contains(const std::string& key) const {
        return items.find(key) != items.end();
    }
    
    size_t size() const { return items.size(); }
};

/**
 * PyTuple - Python tuple type (immutable)
 */
class PyTuple {
public:
    std::vector<PyObject> items;
    
    PyTuple() = default;
    explicit PyTuple(std::vector<PyObject> items) : items(std::move(items)) {}
    
    size_t size() const { return items.size(); }
    const PyObject& operator[](size_t index) const { return items[index]; }
};

/**
 * PySet - Python set type
 */
class PySet {
public:
    std::vector<PyObject> items;  // Simplified: use vector instead of hash set
    
    PySet() = default;
    
    void add(const PyObject& obj) {
        // Simple implementation: no duplicate checking for now
        items.push_back(obj);
    }
    
    size_t size() const { return items.size(); }
};

/**
 * PyFunction - Python function object
 */
class PyFunction {
public:
    std::shared_ptr<struct CodeObject> code;  // Function code object
    std::shared_ptr<PyDict> globals;          // Global namespace
    std::shared_ptr<PyDict> closure;          // Closure variables
    std::string name;
    
    PyFunction(std::shared_ptr<struct CodeObject> code,
               std::shared_ptr<PyDict> globals,
               const std::string& name = "<lambda>")
        : code(std::move(code))
        , globals(std::move(globals))
        , closure(std::make_shared<PyDict>())
        , name(name) {}
};

/**
 * Type checking helpers
 */
inline bool is_none(const PyObject& obj) {
    return std::holds_alternative<std::monostate>(obj);
}

inline bool is_bool(const PyObject& obj) {
    return std::holds_alternative<bool>(obj);
}

inline bool is_int(const PyObject& obj) {
    return std::holds_alternative<int64_t>(obj);
}

inline bool is_float(const PyObject& obj) {
    return std::holds_alternative<double>(obj);
}

inline bool is_string(const PyObject& obj) {
    return std::holds_alternative<std::string>(obj);
}

inline bool is_list(const PyObject& obj) {
    return std::holds_alternative<std::shared_ptr<PyList>>(obj);
}

inline bool is_dict(const PyObject& obj) {
    return std::holds_alternative<std::shared_ptr<PyDict>>(obj);
}

inline bool is_tuple(const PyObject& obj) {
    return std::holds_alternative<std::shared_ptr<PyTuple>>(obj);
}

inline bool is_set(const PyObject& obj) {
    return std::holds_alternative<std::shared_ptr<PySet>>(obj);
}

/**
 * Type conversion helpers
 */
inline int64_t to_int(const PyObject& obj) {
    if (is_int(obj)) return std::get<int64_t>(obj);
    if (is_bool(obj)) return std::get<bool>(obj) ? 1 : 0;
    if (is_float(obj)) return static_cast<int64_t>(std::get<double>(obj));
    throw std::runtime_error("Cannot convert to int");
}

inline double to_float(const PyObject& obj) {
    if (is_float(obj)) return std::get<double>(obj);
    if (is_int(obj)) return static_cast<double>(std::get<int64_t>(obj));
    if (is_bool(obj)) return std::get<bool>(obj) ? 1.0 : 0.0;
    throw std::runtime_error("Cannot convert to float");
}

inline bool to_bool(const PyObject& obj) {
    if (is_bool(obj)) return std::get<bool>(obj);
    if (is_none(obj)) return false;
    if (is_int(obj)) return std::get<int64_t>(obj) != 0;
    if (is_float(obj)) return std::get<double>(obj) != 0.0;
    if (is_string(obj)) return !std::get<std::string>(obj).empty();
    if (is_list(obj)) return std::get<std::shared_ptr<PyList>>(obj)->size() > 0;
    if (is_dict(obj)) return std::get<std::shared_ptr<PyDict>>(obj)->size() > 0;
    return true;  // Most objects are truthy
}

inline std::string to_string(const PyObject& obj) {
    if (is_none(obj)) return "None";
    if (is_bool(obj)) return std::get<bool>(obj) ? "True" : "False";
    if (is_int(obj)) return std::to_string(std::get<int64_t>(obj));
    if (is_float(obj)) {
        std::ostringstream oss;
        oss << std::get<double>(obj);
        return oss.str();
    }
    if (is_string(obj)) return std::get<std::string>(obj);
    if (is_list(obj)) {
        auto list = std::get<std::shared_ptr<PyList>>(obj);
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < list->size(); ++i) {
            if (i > 0) oss << ", ";
            oss << to_string((*list)[i]);
        }
        oss << "]";
        return oss.str();
    }
    if (is_tuple(obj)) {
        auto tuple = std::get<std::shared_ptr<PyTuple>>(obj);
        std::ostringstream oss;
        oss << "(";
        for (size_t i = 0; i < tuple->size(); ++i) {
            if (i > 0) oss << ", ";
            oss << to_string((*tuple)[i]);
        }
        if (tuple->size() == 1) oss << ",";
        oss << ")";
        return oss.str();
    }
    if (is_dict(obj)) {
        auto dict = std::get<std::shared_ptr<PyDict>>(obj);
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        for (const auto& [key, value] : dict->items) {
            if (!first) oss << ", ";
            oss << "'" << key << "': " << to_string(value);
            first = false;
        }
        oss << "}";
        return oss.str();
    }
    return "<object>";
}

/**
 * Print a PyObject to stdout (for debugging and print() function)
 */
inline void print_object(const PyObject& obj) {
    std::cout << to_string(obj);
}

} // namespace vm
} // namespace cpython_cpp
