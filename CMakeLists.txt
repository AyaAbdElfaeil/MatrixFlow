cmake_minimum_required(VERSION 3.10)
project(MatrixFlow VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Core library sources
set(CORE_SOURCES
    src/Fraction.cpp
    src/Matrix.cpp
    src/GaussianEliminator.cpp
    src/SystemClassifier.cpp
    src/InputValidator.cpp
)

# Main desktop visualizer application
add_executable(matrixflow
    ${CORE_SOURCES}
    src/JsonHelper.cpp
    src/HttpServer.cpp
    src/main.cpp
)

if (WIN32)
    target_link_libraries(matrixflow ws2_32)
endif()

# Standalone test runner
add_executable(test_runner
    ${CORE_SOURCES}
    tests/test_main.cpp
)
