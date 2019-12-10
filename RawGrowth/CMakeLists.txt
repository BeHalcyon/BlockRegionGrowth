
cmake_minimum_required(VERSION 3.12)
project(rawgrowth)

add_executable(rawgrowth)
target_sources(rawgrowth PRIVATE "main.cpp")
target_link_libraries(rawgrowth vmcore)
target_include_directories(rawgrowth PRIVATE "../VMCore/extension" "../VMCore/include")

install(TARGETS rawgrowth LIBRARY DESTINATION "lib" RUNTIME DESTINATION "bin" ARCHIVE DESTINATION "lib")
