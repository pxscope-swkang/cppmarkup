cmake_minimum_required(VERSION 3.1)

## --------------------------- AUTOMATED TESTS


## --------------------------- TRIVIAL TESTS
add_executable(poc-static_inline poc-static_inline.cpp)
target_compile_features(poc-static_inline PRIVATE cxx_std_20)
