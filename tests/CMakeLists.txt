cmake_minimum_required(VERSION 3.1)

## --------------------------- AUTOMATED TESTS
aux_source_directory        (automation                 AUTOMATION_SOURCES)

add_executable              (cppmarkup-automated-tests     ${AUTOMATION_SOURCES})
add_dependencies            (cppmarkup-automated-tests     cppmarkup::static)

target_link_libraries       (cppmarkup-automated-tests     PUBLIC      cppmarkup::static)
target_include_directories  (cppmarkup-automated-tests     PUBLIC      cppmarkup::static)

## --------------------------- TRIVIAL TESTS
add_executable(poc-static_inline poc-static_inline.cpp)
target_compile_features(poc-static_inline PRIVATE cxx_std_20)
