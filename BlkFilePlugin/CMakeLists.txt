
cmake_minimum_required(VERSION 3.12)
project(blk)

#add_executable(blk)


option(BlkFilePlugin_SHARED_LIBRARY "Set to ON to build using dynamic linking" ON)
if(BlkFilePlugin_SHARED_LIBRARY)
  #add_definitions(-DVMCORE_SHARED_LIBRARY)
  add_library(blk SHARED)
  target_compile_definitions(blk PUBLIC -DVMCORE_SHARED_LIBRARY)
else()
  add_library(blk STATIC)
endif()

target_sources(blk PRIVATE "BlkFilePluginInterface.cpp")
target_link_libraries(blk vmcore)
target_include_directories(blk PRIVATE "../VMCore/extension" "../VMCore/include")

install(TARGETS blk LIBRARY DESTINATION "lib" RUNTIME DESTINATION "bin/plugins" ARCHIVE DESTINATION "lib")
