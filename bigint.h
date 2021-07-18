/**
 * MIT License
 *
 * Copyright (c) 2021 Zap
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef BIGINT_H
#define BIGINT_H
#include <vector>
#include <cmath>
#include <limits>
#include <iostream>
#include <bitset>
#include <algorithm>
#include <cctype>

class BigInt {
private:
    using word_t = unsigned long long;
    static constexpr word_t full_chunk = std::numeric_limits<word_t>::max();
    static constexpr size_t bit_per_chunk = 8 * sizeof(word_t);
    std::vector<word_t> data;
    bool default_bit = 0;
    bool negative = false;
    
    // Remove leading empty block
    void trim() {
        while (data.back() == (full_chunk * default_bit)) data.pop_back();
        if (size() <= 0) {
            negative = false;
            data = {0};
        }
    }

    // The size of the bianry string representing the number
    size_t size() {
        if (default_bit == 1) return std::numeric_limits<size_t>::max();
        if (data.empty()) return 0;
        size_t result = (data.size() - 1) * bit_per_chunk;
        size_t tmp = data.back();
        while (tmp > 0) {
            tmp >>= 1;
            result++;
        }
        return result;
    }

public:

    // Construct from integer
    BigInt(long long x = 0) : data({(word_t)std::abs(x)}), default_bit(0), negative(x < 0) {};

    // Construct from string
    BigInt(std::string s) {
        data = {0};
        bool is_neg = false;
        if (s.front() == '-') {
            is_neg = true;
            s.erase(s.begin());
        }
        for (char c : s) {
            if (!std::isdigit(c)) throw std::invalid_argument("input string is not a number");
            operator*=(10);
            operator+=(c - '0');
        }
        negative = is_neg;
        trim();
    }

    // Copy-constructor
    BigInt(const BigInt &x) {
        default_bit = x.default_bit;
        negative = x.negative;
        data = x.data;
        trim();
    }

    // Destructor: compiler-generated

    // Convert to string with a base
    std::string to_string(size_t base = 10) const {
        BigInt temp = *this;
        if (base > 10) throw std::out_of_range("base must be less than 10");
        if (temp.size() <= 0) return "0";
        if (default_bit == 1) return "infinity";
        if (base == 2) {
            std::string result = "";
            for (word_t i : data) {
                result = std::bitset<bit_per_chunk>(i).to_string() + result;
            }
            result.erase(0, result.find_first_not_of('0'));
            if (negative) result = '-' + result;
            return result;
        }
        BigInt old = *this;
        std::string result = "";
        while (temp.size()) {
            result = std::to_string((temp % base).data[0]) + result;
            temp /= base;
        }
        if (negative) result = '-' + result;
        return result;
    }

    operator std::string() { return to_string(10); }

    friend std::ostream& operator<<(std::ostream &out, const BigInt &x){
	    out << x.to_string(10);
	    return out;
    }

    friend std::istream& operator>>(std::istream &in, BigInt &x){
	    std::string temp;
        in >> temp;
        x = BigInt(temp);
	    return in;
    }

    BigInt operator~() {
        BigInt old = *this;
        for (word_t &i : data) {
            i = ~i;
        }
        default_bit = !default_bit;
        BigInt temp = *this;
        *this = old;
        return temp;
    }

    BigInt operator&=(BigInt x) {
        if (data.size() < x.data.size()) data.insert(data.end(), x.data.size() - data.size(), full_chunk * default_bit);
        default_bit = default_bit && x.default_bit;
        for (size_t i = 0; i < data.size(); i++) {
            if (i >= x.data.size()) data[i] &= full_chunk * x.default_bit;
            else data[i] &= x.data[i];
        }
        trim();
        return *this;
    }

    BigInt operator|=(BigInt x) {
        if (data.size() < x.data.size()) data.insert(data.end(), x.data.size() - data.size(), full_chunk * default_bit);
        default_bit = default_bit || x.default_bit;
        for (size_t i = 0; i < data.size(); i++) {
            if (i >= x.data.size()) data[i] |= full_chunk * x.default_bit;
            else data[i] |= x.data[i];
        }
        trim();
        return *this;
    }

    BigInt operator^=(BigInt x) {
        if (data.size() < x.data.size()) data.insert(data.end(), x.data.size() - data.size(), full_chunk * default_bit);
        default_bit = default_bit != x.default_bit;
        for (size_t i = 0; i < data.size(); i++) {
            if (i >= x.data.size()) data[i] ^= full_chunk * x.default_bit;
            else data[i] ^= x.data[i];
        }
        trim();
        return *this;
    }

    BigInt operator<<=(size_t x) {
        size_t shift = x / bit_per_chunk;
        size_t offset = x % bit_per_chunk;
        data.insert(data.begin(), shift, 0);
        if (!offset) return *this;
        word_t carry = 0;
        for (word_t &i : data) {
            word_t new_carry = i >> (bit_per_chunk - offset);
            i <<= offset;
            i |= carry;
            carry = new_carry;
        }
        if (carry) data.push_back(carry | ((full_chunk * default_bit) << offset));
        trim();
        return *this;
    }

    BigInt operator>>=(size_t x) {
        size_t shift = x / bit_per_chunk;
        size_t offset = x % bit_per_chunk;
        data.erase(data.begin(), data.begin() + shift);
        for (size_t i = 0; i < data.size(); i++) {
            word_t carry = data[i] << (bit_per_chunk - offset);
            data[i] >>= offset;
            if (i) data[i - 1] |= carry;
        }
        data.back() |= ((full_chunk * default_bit) << (bit_per_chunk - offset));
        trim();
        return *this;
    }


    BigInt operator&(BigInt x) {
        BigInt old = *this;
        operator&=(x);
        BigInt temp = *this;
        *this = old;
        return temp;
    }

    BigInt operator|(BigInt x) {
        BigInt old = *this;
        operator|=(x);
        BigInt temp = *this;
        *this = old;
        return temp;
    }

    BigInt operator^(BigInt x) {
        BigInt old = *this;
        operator^=(x);
        BigInt temp = *this;
        *this = old;
        return temp;
    }

    BigInt operator<<(size_t x) {
        BigInt old = *this;
        operator<<=(x);
        BigInt temp = *this;
        *this = old;
        return temp;
    }

    BigInt operator>>(size_t x) {
        BigInt old = *this;
        operator>>=(x);
        BigInt temp = *this;
        *this = old;
        return temp;
    }


    bool operator<(BigInt x) {
        if (negative != x.negative) return negative;
        if (size() != x.size()) return (size() < x.size());
        return std::lexicographical_compare(data.rbegin(), data.rend(), x.data.rbegin(), x.data.rend(), std::less<word_t>());
    }

    bool operator>(BigInt x) {
        if (negative != x.negative) return x.negative;
        if (size() != x.size()) return (size() > x.size());
        return std::lexicographical_compare(data.rbegin(), data.rend(), x.data.rbegin(), x.data.rend(), std::greater<word_t>());
        return false;
    }

    bool operator==(BigInt x) {
        if (default_bit != x.default_bit || negative != x.negative || size() != x.size()) return false;
        for (size_t i = 0; i < data.size(); i++) if (data[i] != x.data[i]) return false;
        return true;
    }

    bool operator!=(BigInt x) { return !operator==(x); }

    bool operator>=(BigInt x) { return !operator<(x); }

    bool operator<=(BigInt x) { return !operator>(x); }


    BigInt operator+() {
        return *this;
    }

    BigInt operator-() {
        BigInt old = *this;
        negative = !negative;
        BigInt temp = *this;
        *this = old;
        return temp;
    }

    BigInt operator+=(BigInt x) {
        if (x.negative) return operator-=(-x); // x + -y = x - y
        if (negative) {
            *this = -(operator-() - x); // -x + y = -(x - y)
            return *this;
        }

        while (x.size()) {
            BigInt carry = operator&(x);
            operator^=(x);
            x = (carry << 1);
        }
        trim();
        return *this;
    }

    BigInt operator+(BigInt x) {
        BigInt old = *this;
        operator+=(x);
        BigInt temp = *this;
        *this = old;
        return temp;
    }

    BigInt operator-=(BigInt x) {
        if (x.negative) return operator+=(-x); // x - (-y) = x + y
        if (negative) {
            *this = -(operator-() + x); // -x -y = -(x + y)
            return *this;
        }

        if (operator<(x)) {
            *this = -(x - *this); // x - y = -(y - x)
            return *this;
        }

        while (x.size()) {
            BigInt borrow = operator~().operator&(x);
            operator^=(x);
            x = borrow << 1; 
        }
        trim();
        return *this;
    }

    BigInt operator-(BigInt x) {
        BigInt old = *this;
        operator-=(x);
        BigInt temp = *this;
        *this = old;
        return temp;
    }

    BigInt operator*=(BigInt x) { return operator=(operator*(x)); }

    // Russian peasant algorithm
    BigInt operator*(BigInt x) {
        BigInt old = *this;
        BigInt res;
        bool is_neg = (negative != x.negative);
        res.negative = false;
        x.negative = false;
        negative = false;
        while (x.size()) {
            if ((x & BigInt(1)).size()) res += *this;
            operator<<=(1);
            x >>= 1;
        }
        res.negative = is_neg;
        res.trim();
        *this = old;
        return res;
    }

    BigInt operator/=(BigInt x) {
        BigInt res(0), temp(1);
        res.negative = (negative != x.negative);
        negative = false;
        x.negative = false;
        while (operator>=(x)) {
            x <<= 1;
            temp <<= 1;
        }
        while (temp > BigInt(0)) {
            x >>= 1;
            temp >>= 1;
            if (operator>=(x)) {
                operator-=(x);
                res|= temp;
            }
        }
        res.trim();
        *this = res;
        return *this;
    }

    BigInt operator/(BigInt x) {
        BigInt old = *this;
        operator/=(x);
        BigInt temp = *this;
        *this = old;
        return temp;
    }

    BigInt operator%(BigInt x) { return *this - operator/(x) * x; }

    BigInt operator%=(BigInt x) { return operator=(operator%(x)); }

    BigInt operator++() { return operator+=(1); }

    BigInt operator++(int) { 
        BigInt temp = *this;
        operator+=(1);
        return temp;
    }

    BigInt operator--() { return operator-=(1); }

    BigInt operator--(int) { 
        BigInt temp = *this;
        operator-=(1);
        return temp;
    }
};

BigInt operator ""_N(const char* x, size_t size) {
    return BigInt(std::string(x));
}

#endif