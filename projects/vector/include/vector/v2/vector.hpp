/**
 * @file Vector.h
 * @brief A custom vector implementation with raw memory management
 */

#pragma once
#include <cassert>
#include <cstdlib>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

/**
 * @class RawMemory
 * @brief A low-level memory management class for uninitialized storage
 * @tparam _Ty Type of elements to store in memory
 *
 * This class provides basic memory allocation/deallocation functionality
 * without constructing or destroying objects. It's used as a building block
 * for higher-level containers.
 */
template <typename _Ty>
class RawMemory {
public:
    using value_type = _Ty;             ///< Type of elements
    using pointer = _Ty*;               ///< Pointer to element type
    using reference = _Ty&;             ///< Reference to element type
    using const_pointer = const _Ty*;   ///< Const pointer to element type
    using const_reference = const _Ty&; ///< Const reference to element type
    using size_type = std::size_t;      ///< Size type

    /**
     * @brief Default constructor
     */
    RawMemory() = default;

    /**
     * @brief Constructor with capacity
     * @param capacity Number of elements to allocate memory for
     */
    explicit RawMemory(size_type capacity) : buffer_(Allocate(capacity)), capacity_(capacity) {}

    /**
     * @brief Move constructor
     * @param other RawMemory to move from
     */
    RawMemory(RawMemory&& other) noexcept : buffer_(other.buffer_), capacity_(other.capacity_) {
        other.buffer_ = nullptr;
        other.capacity_ = 0;
    }

    /**
     * @brief Move assignment operator
     * @param other RawMemory to move from
     * @return Reference to this object
     */
    RawMemory& operator=(RawMemory&& other) noexcept {
        if (this != &other) {
            Deallocate(buffer_);
            buffer_ = other.buffer_;
            capacity_ = other.capacity_;
            other.buffer_ = nullptr;
            other.capacity_ = 0;
        }
        return *this;
    }

    RawMemory(const RawMemory&) = delete;            ///< Copy constructor is deleted
    RawMemory& operator=(const RawMemory&) = delete; ///< Copy assignment is deleted

    /**
     * @brief Destructor - deallocates memory
     */
    ~RawMemory() { Deallocate(buffer_); }

    /**
     * @brief Pointer arithmetic operator
     * @param offset Offset from start of buffer
     * @return Pointer to offset position
     */
    pointer operator+(size_t offset) noexcept {
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    /**
     * @brief Const pointer arithmetic operator
     * @param offset Offset from start of buffer
     * @return Const pointer to offset position
     */
    const_pointer operator+(size_t offset) const noexcept { return const_cast<RawMemory&>(*this) + offset; }

    /**
     * @brief Subscript operator
     * @param index Position to access
     * @return Reference to element at index
     */
    reference operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    /**
     * @brief Const subscript operator
     * @param index Position to access
     * @return Const reference to element at index
     */
    const_reference operator[](size_t index) const noexcept { return const_cast<RawMemory&>(*this)[index]; }

    /**
     * @brief Swap contents with another RawMemory
     * @param other RawMemory to swap with
     */
    void Swap(RawMemory& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    /**
     * @brief Get buffer address
     * @return Const pointer to buffer
     */
    const_pointer GetAddress() const noexcept { return buffer_; }

    /**
     * @brief Get buffer address
     * @return Pointer to buffer
     */
    pointer GetAddress() noexcept { return buffer_; }

    /**
     * @brief Get capacity
     * @return Capacity in elements
     */
    size_type Capacity() const noexcept { return capacity_; }

    /**
     * @class Traits
     * @brief Provides construction/destruction operations for elements
     */
    class Traits {
    public:
        /**
         * @brief Construct an element at given location
         * @tparam Args Argument types
         * @param location Where to construct the element
         * @param args Arguments to forward to constructor
         * @return Pointer to constructed element
         */
        template <typename... Args>
        static pointer ConstructAt(pointer location, Args&&... args) {
            new (location) _Ty(std::forward<Args>(args)...);
            return location;
        }

        /**
         * @brief Construct a range of elements
         * @param _First Source begin
         * @param _Dest Destination begin
         * @param count Number of elements to construct
         * @return Pointer to end of constructed range
         */
        static pointer ConstructRange(pointer _First, pointer _Dest, size_type count) {
            assert(_First != _Dest);
            if constexpr (std::is_nothrow_move_constructible_v<_Ty> || !std::is_copy_constructible_v<_Ty>) {
                std::uninitialized_move_n(_First, count, _Dest);
            } else {
                std::uninitialized_copy_n(_First, count, _Dest);
            }
            return _Dest + count;
        }

        /**
         * @brief Destroy elements in range
         * @param begin Start of range
         * @param count Number of elements to destroy
         */
        static void Destroy(pointer begin, size_type count = 1) noexcept { std::destroy_n(begin, count); }
    };

private:
    /**
     * @brief Allocate raw memory
     * @param n Number of elements to allocate
     * @return Pointer to allocated memory
     */
    static pointer Allocate(size_type n) {
        return n != 0 ? static_cast<pointer>(operator new(n * sizeof(value_type))) : nullptr;
    }

    /**
     * @brief Deallocate memory
     * @param buf Pointer to memory to deallocate
     */
    static void Deallocate(pointer buf) noexcept { operator delete(buf); }

    pointer buffer_ = nullptr; ///< Pointer to allocated memory
    size_type capacity_ = 0;   ///< Capacity in elements
};

/**
 * @class Vector
 * @brief A dynamic array implementation similar to std::vector
 * @tparam _Ty Type of elements stored in the vector
 * @tparam _Alloc Allocator type (defaults to RawMemory<_Ty>)
 */
template <class _Ty, class _Alloc = RawMemory<_Ty>>
class Vector {
public:
    using value_type = _Ty;                        ///< Type of elements
    using pointer = _Ty*;                          ///< Pointer to element type
    using reference = _Ty&;                        ///< Reference to element type
    using const_pointer = const _Ty*;              ///< Const pointer to element type
    using const_reference = const _Ty&;            ///< Const reference to element type
    using iterator = _Ty*;                         ///< Iterator type
    using const_iterator = const _Ty*;             ///< Const iterator type
    using allocator_type = _Alloc;                 ///< Allocator type
    using _Alloc_traits = typename _Alloc::Traits; ///< Allocator traits
    using size_type = typename _Alloc::size_type;  ///< Size type

    static_assert(
        std::is_same_v<_Ty, typename _Alloc::value_type>,
        "Vector<T, Allocator> requires that Allocator's value_type match T");

    /**
     * @brief Default constructor
     */
    Vector() = default;

    /**
     * @brief Constructor with size
     * @param size Initial size of vector
     */
    explicit Vector(size_type size) : _data(size), _size(size) {
        std::uninitialized_value_construct_n(_data.GetAddress(), size);
    }

    /**
     * @brief Move constructor
     * @param other Vector to move from
     */
    Vector(Vector&& other) noexcept : _data(std::move(other._data)), _size(other._size) { other._size = 0; }

    /**
     * @brief Move assignment operator
     * @param other Vector to move from
     * @return Reference to this object
     */
    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            Swap(other);
        }
        return *this;
    }

    /**
     * @brief Copy constructor
     * @param other Vector to copy from
     */
    Vector(const Vector& other) : _data(other._size), _size(other._size) {
        std::uninitialized_copy_n(other._data.GetAddress(), other._size, _data.GetAddress());
    }

    /**
     * @brief Copy assignment operator
     * @param rhs Vector to copy from
     * @return Reference to this object
     */
    Vector& operator=(const Vector& rhs) {
        if (this != &rhs) {
            if (rhs._size > _data.Capacity()) {
                Vector tmp(rhs);
                Swap(tmp);
            } else {
                auto rhs_begin = rhs._data.GetAddress();
                auto rhs_end = rhs._data.GetAddress() + rhs._size;
                auto rhs_copy_end = rhs._data.GetAddress() + std::min(_size, rhs._size);
                auto new_end = std::copy(rhs_begin, rhs_copy_end, begin());

                if (rhs._size > _size) {
                    std::uninitialized_copy(rhs_copy_end, rhs_end, new_end);
                }

                if (rhs._size < _size) {
                    _Alloc_traits::Destroy(new_end, (_size - rhs._size));
                }

                _size = rhs._size;
            }
        }
        return *this;
    }

    /**
     * @brief Destructor
     */
    ~Vector() {
        if (_size > 0) {
            _Alloc_traits::Destroy(_data.GetAddress(), _size);
        }
    }

    /**
     * @brief Change the size of the vector
     * @param new_size New size of the vector
     */
    void Resize(size_t new_size) {
        if (new_size < _size) {
            _Alloc_traits::Destroy(_data.GetAddress() + new_size, _size - new_size);
        } else if (new_size > _size) {
            if (new_size > _data.Capacity()) {
                Reserve(new_size);
            }
            std::uninitialized_value_construct_n(_data.GetAddress() + _size, new_size - _size);
        }
        _size = new_size;
    }

    /**
     * @brief Add element to the end (copy version)
     * @param value Value to add
     */
    void PushBack(const _Ty& value) { EmplaceOneAtBack(value); }

    /**
     * @brief Add element to the end (move version)
     * @param value Value to add
     */
    void PushBack(_Ty&& value) { EmplaceOneAtBack(std::move(value)); }

    /**
     * @brief Remove last element
     */
    void PopBack() noexcept {
        assert(_size > 0);
        _Alloc_traits::Destroy(_data.GetAddress() + (_size - 1));
        --_size;
    }

    /**
     * @brief Reserve capacity for elements
     * @param new_capacity New capacity
     */
    void Reserve(size_type new_capacity) {
        if (new_capacity <= _data.Capacity())
            return;

        _Alloc new_data(new_capacity);
        if constexpr (std::is_nothrow_move_constructible_v<_Ty> || !std::is_copy_constructible_v<_Ty>) {
            std::uninitialized_move_n(_data.GetAddress(), _size, new_data.GetAddress());
        } else {
            try {
                std::uninitialized_copy_n(_data.GetAddress(), _size, new_data.GetAddress());
            } catch (...) {
                throw;
            }
        }
        _Alloc_traits::Destroy(_data.GetAddress(), _size);
        _data = std::move(new_data);
    }

    /**
     * @brief Construct element in-place at the end
     * @tparam Args Argument types
     * @param args Arguments to forward to constructor
     * @return Reference to constructed element
     */
    template <typename... Args>
    reference EmplaceBack(Args&&... args) {
        return EmplaceOneAtBack(std::forward<Args>(args)...);
    }

    /**
     * @brief Insert element at position (copy version)
     * @param pos Position to insert at
     * @param value Value to insert
     * @return Iterator to inserted element
     */
    iterator Insert(const_iterator pos, const _Ty& value) { return InsertOneOfThemByIndex(pos, value); }

    /**
     * @brief Insert element at position (move version)
     * @param pos Position to insert at
     * @param value Value to insert
     * @return Iterator to inserted element
     */
    iterator Insert(const_iterator pos, _Ty&& value) { return InsertOneOfThemByIndex(pos, std::move(value)); }

    /**
     * @brief Construct element in-place at position
     * @tparam Args Argument types
     * @param pos Position to insert at
     * @param args Arguments to forward to constructor
     * @return Iterator to inserted element
     */
    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args) {
        return InsertOneOfThemByIndex(pos, std::forward<Args>(args)...);
    }

    /**
     * @brief Erase element at position
     * @param pos Position to erase
     * @return Iterator following last removed element
     */
    iterator Erase(const_iterator pos) {
        assert(pos >= begin() && pos < end());
        const size_type index = pos - begin();

        if (index == _size - 1) {
            _Alloc_traits::Destroy(_data.GetAddress() + index);
        } else {
            std::move(begin() + index + 1, end(), begin() + index);
            _Alloc_traits::Destroy(_data.GetAddress() + (_size - 1));
        }

        --_size;
        return begin() + index;
    }

    /**
     * @brief Get current size
     * @return Number of elements in vector
     */
    size_type Size() const noexcept { return _size; }

    /**
     * @brief Get current capacity
     * @return Capacity of vector
     */
    size_type Capacity() const noexcept { return _data.Capacity(); }

    /**
     * @brief Subscript operator
     * @param index Position to access
     * @return Reference to element at index
     */
    reference operator[](size_t index) noexcept {
        assert(index < _size);
        return _data.GetAddress()[index];
    }

    /**
     * @brief Const subscript operator
     * @param index Position to access
     * @return Const reference to element at index
     */
    const reference operator[](size_t index) const noexcept { return const_cast<Vector&>(*this)[index]; }

    /**
     * @brief Swap contents with another vector
     * @param other Vector to swap with
     */
    void Swap(Vector& other) noexcept {
        std::swap(_data, other._data);
        std::swap(_size, other._size);
    }

    /**
     * @brief Access last element
     * @return Reference to last element
     */
    reference back() noexcept { return _data[_size - 1]; }

    /**
     * @brief Access last element (const)
     * @return Const reference to last element
     */
    const_reference back() const noexcept { return _data[_size - 1]; }

    /**
     * @brief Get iterator to beginning
     * @return Iterator to first element
     */
    iterator begin() noexcept { return _data.GetAddress(); }

    /**
     * @brief Get iterator to end
     * @return Iterator to one past last element
     */
    iterator end() noexcept { return _data.GetAddress() + _size; }

    /**
     * @brief Get const iterator to beginning
     * @return Const iterator to first element
     */
    const_iterator begin() const noexcept { return _data.GetAddress(); }

    /**
     * @brief Get const iterator to end
     * @return Const iterator to one past last element
     */
    const_iterator end() const noexcept { return _data.GetAddress() + _size; }

    /**
     * @brief Get const iterator to beginning
     * @return Const iterator to first element
     */
    const_iterator cbegin() const noexcept { return _data.GetAddress(); }

    /**
     * @brief Get const iterator to end
     * @return Const iterator to one past last element
     */
    const_iterator cend() const noexcept { return _data.GetAddress() + _size; }

private:
    /**
     * @brief Internal helper to emplace element at back
     * @tparam Args Argument types
     * @param args Arguments to forward to constructor
     * @return Reference to constructed element
     */
    template <typename... Args>
    reference EmplaceOneAtBack(Args&&... args) {
        if (_size < _data.Capacity()) {
            return EmplaceBackWithUnusedCapacity(std::forward<Args>(args)...);
        }
        return EmplaceReallocate(std::forward<Args>(args)...);
    }

    /**
     * @brief Emplace when capacity is available
     * @tparam Args Argument types
     * @param args Arguments to forward to constructor
     * @return Reference to constructed element
     */
    template <typename... Args>
    reference EmplaceBackWithUnusedCapacity(Args&&... args) {
        assert(_size < _data.Capacity());
        _Alloc_traits::ConstructAt(_data + _size, std::forward<Args>(args)...);
        ++_size;
        return back();
    }

    /**
     * @brief Emplace when reallocation is needed
     * @tparam Args Argument types
     * @param args Arguments to forward to constructor
     * @return Reference to constructed element
     */
    template <typename... Args>
    reference EmplaceReallocate(Args&&... args) {
        const size_type new_capacity = _size == 0 ? 1 : _size * 2;
        _Alloc new_data(new_capacity);

        _Alloc_traits::ConstructAt(new_data + _size, std::forward<Args>(args)...);

        try {
            _Alloc_traits::ConstructRange(_data.GetAddress(), new_data.GetAddress(), _size);
        } catch (...) {
            _Alloc_traits::Destroy(new_data + _size, 1);
            throw;
        }

        _data.Swap(new_data);
        _Alloc_traits::Destroy(new_data.GetAddress(), _size);
        ++_size;

        return back();
    }

    /**
     * @brief Internal helper to insert element at position
     * @tparam Args Argument types
     * @param pos Position to insert at
     * @param args Arguments to forward to constructor
     * @return Iterator to inserted element
     */
    template <typename... Args>
    iterator InsertOneOfThemByIndex(const_iterator pos, Args&&... args) {
        assert(pos >= begin() && pos <= end());

        if (_size < _data.Capacity()) {
            return InsertUsingAvailableCapacity(pos, std::forward<Args>(args)...);
        }
        return InsertWithoutReallocation(pos, std::forward<Args>(args)...);
    }

    /**
     * @brief Insert when capacity is available
     * @tparam Args Argument types
     * @param pos Position to insert at
     * @param args Arguments to forward to constructor
     * @return Iterator to inserted element
     */
    template <typename... Args>
    iterator InsertUsingAvailableCapacity(const_iterator pos, Args&&... args) {
        assert(_size < _data.Capacity());
        const size_type index = pos - begin();
        const pointer backup_end = end();

        if (pos != backup_end) {
            _Ty tmp_copy(std::forward<Args>(args)...);

            try {
                _Alloc_traits::ConstructAt(backup_end, std::move(*(backup_end - 1)));
                std::move_backward(begin() + index, backup_end - 1, backup_end);
                _data[index] = std::move(tmp_copy);
            } catch (...) {
                std::move(backup_end, end(), begin() + index);
                throw;
            }
        } else {
            _Alloc_traits::ConstructAt(backup_end, std::forward<Args>(args)...);
        }

        ++_size;
        return begin() + index;
    }

    /**
     * @brief Insert when reallocation is needed
     * @tparam Args Argument types
     * @param pos Position to insert at
     * @param args Arguments to forward to constructor
     * @return Iterator to inserted element
     */
    template <typename... Args>
    iterator InsertWithoutReallocation(const_iterator pos, Args&&... args) {
        const size_type new_capacity = _size == 0 ? 1 : _size * 2;
        const size_type index = pos - begin();
        _Alloc new_data(new_capacity);

        try {
            if (index > 0) {
                _Alloc_traits::ConstructRange(_data.GetAddress(), new_data.GetAddress(), index);
            }

            _Alloc_traits::ConstructAt(new_data + index, std::forward<Args>(args)...);

            if (index < _size) {
                _Alloc_traits::ConstructRange(
                    _data.GetAddress() + index, new_data.GetAddress() + index + 1, _size - index);
            }
        } catch (...) {
            _Alloc_traits::Destroy(new_data + index, 1);
            throw;
        }

        _data.Swap(new_data);
        _Alloc_traits::Destroy(new_data.GetAddress(), _size);

        ++_size;
        return begin() + index;
    }

    _Alloc _data;        ///< Memory storage
    size_type _size = 0; ///< Number of elements
};
