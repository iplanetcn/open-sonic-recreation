cmake_minimum_required(VERSION 3.25)

project(open-sonic-recreation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(SFML COMPONENTS system window graphics network audio REQUIRED)

file(GLOB src "src/*.cpp" "src/*.h")
file(COPY assets DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${src})
target_link_libraries(${PROJECT_NAME}
        sfml-system
        sfml-window
        sfml-graphics
        sfml-network
        sfml-audio)