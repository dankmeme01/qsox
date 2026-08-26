cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(qsox VERSION 1.0.0)

option(QSOX_BUILD_TESTS "Build tests" OFF)

if (WIN32)
    file(GLOB OS_SOURCES "src/win32/*.cpp")
else()
    file(GLOB OS_SOURCES "src/unix/*.cpp")
endif()

file(GLOB SOURCES
	src/*.cpp
    ${OS_SOURCES}
)

file(GLOB_RECURSE HEADERS
    include/*.hpp
)

if (NOT CURRENT_CPM_VERSION)
    include(cmake/CPM.cmake)
endif()

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

if (WIN32)
    # fix debug builds
    target_compile_definitions(${PROJECT_NAME} PRIVATE _HAS_ITERATOR_DEBUGGING=0)
    target_link_libraries(${PROJECT_NAME} PUBLIC ws2_32) # link to winsock
endif()

if (NOT TARGET GeodeResult)
    CPMAddPackage("gh:geode-sdk/result@1.4.1")
endif()
if (NOT TARGET fmt)
    CPMAddPackage("gh:fmtlib/fmt#12.1.0")
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC GeodeResult)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX=1)

# weird stuff

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

if (QSOX_BUILD_TESTS)
    # unit tests (gtest)

    enable_testing()

    CPMAddPackage("gh:google/googletest@1.18.0")

    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "test/*.cpp")
    add_executable(qsox_tests ${TEST_SOURCES})
    target_link_libraries(qsox_tests PRIVATE ${PROJECT_NAME} gtest_main)

    include(GoogleTest)
    gtest_discover_tests(qsox_tests)
endif()
