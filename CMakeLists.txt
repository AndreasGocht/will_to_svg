project(will_to_svg)

cmake_minimum_required(VERSION 3.2)

find_package(Protobuf REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS will.proto)

add_executable(will_to_svg main.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(will_to_svg ${Protobuf_LIBRARIES} zip)

install (TARGETS will_to_svg RUNTIME DESTINATION bin)

