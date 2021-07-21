<h1 align="center">Big Integer</h1>

<h3 align="center">A space efficient, arbitrary-sized integer library for C++</h3>

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Unit test](https://github.com/ziap/bigint/actions/workflows/main.yml/badge.svg)](https://github.com/ziap/bigint/actions/workflows/main.yml)

## Features

 - No other dependencies apart from C++ STL.
 - Space efficient dynamic bitset based implementation.
 - Two's complement representation for negative values.
 - Provides both arithmetic and **binary** operations.
 - Binary conversion.

## Usage

 Download [bigint.h](bigint.h) to your working directory and include it.

### Construction

 From integer literals (up to long long)
```cpp
x = 9462362647;
x = -9462362647;
```

 From string literals
```cpp
x = "9462362647"; // does not work because const char* conflicts with long long

// Use these instead
x = std::string("9462362647");
x = BigInt("9462362647");
x = "9462362647"_N;
x = "-9462362647"_N;
```

### Arithmetic operations examples

Factorial of 100
```cpp
BigInt x = 1;
for (BigInt i = 2; i <= 100; i++) x *= i;
std::cout << x << '\n';
```

Output:
```
9332621...(158 digits)
```

500th fibonacci number
```cpp
BigInt last = 0, current = 1;
for (BigInt i = 2; i <= 500; i++) {
    BigInt next = last + current;
    last = current;
    current = next;
}
std::cout << current << '\n';
```

Output
```
1394232...(105 digits)
```

### Binary operations examples

Popcount (count number of set bit)
```cpp
BigInt x = 
    "73086464360023780882673032521418893232027492683609"
    "59856979754918308001067438239264676441905590147466"
    "11142268275451620299424657792591853521328628130486"
    "99896179762587328476774585962790097287531937391133"
    "21508956671487680577026783056695345897644618723791"
    "50385078336836700355139513200459082974750893348108"
    "76710930587907958417310347056715199414332960389780"
    "25061452170976837720035028106172214358060585125050"
    "81949516004385986922371585980745374386064107263952"
    "96940303818897733791815776431886498732972138893052"_N;
BigInt count = 0;
while (x > 0) {
    count++;
    x &= (x - 1);
}
std::cout << count << '\n';
```

Output:
```
862
```

Binary Exponentiation
```cpp
BigInt a = 69, b = 420, res = 1;
while (b > 0) {
    if ((b & 1) > 0) res = res * a;
    a = a * a;
    b >>= 1;
}
std::cout << res << '\n';
```

Output:
```
2073089...(773 digits)
```

### Binary conversion

```cpp
BigInt::to_binary();
```

Example
```cpp
std::cout << "666182560353385381510864290614"_N.to_binary() << '\n';
```

Output:
```
1000011010001000110101001100011000000101101011010100100001000110011000001110111101011100001100110110
```

## In development

 - [ ] Add single digit (base 2^64) multiplication and division for faster base conversion.
 - [x] Improve the Karatsuba algorithm.
 - [ ] Use a faster division algorithm than the current binary/school algorithm. 
 - [ ] Add hexademical, octal conversion and binary input.

