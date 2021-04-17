cmake_minimum_required      (VERSION 3.1)
project                     (nlohmann_json)

add_library                 (nlohmann_json      INTERFACE)
target_include_directories  (nlohmann_json      INTERFACE           ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features     (nlohmann_json      INTERFACE           cxx_std_11)    

add_library                 (nlohmann_json::nlohmann_json ALIAS nlohmann_json)