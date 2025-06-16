cmake_minimum_required(VERSION 3.10)
project(HistogramShiftSteganography)

add_definitions(-DUNICODE -D_UNICODE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Если используем vcpkg
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Найти OpenCV4 (через vcpkg)
find_package(OpenCV REQUIRED)
find_package(doctest CONFIG REQUIRED)

# Добавить исполняемый файл
add_executable(project_steg main.cpp steganography.cpp)
add_executable(stega_test stega_test.cpp steganography.cpp)
add_subdirectory(external)

target_link_libraries(stega_test PRIVATE ${OpenCV_LIBS})
target_include_directories(stega_test PRIVATE ${OpenCV_INCLUDE_DIRS})
# Включить заголовочные файлы и линковка
target_link_libraries(stega_test PRIVATE doctest::doctest)
target_include_directories(stega_test PRIVATE ${doctest_DIR})
target_include_directories(project_steg PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(project_steg PRIVATE ${OpenCV_LIBS})


enable_testing()
add_test(NAME stega_test COMMAND stega_test --force-colors -d)

