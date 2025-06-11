cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH

set(PICO_BOARD pico CACHE STRING "Board type")
# note: this must happen before project()
include(pico_sdk_import.cmake)


project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(hello_world
  hello_world.c
)

target_link_libraries(hello_world pico_stdlib)

pico_add_extra_outputs(hello_world)
