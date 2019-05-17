# MSIX\test\msixtest
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
project (msixtest)

if(WIN32)
    set(DESCRIPTION "msixtest manifest")
    configure_file(${CMAKE_PROJECT_ROOT}/manifest.cmakein ${MSIX_TEST_OUTPUT_DIRECTORY}/${PROJECT_NAME}.exe.manifest CRLF)
    set(MANIFEST ${MSIX_TEST_OUTPUT_DIRECTORY}/${PROJECT_NAME}.exe.manifest)
endif()

set(MsixTestsFiles)

list(APPEND MsixTestsFiles
    msixtest.cpp
    unpack.cpp
)

if (WIN32)
    list(APPEND MsixTestsFiles
        "PAL/File/WIN32/FileHelper.cpp"
    )
else()
    list(APPEND MsixTestsFiles
        "PAL/File/POSIX/FileHelper.cpp"
    )
endif()

# For mobile, we create a shared library that will be added to the apps to be
# tested. For non-mobile we create a msixtest executable and we provide our own 
# main, which is for a forwarder to msixtest_main.
if (AOSP OR IOS)
    add_library(${PROJECT_NAME} SHARED
        ${MsixTests}
        )   
else()
    add_executable(${PROJECT_NAME}
        main.cpp
        ${MsixTestsFiles}
        ${MANIFEST}
        )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/src/msix ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# Output test binaries into a test directory
set_target_properties(${PROJECT_NAME} PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${MSIX_TEST_OUTPUT_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${MSIX_TEST_OUTPUT_DIRECTORY}"
  RUNTIME_OUTPUT_DIRECTORY "${MSIX_TEST_OUTPUT_DIRECTORY}"
)

add_dependencies(${PROJECT_NAME} msix)
target_link_libraries(${PROJECT_NAME} msix)
