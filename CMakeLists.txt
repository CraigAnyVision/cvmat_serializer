cmake_minimum_required(VERSION 3.13)
project(mat_serializer)

set(CMAKE_CXX_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS serialization REQUIRED)

set(TARGET mat_serializer)
add_executable(${TARGET} serialize_test.cpp)
target_link_libraries(${TARGET} ${Boost_LIBRARIES} ${OpenCV_LIBS})

set(TARGET client)
add_executable(${TARGET} client.cpp)
target_link_libraries(${TARGET} Threads::Threads ${Boost_LIBRARIES} ${OpenCV_LIBS})

set(TARGET server)
add_executable(${TARGET} server.cpp)
target_link_libraries(${TARGET} Threads::Threads ${Boost_LIBRARIES} ${OpenCV_LIBS})
