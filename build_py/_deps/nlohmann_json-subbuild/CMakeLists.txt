# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.22.1)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(nlohmann_json-populate NONE)



include(ExternalProject)
ExternalProject_Add(nlohmann_json-populate
                     "UPDATE_DISCONNECTED" "False" "URL" "https://github.com/nlohmann/json/releases/download/v3.7.3/include.zip" "URL_HASH" "SHA256=87b5884741427220d3a33df1363ae0e8b898099fbc59f1c451113f6732891014"
                    SOURCE_DIR          "/home/iadc/slippage-preserving-reshaping/build_py/_deps/nlohmann_json-src"
                    BINARY_DIR          "/home/iadc/slippage-preserving-reshaping/build_py/_deps/nlohmann_json-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)


