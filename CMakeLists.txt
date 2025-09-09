cmake_minimum_required(VERSION 3.10)
project(Parvis)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(tree-sitter-core STATIC
	lib/tree-sitter/lib/src/lib.c
)

target_include_directories(tree-sitter-core PUBLIC
	lib/tree-sitter/lib/include
	lib/tree-sitter/lib/src
)

add_library(tree-sitter-cpp-grammar STATIC
	grammars/tree-sitter-cpp/src/parser.c
	grammars/tree-sitter-cpp/src/scanner.c
)

target_include_directories(tree-sitter-cpp-grammar PUBLIC
	grammars/tree-sitter-cpp/src
)

add_library(tree-sitter-python-grammar STATIC
	grammars/tree-sitter-python/src/parser.c
	grammars/tree-sitter-python/src/scanner.c
)

target_include_directories(tree-sitter-python-grammar PUBLIC
	grammars/tree-sitter-python/src
)

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
	tree-sitter-core
	tree-sitter-cpp-grammar
	tree-sitter-python-grammar
)

target_include_directories(${PROJECT_NAME} PRIVATE
	lib/tree-sitter/lib/include
	grammars/tree-sitter-cpp/src
	grammars/tree-sitter-python/src
)
