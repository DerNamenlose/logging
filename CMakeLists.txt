project(loggerv2)

add_executable(loggerv2 main.cpp)

set(CMAKE_CXX_FLAGS "-std=c++11")

install(TARGETS loggerv2 RUNTIME DESTINATION bin)
