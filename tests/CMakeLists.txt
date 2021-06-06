cmake_minimum_required(VERSION 3.1)

## --------------------------- AUTOMATED TESTS
aux_source_directory        (automation                 AUTOMATION_SOURCES)
set                         (TARGET_NAME                cppmarkup-automated-tests)

add_executable              (cppmarkup-automated-tests     ${AUTOMATION_SOURCES})
target_link_libraries       (cppmarkup-automated-tests     PUBLIC      cppmarkup::cppmarkup)

if(MSVC)
    target_compile_options  (${TARGET_NAME}              PRIVATE     "/W4")
endif()

## --------------------------- TRIVIAL TESTS
add_executable(poc-static_inline poc-static_inline.cpp)
target_compile_features(poc-static_inline PRIVATE cxx_std_20)
