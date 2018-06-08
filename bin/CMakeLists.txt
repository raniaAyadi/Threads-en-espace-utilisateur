set (SRCPATH ${CMAKE_CURRENT_SOURCE_DIR}/../src/)

include_directories (${SRCPATH})

if(USE_PTHREAD EQUAL 0)
  add_executable(example
    ${SRCPATH}example.c ${SRCPATH}thread.c)
  target_link_libraries(example rt)
endif()

