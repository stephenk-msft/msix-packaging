# MSIX\src\msix\prototype
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(prototype)

if (WIN32)
    set(DirectoryObject PAL/FileSystem/Windows/DirectoryRead.cpp)
    set(Encoding PAL/Encoding/Win32/Encoding.cpp)
else()
    set(DirectoryObject PAL/FileSystem/POSIX/DirectoryRead.cpp)
    set(Encoding PAL/Encoding/OpenSSL/Encoding.cpp)
endif()

add_executable(${PROJECT_NAME} 
    ${DirectoryObject}
    PAL/XML/Xerces/XmlWriter.cpp # maybe in the future we would to a msxml implementation
    ${Encoding}
    BlockMapWriter.cpp
    ContentTypeWriter.cpp
    ZipObject.cpp
    main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE inc) # this is the inc under prototype

# windows specific
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt crypt32 wintrust)
else()
    # include the libraries needed to use OpenSSL
    target_include_directories(${PROJECT_NAME} PRIVATE ${OpenSLL_INCLUDE_PATH})
    if((IOS) OR (MACOS))
        target_link_libraries(${PROJECT_NAME} PRIVATE crypto -Wl,-dead_strip)
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE crypto -Wl,--gc-sections)
    endif()
endif()

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
