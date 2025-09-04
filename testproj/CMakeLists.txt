cmake_minimum_required(VERSION 3.10)
project(MyProject VERSION 1.0.0 LANGUAGES CXX)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Настройки компилятора
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -O2
    )
endif()

# Рекурсивный поиск исходных файлов
aux_source_directory(src SOURCES)

# Альтернативный способ (более предпочтительный)
# file(GLOB SOURCES "src/*.cpp")
# file(GLOB HEADERS "src/*.h" "src/*.hpp")

# Создание исполняемого файла
add_executable(${PROJECT_NAME} ${SOURCES})

# Включение директории с заголовочными файлами
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Дополнительные настройки
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Установка выходной директории
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)