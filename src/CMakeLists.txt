
cmake_minimum_required(VERSION 3.12)
project(brg)

add_executable(brg)
target_sources(brg PRIVATE "main.cpp")
target_link_libraries(brg vmcore)
target_include_directories(brg PRIVATE "../VMCore/extension" "../VMCore/include")

install(TARGETS brg LIBRARY DESTINATION "lib" RUNTIME DESTINATION "bin" ARCHIVE DESTINATION "lib")
