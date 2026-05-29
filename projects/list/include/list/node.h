#pragma once


    template <typename Type>
    struct Node {
        Node() = default;
        Node(const Type& val, Node* next) : value(val), next_node(next) {}
        Type value{};
        Node* next_node = nullptr;
    };
