def fib(n):
    """Print the Fibonacci series up to n."""

    cdef int test = 3
    a, b = 0, 1
    while b < n:
        print(test)
        print b,
        a, b = b, a + b