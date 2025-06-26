cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(qsox VERSION 1.0.0)

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

CPMAddPackage("gh:geode-sdk/result@1.3.2")
CPMAddPackage("gh:fmtlib/fmt#11.2.0")

target_link_libraries(${PROJECT_NAME} PUBLIC GeodeResult fmt)
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX=1)

# weird stuff

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)
