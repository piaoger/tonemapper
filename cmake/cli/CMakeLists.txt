

project(tonemapper-cli)

cmake_policy(PUSH)
set(CMAKE_BUILD_TYPE Release)

if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
endif()

add_definitions(-DNOGUI)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/nanogui/ext/eigen
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/filesystem
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/tinyexr
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

set (EXTRA_SOURCE "")
if (WIN32)
    set(EXTRA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/../../res/tonemapper.rc)
elseif(APPLE)
    set(EXTRA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/../../res/tonemapper.icns)
endif()

add_executable(tonemapper-cli
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cli.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main-cli.cpp
    ${EXTRA_SOURCE}
)

set_target_properties(tonemapper-cli PROPERTIES OUTPUT_NAME ../../tonemapper-cli)

cmake_policy(POP)