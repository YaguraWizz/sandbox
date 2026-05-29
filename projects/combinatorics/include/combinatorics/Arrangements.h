#pragma once

  #include "Index.h"

 template<typename T>
 class Arrangements {
 private:
     using ulong = unsigned long long;
     std::vector<T> data;
     std::vector<bool> chosen;
     ulong r; // size of each arrangements
     std::vector<std::vector<T>> allArrangements; // to store all generated arrangements

 public:
     Arrangements(const std::vector<T>& elements, ulong r)
         : data(elements), chosen(elements.size(), false), r(r) {
         if (r > elements.size() || r < 0) {
             throw std::invalid_argument("Invalid input: r must be between 1 and the size of elements.");
         }
     }
     std::vector<std::vector<T>> generateArrangements() {
         std::vector<T> currentArrangements;
         generateHelper(0, currentArrangements);
         return allArrangements;
     }
 private:
     void generateHelper(size_t index, std::vector<T>& currentArrangements) {
         if (r == index) {
             allArrangements.push_back(currentArrangements);
             return;
         }

         for (size_t i = 0; i < data.size(); ++i) {
             if (!chosen[i]) {
                 currentArrangements.push_back(data[i]);
                 chosen[i] = true;
                 generateHelper(index + 1, currentArrangements);
                 chosen[i] = false;
                 currentArrangements.pop_back();
             }
         }
     }
 };
