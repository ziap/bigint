from os import system
from random import randint
from subprocess import Popen, PIPE
from time import time


def random_digits(n):
    range_start = 10**(n-1)
    range_end = (10**n)-1

    rand = randint(range_start, range_end)
    if randint(0, 1):
        rand = -rand

    return rand


def run_test(n, x, y):
    print('Running test', n)
    start_time = time()

    process = Popen(['./runner'], stdin=PIPE, stdout=PIPE)
    process.stdin.write('{} {}\n'.format(x, y).encode('utf-8'))
    stdout, stderr = process.communicate()
    process.kill()

    result = stdout.decode('utf-8').split()

    expected = [
        str(i) for i in [
            x,
            y,
            ~x,
            ~y,
            x & y,
            x | y,
            x ^ y,
            x + y,
            x - y,
            x * y,
            # floor(a / n) != a // n if (a and n have different sign)
            # floor(a / n) cast a and n to float which has insufficient precision
            # workaround with abs used
            # (x // abs(x)) is the sign of x
            abs(x) // abs(y) * (x // abs(x)) * (y // abs(y)),
            x % y,
            x > y,
            x < y,
            x >= y,
            x <= y
        ]
    ]

    correct = True

    for i in range(len(expected)):
        if result[i] == expected[i]:
            print('✔️', '\033[92mCorrect!')
        else:
            print('❌', '\033[91mExpected: {}\n   Found:    {}'.format(
                expected[i], result[i]))
            correct = False

    if not correct:
        print('\033[91mTest failed')
        print('\033[0;0mInputs:', x, y)
        exit(1)

    print("\033[0;0mTest {} passed after {:.2f} seconds".format(
        n, time() - start_time))
    print()


if __name__ == '__main__':
    for i in range(100):
        run_test(i + 1, random_digits(10), random_digits(10))
    for i in range(100):
        run_test(i + 101, random_digits(20), random_digits(10))
    for i in range(100):
        run_test(i + 201, random_digits(10), random_digits(20))
    for i in range(100):
        run_test(i + 301, random_digits(100), random_digits(100))
    for i in range(100):
        run_test(i + 401, random_digits(200), random_digits(100))
    for i in range(100):
        run_test(i + 501, random_digits(100), random_digits(200))
    for i in range(100):
        run_test(i + 601, random_digits(1000), random_digits(1000))
    print("\033[92mAll tests passed!")
