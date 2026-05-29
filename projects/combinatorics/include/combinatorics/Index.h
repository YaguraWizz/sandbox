#pragma once
#include "UnsignedInteger.h"
#include <limits>

class Index {
public:
    using ulong = unsigned long long;
    using uint = unsigned int;

    // Число помещается в ulong
    Index(ulong index) {
        if (index > std::numeric_limits<ulong>::max()) {
            throw std::out_of_range("Index exceeds ulong limits");
        }
        cost_index_to_fs(std::to_string(index));
    }

    // Число превышает ulong
    Index(const UnsignedInteger& index) {
        cost_index_to_fs(index);
    }

    Index(const std::vector<uint>& fs_index) {
        cost_fs_to_index_string(fs_index);
    }

    std::vector<uint> get_fs_index() {
        return fs_index_;
    }

    UnsignedInteger get_index() {
        return cost_fs_to_index_string(fs_index_);
    }

    void adjustVectorSize(std::vector<uint>& vec, size_t desired_size) const {
        if (vec.size() != desired_size) {
            std::reverse(vec.begin(), vec.end());
            while (vec.size() != desired_size) {
                vec.push_back(0);
            }
            std::reverse(vec.begin(), vec.end());
        }
    }

private:
    std::vector<uint> fs_index_;

    // Конвертация числа в факториальный вид и запись в fs_index_
    void cost_index_to_fs(UnsignedInteger index_str) {
        std::vector<uint> temp_list;
        UnsignedInteger i = 1;
        temp_list.push_back(0);
        while (index_str != 0) {
            temp_list.push_back(static_cast<uint>((index_str % (i + 1)).toLong()));
            index_str /= (i + 1);
            ++i;
        }
        std::reverse(temp_list.begin(), temp_list.end());
        fs_index_ = temp_list;
    }

    // Конвертация факториального вида числа в индекс (string)
    UnsignedInteger cost_fs_to_index_string(const std::vector<unsigned int>& fs_index) const {
        UnsignedInteger number = 0;
        UnsignedInteger size_ = static_cast<unsigned long>(fs_index.size() - 1);

        for (const auto& value : fs_index) {
            number += factorial(size_) * value;
            --size_;
        }

        return number;
    }
};