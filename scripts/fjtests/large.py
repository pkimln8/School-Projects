from fjtests import *

import socket
"""
Large, machine dependent tests.

Only the benchmarked ones here.
"""

seed=str(43)
large_sort_size = 200000000
thread_count = [5, 10, 20]

large_amd_nodes = ['fir.rlogin', 'sourwood.rlogin']
if socket.gethostname() in large_amd_nodes:
    thread_count = [8, 16, 32, 64]

tests = [
    threadpool_test(
        name="mergesort",
        command="./mergesort",
        description="parallel mergesort",
        runs=[
            test_run(name="mergesort large", args=["-s", seed, str(large_sort_size)], 
                thread_count=thread_count, is_benchmarked=True, timeout=60),
        ]
    ),
    threadpool_test(
        name="quicksort",
        command="./quicksort",
        description="parallel quicksort",
        runs=[
            test_run(name="quicksort large", args=["-s", seed, "-d", "16", str(large_sort_size)], 
                thread_count=thread_count, is_benchmarked=True, timeout=60),
        ]
    ),
    threadpool_test(
        name="nqueens",
        command="./nqueens",
        description="parallel n-queens solver",
        runs=[
            test_run(name="nqueens 13", args=["13"], thread_count=thread_count ,
                is_benchmarked=True, timeout=60),
        ]
    )
]
