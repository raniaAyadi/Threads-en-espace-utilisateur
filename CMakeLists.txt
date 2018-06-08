cmake_minimum_required (VERSION  2.8.11)
include (CheckCCompilerFlag)
enable_testing()

project (THREADS_IN_USER_SPACE)

find_package(Threads REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -O0")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

if(CMAKE_BUILD_TYPE STREQUAL "pthread")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_PTHREAD -Wall -g -O0 -pthread")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
  set(USE_PTHREAD 1)
  message("Compilation mode : pthread")
elseif(CMAKE_BUILD_TYPE STREQUAL "postpone")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPOSTPONE_EXEC -Wall -g -O0 -pthread")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
  set(USE_PTHREAD 0)
  set(POSTPONE 1)
  message("Compilation mode : Default - postpone")
else()
  message("Compilation mode : Default")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -O0")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
  set(POSTPONE 0)
  set(USE_PTHREAD 0)
endif()


include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tst)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/bin)
