cmake_minimum_required(VERSION 3.16)
project(dlms-cosem-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optional: OpenSSL for AES-GCM
find_package(OpenSSL COMPONENTS crypto QUIET)
if(OpenSSL_FOUND)
    add_definitions(-DDLMS_HAS_OPENSSL=1)
endif()

# Header-only library interface
add_library(dlms-cosem INTERFACE)
target_include_directories(dlms-cosem INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(dlms-cosem INTERFACE cxx_std_17)

# Compiled sources (non-header-only)
file(GLOB COMPILED_SOURCES
    src/dlms/hdlc/*.cpp
    src/dlms/axdr/*.cpp
    src/dlms/asn1/*.cpp
    src/dlms/security/sm4*.cpp
    src/dlms/security/security.cpp
    src/dlms/transport/*.cpp
    src/dlms/client/*.cpp
)
if(COMPILED_SOURCES)
    add_library(dlms-impl STATIC ${COMPILED_SOURCES})
    target_link_libraries(dlms-impl PUBLIC dlms-cosem)
    if(OpenSSL_FOUND)
        target_link_libraries(dlms-impl PUBLIC OpenSSL::Crypto)
    endif()
endif()

# Testing
enable_testing()
find_package(GTest QUIET)
if(GTest_FOUND)
    add_subdirectory(tests)
endif()
