#pragma once

#include <cstdlib>
#include <algorithm>
#include <cassert>

template <typename Type>
class ArrayPtr {
public:

    ArrayPtr() = default;


    explicit ArrayPtr(size_t size) {
        if (size != 0) {
            raw_ptr_ = new Type[size];
        }
    }
    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }

    ArrayPtr(ArrayPtr&& other) {
        raw_ptr_ = std::exchange(other.raw_ptr_, nullptr);
    }

    ArrayPtr& operator=(ArrayPtr&& rhs) {
        if (this == &rhs) {
            return *this;
        }
        std::swap(raw_ptr_, rhs.raw_ptr_);
        return *this;
    }

    ArrayPtr(const ArrayPtr&) = delete;

    ~ArrayPtr() {

        delete[] raw_ptr_;
    }

    ArrayPtr& operator=(const ArrayPtr&) = delete;

    [[nodiscard]] Type* Release() noexcept {
        Type* prev_ptr_ = raw_ptr_;
        raw_ptr_ = nullptr;
        return prev_ptr_;
    }

    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    explicit operator bool() const {
        return raw_ptr_ != nullptr;
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;
};