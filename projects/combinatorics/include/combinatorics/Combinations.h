#pragma once
  #include "Index.h"

 template<typename T>
 class Combinations {
 private:
     using ulong = unsigned long long;
     std::vector<T> data;
     std::vector<bool> chosen;
     ulong r; // size of each combination

     // Stores the generated combinations
     std::vector<std::vector<T>> combinations;
 public:
     Combinations(const std::vector<T>& elements, ulong r)
         : data(elements), chosen(elements.size(), false), r(r) {}

     // Generates combinations and returns a pointer to the internal vector
     const std::vector<std::vector<T>> generateCombinations() {
         combinations.clear(); // Clear previous combinations
         generateHelper(0, 0, std::vector<T>());
         return combinations;
     }
 private:
     void generateHelper(size_t index, size_t selected, std::vector<T> currentCombination) {
         if (r == selected) {
             combinations.push_back(currentCombination); // Add combination to results
             return;
         }
         if (index == data.size()) {
             return;
         }

         // Include element at index in the combination
         currentCombination.push_back(data[index]);
         chosen[index] = true;
         generateHelper(index + 1, selected + 1, currentCombination);

         // Exclude element at index from the combination
         currentCombination.pop_back();
         chosen[index] = false;
         generateHelper(index + 1, selected, currentCombination);
     }
 };