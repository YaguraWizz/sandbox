 #pragma once
 #include "Index.h"


 template<typename T>
 class Permutations {
 private:
     std::vector<T> elements_;
 public:
     Permutations(const std::vector<T>& elements) : elements_(elements) {
         if (elements_.empty()) {
             throw std::invalid_argument("Elements is empty");
         }
     }

     std::vector<T> FindPermutations(Index index) const {
         if (factorial(elements_.size()) < index.get_index()) {
             throw std::invalid_argument("Index out of range");
         }

         std::vector<unsigned int> vec_index = index.get_fs_index();
         index.adjustVectorSize(vec_index, elements_.size());

         std::vector<T> result;
         std::vector<bool> used(elements_.size(), false);

         for (const auto& idx : vec_index) {
             size_t actual_index = 0;
             for (size_t j = 0; j < elements_.size(); ++j) {
                 if (!used[j]) {
                     if (idx == actual_index) {
                         result.push_back(elements_[j]);
                         used[j] = true;
                         break;
                     }
                     ++actual_index;
                 }
             }
         }
         return result;
     }

     Index FindNumberPermutations(const std::vector<T>& permutation) const {
         std::vector<bool> used(elements_.size(), false);
         std::vector<unsigned int> factorial_indices;

         for (size_t i = 0; i < permutation.size(); ++i) {
             T current_element = permutation[i];
             size_t current_index = 0;

             // Find the index of current_element in elements_
             for (size_t j = 0; j < elements_.size(); ++j) {
                 if (!used[j] && elements_[j] == current_element) {
                     used[j] = true;
                     break;
                 }
                 if (!used[j]) {
                     ++current_index;
                 }
             }

             // Store current_index in factorial_indices
             factorial_indices.push_back(current_index);
         }

         // Return an instance of IndexFactorialSystem with factorial_indices
         return Index(factorial_indices);
     }
 private:
     
 };
