cmake_minimum_required(VERSION 3.0)

project(nsroot)
enable_language(C)

file(GLOB src src/*.c)

add_executable(nsroot ${src})
install(TARGETS nsroot RUNTIME DESTINATION bin)
