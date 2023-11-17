function(add_benchmark BENCH_NAME SOURCE)
    add_executable(${BENCH_NAME} ${SOURCE})
    target_link_libraries(${BENCH_NAME}
        PRIVATE
            benchmark
            Threads::Threads)
endfunction()

add_benchmark(bench_example bench_example.cpp)
