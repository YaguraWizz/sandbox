#include <gtest/gtest.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <string>

#include "combinatorics/UnsignedInteger.h" 

using namespace std;
using namespace std::chrono;

struct TestResult {
    string testName;
    string operationName;
    long long averageTime;
};

// Функция для измерения времени сложения
static TestResult testAddition(const UnsignedInteger& a, const UnsignedInteger& b, int numTests, const string& testName, const string& operationName) {
    auto start = high_resolution_clock::now();
    UnsignedInteger result = 0;
    for (int i = 0; i < numTests; ++i) {
        result = a + b;
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start).count() / numTests;
    return { testName, operationName, duration };
}

// Функция для измерения времени вычитания
static TestResult testSubtraction(const UnsignedInteger& a, const UnsignedInteger& b, int numTests, const string& testName, const string& operationName) {
    auto start = high_resolution_clock::now();
    UnsignedInteger result = 0;
    for (int i = 0; i < numTests; ++i) {
        result = a - b;
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start).count() / numTests;
    return { testName, operationName, duration };
}

// Функция для измерения времени умножения
static TestResult testMultiplication(const UnsignedInteger& a, const UnsignedInteger& b, int numTests, const string& testName, const string& operationName) {
    auto start = high_resolution_clock::now();
    UnsignedInteger result = 0;
    for (int i = 0; i < numTests; ++i) {
        result = a * b;
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start).count() / numTests;
    return { testName, operationName, duration };
}

// Функция для измерения времени деления
static TestResult testDivision(const UnsignedInteger& a, const UnsignedInteger& b, int numTests, const string& testName, const string& operationName) {
    auto start = high_resolution_clock::now();
    UnsignedInteger result = 0;
    for (int i = 0; i < numTests; ++i) {
        result = a / b;
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start).count() / numTests;
    return { testName, operationName, duration };
}


TEST(BenchmarkTest, UnsignedIntegerPerformance) {
    const int numTests = 1;  // Number of repetitions for averaging

    cout << "\n### Performance Report for UnsignedInteger Operations ###\n" << endl;

    vector<TestResult> results;
    vector<string> msg;
    
    try {
        for (int cycle = 1; cycle <= 4; ++cycle) {
            string num1_str(pow(10, cycle), '9');
            string num2_str(pow(10, cycle), '8');

            UnsignedInteger a(num1_str);
            UnsignedInteger b(num2_str);

            msg.push_back("Cycle " + to_string(cycle) + ": Testing with numbers of size " + to_string(num1_str.length()) + " digits");

            results.push_back(testAddition(a, b, numTests, "Test " + to_string(cycle), "Addition"));
            results.push_back(testSubtraction(a, b, numTests, "Test " + to_string(cycle), "Subtraction"));
            results.push_back(testMultiplication(a, b, numTests, "Test " + to_string(cycle), "Multiplication"));
            results.push_back(testDivision(a, b, numTests, "Test " + to_string(cycle), "Division"));
        }
    }
    catch (const std::exception& e) {
        FAIL() << "Exception caught during benchmark: " << e.what();
    }
    catch (...) {
        FAIL() << "Unknown exception caught during benchmark.";
    }

    for (size_t cycle = 0; cycle < msg.size(); ++cycle) {
        cout << msg[cycle] << endl;
        for (int j = 0; j < 4; ++j) {
            int index = cycle * 4 + j;
            cout << "- " << results[index].testName << ": " << results[index].operationName << endl;
            cout << "  - Average time: " << results[index].averageTime << " ms" << endl;
        }
    }
    
    cout << "#########################################################\n" << endl;
}