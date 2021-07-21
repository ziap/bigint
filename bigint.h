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
#include <algorithm>
#include <bitset>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

class BigInt {
   private:
    using word_t = unsigned long long;
    static constexpr word_t full_chunk = std::numeric_limits<word_t>::max();
    static constexpr size_t bit_per_chunk = 8 * sizeof(word_t);

    std::vector<word_t> data;
    bool default_bit = 0;

    // Remove leading empty block
    void trim() {
        while (data.back() == (full_chunk * default_bit) && data.size() > 1) data.pop_back();
        if (data.size() <= 0) { data = {full_chunk * default_bit}; }
    }

    // Russian peasant multiplication algorithm (Working)
    static BigInt multiply(BigInt x, BigInt y) {
        bool is_neg = (x.default_bit != y.default_bit);
        if (x.default_bit) x = -x;
        if (y.default_bit) y = -y;

        BigInt res = 0;

        while (y > 0) {
            if ((y & 1) > 0) res += x;
            x <<= 1;
            y >>= 1;
        }

        if (is_neg) res = -res;
        return res;
    }

    // Karatsuba multiplication algorithm (Experimental)
    static BigInt karatsuba(BigInt x, BigInt y) {
        bool is_neg = (x.default_bit != y.default_bit);
        if (x.default_bit) x = -x;
        if (y.default_bit) y = -y;

        BigInt res = 0;

        size_t n = std::max(x.data.size(), y.data.size());
        if (n < 2) {
            __uint128_t mul = __uint128_t(x.data[0]) * __uint128_t(y.data[0]);
            BigInt result;
            result.data = {word_t(mul), word_t(mul >> 64)};
            result.trim();
            return result;
        }

        n = (n + 1) >> 1;

        BigInt a, b;
        if (x.data.size() > n) {
            a.data = std::vector<word_t>(x.data.begin(), x.data.begin() + n);
            b.data = std::vector<word_t>(x.data.begin() + n, x.data.end());
        } else
            a.data = x.data;

        BigInt c, d;
        if (y.data.size() > n) {
            c.data = std::vector<word_t>(y.data.begin(), y.data.begin() + n);
            d.data = std::vector<word_t>(y.data.begin() + n, y.data.end());
        } else
            c.data = y.data;

        BigInt ac = karatsuba(a, c);
        BigInt bd = karatsuba(b, d);
        BigInt abcd = karatsuba(a + b, c + d) - ac - bd;

        BigInt result = ac + (abcd << (n << 6)) + (bd << (n << 7));
        if (is_neg) result = -result;
        return result;
    }

    std::pair<BigInt, BigInt> divide(BigInt x) {
        BigInt old = *this;
        BigInt res(0), temp(1);
        bool is_neg = (default_bit != x.default_bit);
        if (default_bit) *this = operator-();
        if (x.default_bit) x = -x;
        while (operator>=(x)) {
            x <<= 1;
            temp <<= 1;
        }
        while (temp > BigInt(1)) {
            x >>= 1;
            temp >>= 1;
            if (operator>=(x)) {
                operator-=(x);
                res |= temp;
            }
        }
        BigInt curr = *this;
        if (is_neg) res = -res;
        if (old.default_bit) curr = -curr;
        *this = old;
        return {res, curr};
    }

    // The size of the bianry string representing the number
    size_t size() {
        if (default_bit == 1) return operator-().size();
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

    // 10^x = 5^x << x
    // 5^x is calculated using binary exponetiation
    static BigInt ten_exp(size_t x) {
        size_t shift = x;
        BigInt five(5), res(1);
        while (x) {
            if (x & 1) res = res * five;
            five *= five;
            x >>= 1;
        }
        return (res << shift);
    }

    // Construct from integer
    BigInt(long long x = 0) : data({(word_t)std::abs(x)}), default_bit(0) {
        if (x < 0) *this = operator-();
    };

    // Construct from string
    BigInt(std::string s) {
        data = {0};
        bool is_neg = false;
        if (s.front() == '-') {
            is_neg = true;
            s.erase(s.begin());
        }
        if (s.length() <= 18) operator+=(std::stoull(s));
        else {
            size_t m = (s.length() + 1) >> 1;
            BigInt lo(s.substr(0, m));
            BigInt hi(s.substr(m));
            operator+=(lo);
            operator*=(ten_exp(s.length() - m));
            operator+=(hi);
        }
        if (is_neg) *this = operator-();
    }

    // Copy-constructor
    BigInt(const BigInt &x) {
        default_bit = x.default_bit;
        data = x.data;
        trim();
    }

    // Destructor: compiler-generated

    // Convert to binary
    std::string to_binary() const {
        bool is_neg = default_bit;
        BigInt temp = *this;
        if (is_neg) temp = -temp;
        std::string result = "";
        for (word_t i : temp.data) { result = std::bitset<bit_per_chunk>(i).to_string() + result; }
        result.erase(0, result.find_first_not_of('0'));
        if (is_neg) result = '-' + result;
        return result;
    }

    std::string to_string() {
        if (default_bit) return std::string("-") + operator-().to_string();
        if (data.size() == 1) return std::to_string(data[0]);
        size_t m = (size_t(double(size()) / log2(10) + 1) + 1) >> 1;
        std::pair<BigInt, BigInt> div_res = divide(ten_exp(m));
        std::string lo = div_res.second.to_string();
        return div_res.first.to_string() + std::string(m - lo.length(), '0') + lo;
    }

    friend std::ostream &operator<<(std::ostream &out, BigInt x) {
        out << x.to_string() << '\n';
        return out;
    }

    friend std::istream &operator>>(std::istream &in, BigInt &x) {
        std::string temp;
        in >> temp;
        x = BigInt(temp);
        return in;
    }

    BigInt operator~() {
        BigInt old = *this;
        for (word_t &i : data) { i = ~i; }
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
            else
                data[i] &= x.data[i];
        }
        trim();
        return *this;
    }

    BigInt operator|=(BigInt x) {
        if (data.size() < x.data.size()) data.insert(data.end(), x.data.size() - data.size(), full_chunk * default_bit);
        default_bit = default_bit || x.default_bit;
        for (size_t i = 0; i < data.size(); i++) {
            if (i >= x.data.size()) data[i] |= full_chunk * x.default_bit;
            else
                data[i] |= x.data[i];
        }
        trim();
        return *this;
    }

    BigInt operator^=(BigInt x) {
        if (data.size() < x.data.size()) data.insert(data.end(), x.data.size() - data.size(), full_chunk * default_bit);
        default_bit = default_bit != x.default_bit;
        for (size_t i = 0; i < data.size(); i++) {
            if (i >= x.data.size()) data[i] ^= full_chunk * x.default_bit;
            else
                data[i] ^= x.data[i];
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
        if (default_bit != x.default_bit) return default_bit;
        if (default_bit) return operator-().operator>(-x);
        if (data.size() != x.data.size()) return (data.size() < x.data.size());
        return std::lexicographical_compare(data.rbegin(), data.rend(), x.data.rbegin(), x.data.rend(), std::less<word_t>());
    }

    bool operator>(BigInt x) {
        if (default_bit != x.default_bit) return x.default_bit;
        if (default_bit) return operator-().operator<(-x);
        if (data.size() != x.data.size()) return (data.size() > x.data.size());
        return std::lexicographical_compare(data.rbegin(), data.rend(), x.data.rbegin(), x.data.rend(), std::greater<word_t>());
    }

    bool operator==(BigInt x) {
        if (default_bit != x.default_bit || default_bit != x.default_bit || size() != x.size()) return false;
        for (size_t i = 0; i < data.size(); i++)
            if (data[i] != x.data[i]) return false;
        return true;
    }

    bool operator!=(BigInt x) { return !operator==(x); }

    bool operator>=(BigInt x) { return !operator<(x); }

    bool operator<=(BigInt x) { return !operator>(x); }

    BigInt operator+() { return *this; }

    BigInt operator-() {
        if (operator==(0)) return *this;
        BigInt old = *this;
        *this = operator~();
        operator++();
        BigInt temp = *this;
        *this = old;
        return temp;
    }

    BigInt operator+=(BigInt x) {
        if (x.default_bit) return operator-=(-x);  // x + -y = x - y
        if (default_bit) {
            *this = -(operator-() - x);  // -x + y = -(x - y)
            return *this;
        }

        if (data.size() < x.data.size()) data.insert(data.end(), x.data.size() - data.size(), 0);
        bool carry = 0;
        for (size_t i = 0; i < data.size(); i++) {
            if (i >= x.data.size()) {
                if (carry) {
                    if (data[i] == full_chunk) data[i] = 0;
                    else {
                        data[i] += carry;
                        carry = 0;
                    }
                } else
                    break;
            } else {
                bool next_carry = 0;
                if (full_chunk - x.data[i] < data[i]) {
                    next_carry = 1;
                    data[i] -= (full_chunk - x.data[i] + 1);
                } else
                    data[i] += x.data[i];
                if (carry) {
                    if (data[i] == full_chunk) {
                        data[i] = 0;
                        next_carry = 1;
                    } else
                        data[i] += carry;
                }
                carry = next_carry;
            }
        }
        if (carry) data.push_back(1);
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
        if (x.default_bit) return operator+=(-x);  // x - (-y) = x + y
        if (default_bit) {
            *this = -(operator-() + x);  // -x -y = -(x + y)
            return *this;
        }

        if (operator<(x)) {
            *this = -(x - *this);  // x - y = -(y - x)
            return *this;
        }

        if (data.size() < x.data.size()) data.insert(data.end(), x.data.size() - data.size(), 0);
        bool carry = 0;
        for (size_t i = 0; i < data.size(); i++) {
            if (i >= x.data.size()) {
                if (carry) {
                    if (data[i] == 0) data[i] = full_chunk;
                    else {
                        data[i] -= carry;
                        carry = 0;
                    }
                } else
                    break;
            } else {
                bool next_carry = 0;
                if (data[i] < x.data[i]) {
                    next_carry = 1;
                    data[i] += (full_chunk - x.data[i] + 1);
                } else
                    data[i] -= x.data[i];
                if (carry) {
                    if (data[i] == 0) {
                        data[i] = full_chunk;
                        next_carry = 1;
                    } else
                        data[i] -= carry;
                }
                carry = next_carry;
            }
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

    BigInt operator*(BigInt x) { return multiply(*this, x); }

    BigInt operator/=(BigInt x) { return operator=(operator/(x)); }

    BigInt operator/(BigInt x) { return divide(x).first; }

    BigInt operator%(BigInt x) { return divide(x).second; }

    BigInt operator%=(BigInt x) { return operator=(operator%(x)); }

    BigInt operator++() {
        bool carry = 1;
        for (word_t &i : data) {
            if (i == full_chunk) i = 0;
            else {
                i++;
                carry = 0;
                break;
            }
        }
        if (carry) *this = 0;
        return *this;
    }

    BigInt operator++(int) {
        BigInt temp = *this;
        operator++();
        return temp;
    }

    BigInt operator--() {
        bool carry = 1;
        for (word_t &i : data) {
            if (i == 0) i = full_chunk;
            else {
                i--;
                carry = 0;
                break;
            }
        }
        if (carry) *this = -1;
        return *this;
    }

    BigInt operator--(int) {
        BigInt temp = *this;
        operator--();
        return temp;
    }
};

BigInt operator""_N(const char *x, size_t size) { return BigInt(std::string(x)); }

#endif