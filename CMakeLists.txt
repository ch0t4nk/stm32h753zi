cmake_minimum_required(VERSION 3.15)
project(stm32h753_ihm02a1)

# TODO: See `.instructions/CMakeLists.txt.md` for module-specific build logic

add_subdirectory(examples/stepper_control)
add_subdirectory(examples/encoder_read)
add_subdirectory(examples/api_demo)
add_subdirectory(examples/can_api_demo)

# Add other subdirectories as needed
