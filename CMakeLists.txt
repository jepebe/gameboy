cmake_minimum_required(VERSION 3.1)

project(ModernC C)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

#add_executable(heron src/heron.c)
set(sources 
    src/cartridge.c
    src/gbcpu.c 
    src/tools.c
)

add_library(gameboy ${sources})

enable_testing()
configure_file(tests/roms/cpu_instrs.gb roms/cpu_instrs.gb COPYONLY)
include_directories(include)

add_executable(test_cpu tests/test_cpu.c)
target_link_libraries(test_cpu gameboy)
add_test("CPU" test_cpu)