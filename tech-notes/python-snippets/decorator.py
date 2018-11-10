#!/usr/bin/env python3

import os
import time

# timer decorator to wrap arbitrary function whose call you want to time
def timer_decorator_func(func):
    # *args unpacks to tuple, **kwargs unpacks to dictionary
    def wrap_in_timer(*args, **kwargs):
        start = time.time()
        res = func(*args)
        end = time.time()
        print('took ' + str(end - start) + ' seconds')
        return res
    return wrap_in_timer

def run(x):
    print('call run')
    time.sleep(x)
    return

@timer_decorator_func
def run_decorated(x):
    return run(x)

@timer_decorator_func
def wait_func():
    time.sleep(1)
    return

# cache decorator with exclude to demonstrate parameters in decorators
# (takes parameter, returns decorator)
def cache_with_exclude_decorator(exclude):
    def cache_decorator(func):
        cache = dict()
        def cache_func(*args, **kwargs):
            if args in cache and not args in exclude:
                print("Note: Returning cached data.")
                return cache[args]
            else:
                # Call the function and cache the result for next calls
                res = func(*args, **kwargs)
                cache[args] = res
                return res
        return cache_func
    return cache_decorator

@cache_with_exclude_decorator([(3,)])
def factorial(x):
    total = 1
    for i in range(2, x + 1):
        total *= i
    return total

if __name__ == "__main__":
    run(0.1)
    
    decorated_run_var = timer_decorator_func(run)
    # the @ syntax is a shorthand for this
    
    decorated_run_var(0.1)
    
    run_decorated(0.1)

    wait_func()

    print(factorial(3))
    print(factorial(3))
    print(factorial(4))
    print(factorial(4))