cmake_minimum_required(VERSION 3.13)
project(mat_serializer)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS serialization REQUIRED)

set(TARGET mat_serializer)
add_executable(${TARGET} main.cpp)
target_link_libraries(${TARGET} ${Boost_LIBRARIES} ${OpenCV_LIBS})
