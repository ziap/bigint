from subprocess import Popen, PIPE
from time import time
from os import system
from random import randint

def random_digits(n):
    range_start = 10**(n-1)
    range_end = (10**n)-1

    rand = randint(range_start, range_end)
    if randint(0, 1): rand = -rand

    return rand

def run_test(n, x, y):
    print('Running test', n)
    start_time = time()

    process = Popen(['./runner'], stdin=PIPE, stdout=PIPE)
    process.stdin.write('{num1} {num2}\n'.format(num1 = x, num2 = y).encode('utf-8'))
    stdout, stderr = process.communicate()

    result = stdout.decode('utf-8').split()

    expected = [str(i) for i in [x, y, ~x, ~y, x & y, x | y, x ^ y, x + y, x - y, x * y, int(x / y), x % y]]

    correct = True

    for i in range(len(expected)):
        if result[i] == expected[i]: print('✔️ ', '\033[92mCorrect!')
        else: 
            print('❌', '\033[91mExpected: {exp}\n   Found:    {res}'.format(exp = expected[i], res = result[i]))
            correct = False

    process.kill()

    if not correct: 
        print('\033[91mTest failed')
        exit(1)

    print("\033[0;0mTest {n} passed after {sec:.2f} seconds".format(n = n, sec = time() - start_time))
    print()

if __name__ == '__main__':
    for i in range(20):
        run_test(i + 1, random_digits(1000), random_digits(1000))
    print("\033[92mAll tests passed!")