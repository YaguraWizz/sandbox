#pragma once
#include <cassert>
#include <cstddef>
#include <iterator>
#include <string>
#include <utility>
#include <initializer_list>
#include <algorithm>

#include "list/node.h"



template <typename Type>
class SingleLinkedList {
public:
    SingleLinkedList() = default; 

    SingleLinkedList(std::initializer_list<Type> values) {
        AppendFromRange(values.begin(), values.end());
    }

    SingleLinkedList(const SingleLinkedList& other) {
        AppendFromRange(other.begin(), other.end());
    }

    template <typename Iterator>
    SingleLinkedList(Iterator begin, Iterator end) {
        AppendFromRange(begin, end);
    }

    ~SingleLinkedList() noexcept {
        Clear();
    }

    SingleLinkedList& operator=(const SingleLinkedList& rhs) {
        if (this != &rhs) {
            SingleLinkedList tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    void PushFront(const Type& value) {
        head_.next_node = new Node<Type>(value, head_.next_node);
        ++size_;
    }

    void Clear() noexcept {
        while (head_.next_node != nullptr) {
            Node<Type>* next_node = head_.next_node->next_node;
            delete head_.next_node;
            head_.next_node = next_node;
        }
        size_ = 0u;
    }

    [[nodiscard]] size_t GetSize() const noexcept {
        return size_;
    }

    [[nodiscard]] bool IsEmpty() const noexcept {
        return size_ == 0u;
    }

    template <typename ValueType>
    class BasicIterator {
        friend class SingleLinkedList;

        explicit BasicIterator(Node<Type>* node) : node_(node) {}

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Type;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        BasicIterator() = default;

        BasicIterator(const BasicIterator<Type>& other) noexcept : node_(other.node_) {}

        BasicIterator& operator=(const BasicIterator& rhs) = default;

        [[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
            return node_ != rhs.node_;
        }

        [[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept {
            return node_ != rhs.node_;
        }

        BasicIterator& operator++() noexcept {
            assert(node_);
            node_ = node_->next_node;
            return *this;
        }

        BasicIterator operator++(int) noexcept {
            assert(node_);
            auto old_value(*this);
            ++(*this);
            return old_value;
        }

        [[nodiscard]] reference operator*() const noexcept {
            assert(node_);
            return node_->value;
        }

        [[nodiscard]] pointer operator->() const noexcept {
            assert(node_);
            return &(node_->value);
        }

    private:
        Node<Type>* node_ = nullptr;
    };

public:
    using value_type = Type;
    using reference = value_type&;
    using const_reference = const value_type&;

    using Iterator = BasicIterator<Type>;
    using ConstIterator = BasicIterator<const Type>;

    [[nodiscard]] Iterator begin() noexcept {
        return Iterator{ head_.next_node };
    }

    [[nodiscard]] Iterator end() noexcept {
        return Iterator();
    }

    [[nodiscard]] ConstIterator begin() const noexcept {
        return ConstIterator{ head_.next_node };
    }

    [[nodiscard]] ConstIterator end() const noexcept {
        return ConstIterator();
    }

    [[nodiscard]] ConstIterator cbegin() const noexcept {
        return ConstIterator{ head_.next_node };
    }

    [[nodiscard]] ConstIterator cend() const noexcept {
        return ConstIterator();
    }

    [[nodiscard]] Iterator before_begin() noexcept {
        return Iterator(&head_);
    }

    [[nodiscard]] ConstIterator cbefore_begin() const noexcept {
        return ConstIterator(const_cast<Node<Type>*>(&head_));
    }

    [[nodiscard]] ConstIterator before_begin() const noexcept {
        return Iterator(&head_);
    }

    Iterator InsertAfter(ConstIterator pos, const Type& value) {
        Node<Type>* pos_node = const_cast<Node<Type>*>(pos.node_);
        if (pos_node == nullptr) {
            PushFront(value);
            return begin();
        }
        else {
            Node<Type>* new_node = new Node<Type>(value, pos_node->next_node);
            pos_node->next_node = new_node;
            ++size_;
            return Iterator{ new_node };
        }
    }

    void PopFront() noexcept {
        if (head_.next_node != nullptr) {
            Node<Type>* to_delete = head_.next_node;
            head_.next_node = to_delete->next_node;
            delete to_delete;
            --size_;
        }
    }

    Iterator EraseAfter(ConstIterator pos) noexcept {
        Node<Type>* pos_node = const_cast<Node<Type>*>(pos.node_);
        if (pos_node == nullptr || pos_node->next_node == nullptr) {
            return end();
        }
        else {
            Node<Type>* to_delete = pos_node->next_node;
            pos_node->next_node = to_delete->next_node;
            delete to_delete;
            --size_;
            return Iterator{ pos_node->next_node };
        }
    }

    void swap(SingleLinkedList& other) noexcept {
        std::swap(head_.next_node, other.head_.next_node);
        std::swap(size_, other.size_);
    }

private:
    Node<Type> head_;
    size_t size_ = 0;

    template <typename Iterator>
    void AppendFromRange(Iterator begin, Iterator end) {
        Node<Type>* current = &head_;
        while (current->next_node != nullptr) {
            current = current->next_node;
        }

        while (begin != end) {
            current->next_node = new Node<Type>(*begin, nullptr);
            current = current->next_node;
            ++begin;
            ++size_;
        }
    }
};

template <typename Type>
void swap(SingleLinkedList<Type>& lhs, SingleLinkedList<Type>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename Type>
bool operator==(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs < rhs);
}
