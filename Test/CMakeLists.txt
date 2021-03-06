if(NOT skip_google_test)

  # Download and unpack googletest at configure time
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in googletest-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Test/googletest-download )
  if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Test/googletest-download )
  if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()

  # Prevent overriding the parent project's compiler/linker
  # settings on Windows
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This defines
  # the gtest and gtest_main targets.
  add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                   ${CMAKE_BINARY_DIR}/googletest-build
                   EXCLUDE_FROM_ALL)

endif()

include_directories(${CMAKE_BINARY_DIR}/googletest-src/googletest/include)

add_library(testqrlib SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/Details/examplenetwork.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/testcapacitynetwork.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/testgraphml.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/testqrutils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/testpoissonpointprocess.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/testyen.cpp
)

target_link_libraries(testqrlib
  uiiitqr
  uiiitsupport

  gtest
  ${Boost_LIBRARIES}
)

add_executable(testqr
  ${CMAKE_CURRENT_SOURCE_DIR}/testmain.cpp
)

target_link_libraries(testqr
  testqrlib
)

