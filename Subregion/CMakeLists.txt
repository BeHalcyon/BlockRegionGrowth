
cmake_minimum_required(VERSION 3.12)
project(subregion)

add_executable(subregion)
target_sources(subregion PRIVATE "main.cpp")
target_link_libraries(subregion vmcore)
target_include_directories(subregion PRIVATE "../VMCore/extension" "../VMCore/include")

install(TARGETS subregion LIBRARY DESTINATION "lib" RUNTIME DESTINATION "bin" ARCHIVE DESTINATION "lib")
