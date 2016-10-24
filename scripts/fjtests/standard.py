from fjtests import *

"""
Standard tests for threadpool, 
with quicksort, mergesort, nqueens as the benchmarked tests.
"""

seed=str(43)
large_sort_size = 200000000
medium_sort_size = large_sort_size/10
small_sort_size = large_sort_size/100

tests = [
    threadpool_test(
        name="basic1",
        command="./threadpool_test",
        description="Basic functionality testing (1)",
        is_required = True,
        runs=[
            test_run(name="basic test 1", args=[], thread_count=[1,2,4])
        ]
    ),
    threadpool_test(
        name="basic2",
        command="./threadpool_test2",
        description="Basic functionality testing (2)",
        is_required = True,
        runs=[
            test_run(name="basic test 2", args=[], thread_count=[1,2,4])
        ]
    ),
    threadpool_test(
        name="basic3",
        command="./threadpool_test3",
        description="Basic functionality testing (3)",
        is_required = True,
        runs=[
            test_run(name="basic test 3", args=[], thread_count=[1,2,4])
        ]
    ),
    threadpool_test(
        name="basic4",
        command="./threadpool_test4",
        description="Basic functionality testing (4)",
        is_required = True,
        runs=[
            test_run(name="basic test 4", args=[], thread_count=[2,4])
        ]
    ),
    threadpool_test(
        name="basic5",
        command="./threadpool_test5",
        description="Basic functionality testing (5)",
        is_required = True,
        runs=[
            test_run(name="basic test 5", args=[], thread_count=[2,4])
        ]
    ),
    threadpool_test(
        name="mergesort",
        command="./mergesort",
        description="parallel mergesort",
        runs=[
            test_run(name="mergesort small", args=["-s", seed, str(small_sort_size)]),
            test_run(name="mergesort medium", args=["-s", seed, str(medium_sort_size)]),
            test_run(name="mergesort large", args=["-s", seed, str(large_sort_size)], 
                thread_count=[5,10,20], is_benchmarked=True, timeout=60),
        ]
    ),
    threadpool_test(
        name="quicksort",
        command="./quicksort",
        description="parallel quicksort",
        runs=[
            test_run(name="quicksort small", args=["-s", seed, "-d", "16", str(small_sort_size)]),
            test_run(name="quicksort medium", args=["-s", seed, "-d", "16", str(medium_sort_size)]),
            test_run(name="quicksort large", args=["-s", seed, "-d", "16", str(large_sort_size)], 
                thread_count=[5,10,20], is_benchmarked=True, timeout=60),
        ]
    ),
    threadpool_test(
        name="psum",
        command="./psum_test",
        description="parallel sum using divide-and-conquer",
        runs=[
            test_run(name="psum_test small", args=["10000000"]),
            test_run(name="psum_test medium", args=["100000000"]),
            test_run(name="psum_test large", args=["1000000000"], thread_count=[4,8,16], timeout=60),
        ]
    ),
    threadpool_test(
        name="nqueens",
        command="./nqueens",
        description="parallel n-queens solver",
        runs=[
            test_run(name="nqueens 11", args=["11"]),
            test_run(name="nqueens 12", args=["12"], timeout=60),
            test_run(name="nqueens 13", args=["13"], thread_count=[5,10,20], 
                is_benchmarked=True, timeout=60),
        ]
    ),
    threadpool_test(
        name="fibonacci",
        command="./fib_test",
        description="parallel fibonacci toy test",
        runs=[
            test_run(name="fibonacci 30", args=["30"], timeout=60),
            test_run(name="fibonacci 38", args=["38"], thread_count=[1,2,4], timeout=60),
        ]
    ),
]
