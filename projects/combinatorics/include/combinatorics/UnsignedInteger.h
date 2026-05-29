#pragma once
#include <string>
#include <stdexcept>
#include <algorithm>
#include <vector>

class UnsignedInteger{
public:
    // Constructor that takes a positive integer as a string
    UnsignedInteger(const std::string& value) {
        if (!IsPositiveInteger(value)) {
            throw std::invalid_argument("Value must be a positive integer");
        }
        value_ = value;
    }

    // New constructor that takes an integer literal
    UnsignedInteger(unsigned long long value) {
        if (value < 0) {
            throw std::invalid_argument("Value must be a non-negative integer");
        }
        value_ = std::to_string(value);
    }

    // Function to get the value as a string (unchanged)
    std::string getValueAsString() const {
        return value_;
    }

    static int compare(const UnsignedInteger& a, const UnsignedInteger& b) {
        const std::string& str1 = a.value_;
        const std::string& str2 = b.value_;

        // Удаление ведущих нулей для корректного сравнения
        size_t i = str1.find_first_not_of('0');
        size_t j = str2.find_first_not_of('0');

        std::string num1 = (i == std::string::npos) ? "0" : str1.substr(i);
        std::string num2 = (j == std::string::npos) ? "0" : str2.substr(j);

        // Сравнение по длине
        if (num1.length() != num2.length()) {
            return num1.length() < num2.length() ? -1 : 1;
        }

        // Посимвольное сравнение
        for (size_t k = 0; k < num1.length(); ++k) {
            if (num1[k] != num2[k]) {
                return num1[k] < num2[k] ? -1 : 1;
            }
        }

        return 0; // строки равны
    }

    // Comparison operators
    bool operator<(const UnsignedInteger& other) const {
        return compare(*this, other) < 0;
    }

    bool operator>(const UnsignedInteger& other) const {
        return compare(*this, other) > 0;
    }

    bool operator==(const UnsignedInteger& other) const {
        return compare(*this, other) == 0;
    }

    bool operator<=(const UnsignedInteger& other) const {
        return compare(*this, other) <= 0;
    }

    bool operator>=(const UnsignedInteger& other) const {
        return compare(*this, other) >= 0;
    }

    bool operator!=(const UnsignedInteger& other) const {
        return compare(*this, other) != 0;
    }


    // Arithmetic operators
    UnsignedInteger operator+(const UnsignedInteger& other) const {
        return UnsignedInteger(Add(value_, other.value_));
    }
    UnsignedInteger operator-(const UnsignedInteger& other) const {
        return UnsignedInteger(Subtract(value_, other.value_));
    }
    UnsignedInteger operator*(const UnsignedInteger& other) const {
        return UnsignedInteger(Multiply(value_, other.value_));
    }
    UnsignedInteger operator/(const UnsignedInteger& other) const {
        return UnsignedInteger(Divide(value_, other.value_));
    }
    UnsignedInteger operator%(const UnsignedInteger& other) const {
        return (*this) - ((*this) / other) * other;
    }

    // Compound assignment operators
    UnsignedInteger& operator+=(const UnsignedInteger& other) {
        value_ = Add(value_, other.value_);
        return *this;
    }
    UnsignedInteger& operator-=(const UnsignedInteger& other) {
        value_ = Subtract(value_, other.value_);
        return *this;
    }
    UnsignedInteger& operator*=(const UnsignedInteger& other) {
        value_ = Multiply(value_, other.value_);
        return *this;
    }
    UnsignedInteger& operator/=(const UnsignedInteger& other) {
        value_ = Divide(value_, other.value_);
        return *this;
    }

    // Prefix and postfix increment/decrement operators
    UnsignedInteger operator++() {
        value_ = Add(value_, "1");
        return *this;
    }
    UnsignedInteger operator--() {
        value_ = Subtract(value_, "1");
        return *this;
    }
    UnsignedInteger operator++(int) {
        UnsignedInteger temp = *this;
        *this = operator++();
        return temp;
    }
    UnsignedInteger operator--(int) {
        UnsignedInteger temp = *this;
        *this = operator--();
        return temp;
    }

    friend std::ostream& operator<<(std::ostream& oss, const UnsignedInteger& integer) {
        oss << integer.getValueAsString();
        return oss;
    }


    // Method to convert to unsigned long long
    unsigned long long toLong() const {
        try {
            return std::stoull(value_);
        }
        catch (const std::out_of_range& e) {
            throw std::out_of_range("Value is out of range for an unsigned long long" + std::string(e.what()));
        }
        catch (const std::invalid_argument& e) {
            throw std::invalid_argument("Invalid argument for conversion to unsigned long long" + std::string(e.what()));
        }
    }


private:
    std::string value_;

    bool IsPositiveInteger(const std::string& str) const {
        if (str.empty() || str[0] != '+' && !std::isdigit(str[0])) {
            return false;
        }
        for (char c : str) {
            if (!std::isdigit(c)) {
                return false;
            }
        }
        return true;
    }
    
    std::string Add(const std::string& lhs, const std::string& rhs) const {
        using ulong = unsigned long long;
        std::string result;

        int carry = 0;
        ulong lhs_length = lhs.length();
        ulong rhs_length = rhs.length();
        ulong max_length = std::max(lhs_length, rhs_length);

        for (ulong i = 0; i < max_length || carry; ++i) {
            int lhs_digit = (i < lhs_length) ? lhs[lhs_length - 1 - i] - '0' : 0;
            int rhs_digit = (i < rhs_length) ? rhs[rhs_length - 1 - i] - '0' : 0;
            int sum = lhs_digit + rhs_digit + carry;
            carry = sum / 10;
            result.push_back(sum % 10 + '0');
        }

        std::reverse(result.begin(), result.end());
        return result;
    }

    std::string Subtract(const std::string& lhs, const std::string& rhs) const {
        using ulong = unsigned long long;
        std::string result;
        int borrow = 0;
        ulong lhs_length = lhs.length();
        ulong rhs_length = rhs.length();

        for (ulong i = 0; i < lhs_length; ++i) {
            int lhs_digit = lhs[lhs_length - 1 - i] - '0';
            int rhs_digit = (i < rhs_length) ? rhs[rhs_length - 1 - i] - '0' : 0;
            int diff = lhs_digit - rhs_digit - borrow;
            if (diff < 0) {
                diff += 10;
                borrow = 1;
            }
            else {
                borrow = 0;
            }
            result.push_back(diff + '0');
        }

        while (result.length() > 1 && result.back() == '0') {
            result.pop_back();
        }

        std::reverse(result.begin(), result.end());
        return result;
    }

    std::string Multiply(const std::string& lhs, const std::string& rhs) const {
        size_t lhs_length = lhs.length();
        size_t rhs_length = rhs.length();
        std::vector<int> result(lhs_length + rhs_length, 0);

        // Умножение цифр с учетом переносов
        for (size_t i = lhs_length; i > 0; --i) {
            for (size_t j = rhs_length; j > 0; --j) {
                int product = (lhs[i - 1] - '0') * (rhs[j - 1] - '0');
                int sum = product + result[i + j - 1];
                result[i + j - 1] = sum % 10;  // текущая цифра
                result[i + j - 2] += sum / 10; // перенос разряда
            }
        }

        // Преобразование в строку
        std::string result_str;
        for (int digit : result) {
            result_str.push_back(digit + '0');
        }

        // Удаление ведущих нулей
        while (result_str.length() > 1 && result_str[0] == '0') {
            result_str.erase(result_str.begin());
        }

        return result_str;
    }
   
    std::string Divide(const std::string& lhs, const std::string& rhs) const {
        std::string quotient = "0";
        std::string dividend = lhs;  // Temporary variable for remaining value

        if (rhs == "0") {
            throw std::runtime_error("Division by zero");
        }

        auto Compare = [](const std::string& lhs, const std::string& rhs) -> std::ptrdiff_t {
            if (lhs.length() != rhs.length()) {
                return lhs.length() - rhs.length();  // Compare lengths directly
            }

            for (size_t i = 0; i < lhs.length(); ++i) {
                if (lhs[i] != rhs[i]) {
                    return static_cast<ptrdiff_t>(lhs[i] - rhs[i]);  // Compare characters if lengths are equal
                }
            }

            return 0;  // Equal strings
            };

        while (Compare(dividend, rhs) >= 0) {  // Now correctly compares based on Compare function result
            dividend = Subtract(dividend, rhs);
            quotient = Add(quotient, "1");
        }

        return quotient;
    }
};

 static UnsignedInteger factorial(UnsignedInteger value) {
     if (value == 0 || value == 1) {
         return 1;
     }

     UnsignedInteger result = 1;
     for (UnsignedInteger i = 2; i <= value; ++i) {
         result *= i;
     }
     return result;
 }

 static UnsignedInteger factorialNumber(UnsignedInteger value) {
     if (value <= 1) {
         return value;
     }
     UnsignedInteger current_value = value;
     UnsignedInteger result = 1;

     while (current_value % result == 0) {
         current_value = current_value / result;
         ++result;
     }
     return --result;
 }
