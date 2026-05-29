#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>

#include "allocator.hpp"

struct ReserveProxyObj {
    ReserveProxyObj(size_t new_capacity) : capacity(new_capacity) {}
    size_t capacity;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) { return ReserveProxyObj(capacity_to_reserve); }

template <typename Type>
class Vector {
private:
    ArrayPtr<Type> items_;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;

public:
    using iterator = Type*;
    using const_iterator = const Type*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() noexcept { return items_.Get(); }
    iterator end() noexcept { return items_.Get() + size_; }
    const_iterator begin() const noexcept { return items_.Get(); }
    const_iterator end() const noexcept { return items_.Get() + size_; }
    const_iterator cbegin() const noexcept { return items_.Get(); }
    const_iterator cend() const noexcept { return items_.Get() + size_; }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

    std::size_t GetSize() const noexcept { return size_; }
    size_t GetCapacity() const noexcept { return capacity_; }
    bool IsEmpty() const noexcept { return size_ == 0; }
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        } else if (new_size <= capacity_) {
            for (size_t i = size_; i < new_size; ++i) {
                items_[i] = Type(); // Initialize with default value
            }
            size_ = new_size;
        } else {
            Vector tmp(new_size);
            for (size_t i = 0; i < size_; ++i) {
                tmp[i] = std::move(items_[i]);
            }
            swap(tmp);
        }
    }

    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : 2 * capacity_);
        }
        items_[size_++] = item;
    }
    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : 2 * capacity_);
        }
        items_[size_++] = std::move(item);
    }
    iterator Insert(const_iterator pos, Type&& value) {
        assert(begin() <= pos && pos <= end());
        size_t index = pos - cbegin();
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : 2 * capacity_);
        }
        if (index < size_) {
            std::move_backward(begin() + index, end(), end() + 1);
        }
        items_[index] = std::move(value);
        ++size_;
        return begin() + index;
    }

    iterator Insert(const_iterator pos, const Type& value) {
        assert(begin() <= pos && pos <= end());
        size_t index = pos - cbegin();
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : 2 * capacity_);
        }
        if (index < size_) {
            std::move_backward(begin() + index, end(), end() + 1);
        }
        items_[index] = value;
        ++size_;
        return begin() + index;
    }

    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    iterator Erase(const_iterator pos) {
        assert(begin() <= pos && pos <= end());
        std::size_t index = pos - cbegin();
        if (index < size_) {
            std::move(begin() + index + 1, end(), begin() + index);
            --size_;
        }
        return begin() + index;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_items(new_capacity);
            std::move(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return items_[index];
    }
    void swap(Vector& other) noexcept {
        std::swap(items_, other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    void Clear() noexcept { size_ = 0; }

    Vector() = default;
    explicit Vector(std::size_t size) : items_(size), size_(size), capacity_(size) {}

    Vector(ReserveProxyObj reserved) : items_(reserved.capacity), size_(0), capacity_(reserved.capacity) {}

    Vector(size_t size, const Type& value) : items_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), value);
    }

    Vector(std::initializer_list<Type> init) : items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }

    Vector(const Vector& other) : items_(other.capacity_), size_(other.size_), capacity_(other.capacity_) {
        std::copy(other.begin(), other.end(), begin());
    }

    Vector(Vector&& other) noexcept
        : items_(std::move(other.items_)), size_(other.size_), capacity_(other.capacity_) {
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Vector& operator=(Vector&& rhs) noexcept {
        if (this != &rhs) {
            size_ = rhs.size_;
            capacity_ = rhs.capacity_;
            items_ = std::move(rhs.items_);

            rhs.size_ = 0;
            rhs.capacity_ = 0;
        }
        return *this;
    }
    Vector& operator=(const Vector& rhs) {
        if (this != &rhs) {
            Vector copy(rhs);
            swap(copy);
        }
        return *this;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }
};

template <typename Type>
inline bool operator==(const Vector<Type>& lhs, const Vector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const Vector<Type>& lhs, const Vector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const Vector<Type>& lhs, const Vector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator>(const Vector<Type>& lhs, const Vector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator<=(const Vector<Type>& lhs, const Vector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>=(const Vector<Type>& lhs, const Vector<Type>& rhs) {
    return !(lhs < rhs);
}
