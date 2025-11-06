    cmake_minimum_required(VERSION 4.0)
    project(DataStructure)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

    set(CMAKE_CXX_STANDARD 20)

    set(HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/header")
    set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")

    file(GLOB_RECURSE HEADER_FILES ${HEADER_DIR}/*.h ${HEADER_DIR}/*.hpp)
    file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.c ${SOURCE_DIR}/*.cpp)



    add_library(DataStructure STATIC)
    target_sources(DataStructure
            PUBLIC
            ${HEADER_FILES}
            PRIVATE
            ${SOURCE_FILES}
    )
    target_include_directories(DataStructure PUBLIC ${HEADER_DIR} PRIVATE ${CMAKE_BINARY_DIR})

    add_executable(TestDataStructure main.cpp)
    target_link_libraries(TestDataStructure PUBLIC DataStructure)

