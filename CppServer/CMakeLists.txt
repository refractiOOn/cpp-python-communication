cmake_minimum_required(VERSION 3.0.0)
project(CppServer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

add_executable(${PROJECT_NAME}
    main.cpp
    functions.hpp functions.cpp
    Server.hpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

### Windows Implementation Libraries ###
target_include_directories(${PROJECT_NAME} PUBLIC wil/include)
target_link_libraries(${PROJECT_NAME} ws2_32)

### RapidJSON ###
target_include_directories(${PROJECT_NAME} PUBLIC rapidjson/include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
