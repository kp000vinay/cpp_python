#ifndef CPYTHON_CPP_REFCOUNT_HPP
#define CPYTHON_CPP_REFCOUNT_HPP

#include <atomic>
#include <cstdint>

namespace cpython_cpp {
namespace core {

/**
 * Reference counting base class
 *
 * This module can be developed independently to understand
 * CPython's reference counting mechanism.
 *
 * Reference: Objects/object.c, Include/object.h
 */
class RefCounted {
public:
    RefCounted() : ref_count_(1) {}
    virtual ~RefCounted() = default;

    // Increment reference count
    void incref() {
        ref_count_.fetch_add(1, std::memory_order_relaxed);
    }

    // Decrement reference count
    // Returns true if object should be deallocated
    bool decref() {
        auto old_count = ref_count_.fetch_sub(1, std::memory_order_acq_rel);
        return old_count == 1;
    }

    // Get current reference count
    int64_t get_ref_count() const {
        return ref_count_.load(std::memory_order_acquire);
    }

protected:
    // Virtual method called when refcount reaches 0
    virtual void dealloc() {
        delete this;
    }

private:
    std::atomic<int64_t> ref_count_;
};

} // namespace core
} // namespace cpython_cpp

#endif // CPYTHON_CPP_REFCOUNT_HPP







