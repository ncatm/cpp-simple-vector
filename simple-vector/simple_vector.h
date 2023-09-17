#pragma once

#include <stdexcept>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <utility>
#include <cassert>
#include "array_ptr.h"

struct ReserveItem {
public:
    ReserveItem(const size_t capacity_in) : capacity_(capacity_in) {}

    size_t Size() {
        return capacity_;
    }

private:
    size_t capacity_;
};

ReserveItem Reserve(const size_t in) {
    return ReserveItem(in);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    SimpleVector(ReserveItem item) {
        SimpleVector<Type> result;
        result.Reserve(item.Size());
        swap(result);
    }

    explicit SimpleVector(size_t size) : SimpleVector(size, Type{}) {
    }

    SimpleVector(size_t size, const Type& value) : items_(size), size_(size), capacity_(size) {
        std::fill(items_.Get(), items_.Get() + size, value);
    }

    SimpleVector(std::initializer_list<Type> init) : items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(const SimpleVector& other) : items_(other.size_), size_(other.size_), capacity_(other.size_) {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    SimpleVector(SimpleVector&& other) {
        items_ = std::move(other.items_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            if (rhs.IsEmpty()) {
                Clear();
                return *this;
            }
            SimpleVector<Type> new_vector(rhs.size_);
            std::copy(rhs.begin(), rhs.end(), new_vector.begin());
            new_vector.capacity_ = rhs.capacity_;
            swap(new_vector);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            if (rhs.IsEmpty()) {
                Clear();
                return *this;
            }
            SimpleVector<Type> new_vector(rhs.size_);
            std::copy(rhs.begin(), rhs.end(), new_vector.begin());
            new_vector.capacity_ = rhs.capacity_;
            swap(new_vector);
        }
        return *this;
    }

    void Reserve(size_t reserve_capacity) {
        if (capacity_ < reserve_capacity) {
            SimpleVector<Type> new_vector(reserve_capacity);
            std::copy(cbegin(), cend(), new_vector.begin());
            new_vector.size_ = size_;
            swap(new_vector);
        }
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("out_of_range");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("out_of_range");
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, 2 * capacity_);
            ArrayPtr<Type> new_vector(new_capacity);
            std::move(&items_[0], &items_[size_], &new_vector[0]);
            for (auto it = &new_vector[size_]; it != &new_vector[new_size]; ++it) {
                *it = std::move(Type{});
            }
            items_.swap(new_vector);
            capacity_ = new_capacity;
        }
        size_ = new_size;
    }

    Iterator begin() noexcept {
        return { items_.Get() };
    }

    Iterator end() noexcept {
        return { &items_[size_] };
    }

    ConstIterator begin() const noexcept {
        return { items_.Get() };
    }

    ConstIterator end() const noexcept {
        return { &items_[size_] };
    }

    ConstIterator cbegin() const noexcept {
        return { items_.Get() };
    }

    ConstIterator cend() const noexcept {
        return { &items_[size_] };
    }

    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            items_[size_] = item;
        }
        else {
            auto new_capacity = std::max(size_t(1), 2 * capacity_);
            ArrayPtr<Type> new_vector(new_capacity);
            std::copy(&items_[0], &items_[size_], &new_vector[0]);
            new_vector[size_] = item;
            items_.swap(new_vector);
            capacity_ = new_capacity;
        }
        ++size_;
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            items_[size_] = std::move(item);
        }
        else {
            auto new_capacity = std::max(size_t(1), 2 * capacity_);
            ArrayPtr<Type> new_vector(new_capacity);
            std::move(&items_[0], &items_[size_], &new_vector[0]);
            new_vector[size_] = std::move(item);
            items_.swap(new_vector);
            capacity_ = new_capacity;
        }
        ++size_;
    }

    Iterator Insert(ConstIterator index, const Type& item) {
        assert(cbegin() <= index && index <= cend());

        auto item_pos = std::distance(cbegin(), index);

        if (size_ < capacity_) {
            std::copy_backward(index, cend(), &items_[size_ + 1]);
            items_[item_pos] = item;
        }
        else {
            auto new_capacity = std::max(size_t(1), 2 * capacity_);
            ArrayPtr<Type> new_vector(new_capacity);
            std::copy(&items_[0], &items_[item_pos], &new_vector[0]);
            std::copy_backward(index, cend(), &new_vector[(size_ + 1)]);
            new_vector[size_] = item;
            items_.swap(new_vector);
            capacity_ = new_capacity;
        }

        ++size_;
        return { &items_[item_pos] };
    }

    Iterator Insert(ConstIterator index, Type&& item) {
        assert(cbegin() <= index && index <= cend());

        auto item_pos = const_cast<Iterator>(index);
        auto pos_item = std::distance(begin(), item_pos);

        if (size_ < capacity_) {
            std::move_backward(item_pos, end(), &items_[(size_ + 1)]);
            items_[pos_item] = std::move(item);
        }
        else {
            auto new_capacity = std::max(size_t(1), 2 * capacity_);
            ArrayPtr<Type> arr_ptr(new_capacity);
            std::move(&items_[0], &items_[pos_item], &arr_ptr[0]);
            std::move_backward(item_pos, end(), &arr_ptr[(size_ + 1)]);
            arr_ptr[pos_item] = std::move(item);
            items_.swap(arr_ptr);
            capacity_ = new_capacity;
        }

        ++size_;
        return Iterator{ &items_[pos_item] };
    }

    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    Iterator Erase(ConstIterator index) noexcept {
        assert(index >= cbegin() && index < cend());
        auto pos = const_cast<Iterator>(index);
        auto pos_element = std::distance(begin(), pos);
        std::move(++pos, end(), &items_[pos_element]);
        --size_;
        return { &items_[pos_element] };
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0u;
    size_t capacity_ = 0u;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs < rhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}