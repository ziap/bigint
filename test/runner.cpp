#include "../bigint.h"

int main() {
    BigInt x, y;
    std::cin >> x >> y;
    std::cout << x << '\n';
    std::cout << y << '\n';
    std::cout << ~x << '\n';
    std::cout << ~y << '\n';
    std::cout << (x & y) << '\n';
    std::cout << (x | y) << '\n';
    std::cout << (x ^ y) << '\n';
    std::cout << (x + y) << '\n';
    std::cout << (x - y) << '\n';
    std::cout << (x * y) << '\n';
    std::cout << (x / y) << '\n';
    std::cout << (((x % y) + y) % y) << '\n';  // Python compatibility
    std::cout << (x > y ? "True" : "False") << '\n';
    std::cout << (x < y ? "True" : "False") << '\n';
    std::cout << (x >= y ? "True" : "False") << '\n';
    std::cout << (x <= y ? "True" : "False") << '\n';
}