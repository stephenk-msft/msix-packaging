# MSIX\src\test
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

# Copy the test packages to the test output directory to don't have have to use
# ..\..\src\test\testData\<path> in the test 
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/testData" DESTINATION "${MSIX_TEST_OUTPUT_DIRECTORY}")

add_subdirectory(api)
add_subdirectory(msixtest)
