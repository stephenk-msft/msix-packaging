# MSIX\src\msix\prototype
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(prototype)

add_executable(${PROJECT_NAME} 
    PAL/Directory/Windows/DirectoryRead.cpp
    PAL/XML/Xerces/XmlWriter.cpp
    PAL/SHA256/Win32/SHA256.cpp
    BlockMapWriter.cpp
    ContentTypeWriter.cpp
    ZipObject.cpp
    main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE inc) # this is the inc under prototype

# windows specific
target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt crypt32 wintrust)

# zlib
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlib
    ${CMAKE_PROJECT_ROOT}/lib/zlib
    ${CMAKE_PROJECT_ROOT}/src/msix/PAL/DataCompression/Zlib
)
target_link_libraries(${PROJECT_NAME} PRIVATE zlibstatic)

# xerces
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/xerces/src
    ${CMAKE_PROJECT_ROOT}/lib/xerces/src
)
target_link_libraries(${PROJECT_NAME} PRIVATE xerces-c)
