cmake_minimum_required(VERSION 3.1)

project(Gameboi C)
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
    src/instructions.c
    src/opcodes.c 
    src/tools.c
)

include_directories(include)

add_library(gameboy ${sources})

enable_testing()

add_executable(test_cartridge tests/test_cartridge.c)
target_link_libraries(test_cartridge gameboy)
add_test("Cartridge" test_cartridge)

add_executable(test_cpu tests/test_cpu.c)
target_link_libraries(test_cpu gameboy)
add_test("CPU" test_cpu)

add_executable(test_roms tests/test_roms.c)
target_link_libraries(test_roms gameboy)
add_test("ROMS" test_roms)

add_executable(test_instructions tests/test_instructions.c)
target_link_libraries(test_instructions gameboy)
add_test("Instructions" test_instructions)

add_executable(test_tools tests/test_tools.c)
target_link_libraries(test_tools gameboy)
add_test("Tools" test_tools)