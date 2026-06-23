cmake_minimum_required(VERSION 3.16)
project(DijkstraVisualizer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
message(STATUS "Found Qt6")

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/GraphCanvas.cpp
    src/Graph.cpp
    src/Vertex.cpp
    src/Edge.cpp
    src/DijkstraSolver.cpp
)

set(HEADERS
    src/MainWindow.h
    src/GraphCanvas.h
    src/Graph.h
    src/Vertex.h
    src/Edge.h
    src/DijkstraSolver.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources)
    file(COPY resources DESTINATION ${CMAKE_BINARY_DIR}/bin)
endif()

# Test for specification graph (console only, no GUI)
set(TEST_SOURCES
    test_spec.cpp
    src/DijkstraSolver.cpp
    src/Graph.cpp
    src/Vertex.cpp
    src/Edge.cpp
)
add_executable(DijkstraSpecTest ${TEST_SOURCES})
target_link_libraries(DijkstraSpecTest Qt6::Core)
set_target_properties(DijkstraSpecTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)