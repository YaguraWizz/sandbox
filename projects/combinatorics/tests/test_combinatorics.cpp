#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "combinatorics/UnsignedInteger.h"
#include "combinatorics/Index.h"
#include "combinatorics/Permutations.h"
#include "combinatorics/Combinations.h"
#include "combinatorics/Arrangements.h"

// --- Тесты для UnsignedInteger ---
TEST(UnsignedIntegerTest, ArithmeticOperations) {
    UnsignedInteger value1(std::string(6, '9')); // 999999
    UnsignedInteger value2(std::string(6, '7')); // 777777

    EXPECT_EQ((value1 + value2).toLong(), 1777776ULL);
    EXPECT_EQ((value1 - value2).toLong(), 222222ULL);
    EXPECT_EQ((value1 * value2).toLong(), 777776222223ULL);
    EXPECT_EQ((value1 / value2).toLong(), 1ULL);
    EXPECT_EQ((value1 % value2).toLong(), 222222ULL);
}

TEST(UnsignedIntegerTest, ComparisonOperators) {
    UnsignedInteger value1(std::string(6, '9'));
    UnsignedInteger value2(std::string(6, '7'));

    EXPECT_FALSE(value1 == value2);
    EXPECT_TRUE(value1 != value2);
    EXPECT_FALSE(value1 <= value2);
    EXPECT_TRUE(value1 >= value2);
    EXPECT_TRUE(value1 > value2);
    EXPECT_FALSE(value1 < value2);
}

TEST(UnsignedIntegerTest, Factorial) {
    UnsignedInteger value(std::string(1, '9')); // 9
    EXPECT_EQ(factorial(value).toLong(), 362880ULL);
}


// --- Тесты для Index ---
TEST(IndexTest, SmallIndexFactorialSystem) {
    unsigned long long index = 20;
    Index Idx1(index);
    std::vector<unsigned int> expected_fs = { 3, 1, 0, 0 };
    
    EXPECT_EQ(Idx1.get_fs_index(), expected_fs);
    EXPECT_EQ(Idx1.get_index().toLong(), index);
}

TEST(IndexTest, LargeIndexFactorialSystem) {
    UnsignedInteger index = 2020;
    Index Idx1(index);
    std::vector<unsigned int> expected_fs = { 2, 4, 4, 0, 2, 0, 0 };
    
    EXPECT_EQ(Idx1.get_fs_index(), expected_fs);
    EXPECT_EQ(Idx1.get_index(), index);
}


// --- Тесты для Permutations ---
TEST(PermutationsTest, IntegerPermutations) {
    std::vector<int> int_v = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    Index Idx(20);
    Permutations<int> permVec(int_v);
    
    std::vector<int> expected = { 1, 2, 3, 4, 5, 6, 10, 8, 7, 9 };
    EXPECT_EQ(permVec.FindPermutations(Idx), expected);
}

TEST(PermutationsTest, CharPermutations) {
    std::vector<char> char_v = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    Index Idx(20);
    Permutations<char> permVec(char_v);
    
    std::vector<char> expected = { 'a', 'b', 'c', 'd', 'e', 'f', 'j', 'h', 'g', 'i' };
    EXPECT_EQ(permVec.FindPermutations(Idx), expected);
}


// --- Тесты для Combinations ---
TEST(CombinationsTest, IntegerCombinations) {
    std::vector<int> int_v = { 1, 2, 3, 4 };
    unsigned long long r = 2;
    Combinations<int> comb(int_v, r);
    
    std::vector<std::vector<int>> expected = { 
        { 1, 2 }, { 1, 3 }, { 1, 4 }, { 2, 3 }, { 2, 4 }, { 3, 4 }
    };
    EXPECT_EQ(comb.generateCombinations(), expected);
}

TEST(CombinationsTest, CharCombinations) {
    std::vector<char> char_v = { 'a', 'b', 'c', 'd' };
    unsigned long long r = 2;
    Combinations<char> comb(char_v, r);
    
    std::vector<std::vector<char>> expected = {
        { 'a', 'b' }, { 'a', 'c' }, { 'a', 'd' },
        { 'b', 'c' }, { 'b', 'd' }, { 'c', 'd' }
    };
    EXPECT_EQ(comb.generateCombinations(), expected);
}


// --- Тесты для Arrangements ---
TEST(ArrangementsTest, IntegerArrangements) {
    std::vector<int> int_v = { 1, 2, 3 };
    unsigned long long r = 2;
    Arrangements<int> arr(int_v, r);
    
    std::vector<std::vector<int>> expected = {
        { 1, 2 }, { 1, 3 }, { 2, 1 }, 
        { 2, 3 }, { 3, 1 }, { 3, 2 }
    };
    EXPECT_EQ(arr.generateArrangements(), expected);
}

TEST(ArrangementsTest, CharArrangements) {
    std::vector<char> char_v = { 'a', 'b', 'c' };
    unsigned long long r = 2;
    Arrangements<char> arr(char_v, r);
    
    std::vector<std::vector<char>> expected = {
        { 'a', 'b' }, { 'a', 'c' }, { 'b', 'a' },
        { 'b', 'c' }, { 'c', 'a' }, { 'c', 'b' }
    };
    EXPECT_EQ(arr.generateArrangements(), expected);
}