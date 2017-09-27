cmake_minimum_required(VERSION 2.6)

if ( NOT "${SKIPGUI}" STREQUAL "true")
    add_subdirectory(cmake/gui)
endif()

if ( NOT "${SKIPCLI}" STREQUAL "true")
    add_subdirectory(cmake/cli)
endif()
