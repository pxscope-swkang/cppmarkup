cmake_minimum_required(VERSION 3.1)

## --------------------------- AUTOMATED TESTS
aux_source_directory        (automation                 AUTOMATION_SOURCES)

add_executable              (ezdata-automated-tests     ${AUTOMATION_SOURCES})
add_dependencies            (ezdata-automated-tests     ezdata::static)

target_link_libraries       (ezdata-automated-tests     PRIVATE     ezdata::static)
target_include_directories  (ezdata-automated-tests     PRIVATE     ezdata::static)

## --------------------------- TRIVIAL TESTS
add_executable(poc-static_inline poc-static_inline.cpp)
target_compile_features(poc-static_inline PRIVATE cxx_std_20)
