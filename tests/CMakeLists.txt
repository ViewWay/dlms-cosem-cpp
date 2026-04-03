cmake_minimum_required(VERSION 3.16)
enable_testing()
find_package(GTest REQUIRED)

# Test executables
macro(dlms_test name sources)
    add_executable(${name} ${sources})
    target_link_libraries(${name} PRIVATE dlms-cosem GTest::GTest GTest::Main)
    if(TARGET dlms-impl)
        target_link_libraries(${name} PRIVATE dlms-impl)
    endif()
    add_test(NAME ${name} COMMAND ${name})
endmacro()

file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp)
foreach(src ${TEST_SOURCES})
    get_filename_component(test_name ${src} NAME_WE)
    dlms_test(${test_name} ${src})
endforeach()
