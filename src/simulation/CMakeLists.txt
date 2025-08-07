# Simulation Framework CMakeLists.txt
# Hardware simulation library for safe development

# Set minimum CMake version
cmake_minimum_required(VERSION 3.15)

# Define simulation library
add_library(simulation
    hardware_simulation.c
)

# Include directories for simulation
target_include_directories(simulation
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/simulation
)

# Compiler options for simulation
target_compile_options(simulation PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
)

# Add simulation-specific definitions
target_compile_definitions(simulation PRIVATE
    SIMULATION_ENABLED=1
)

# Create standalone test executable for simulation
if(ENABLE_TESTING)
    add_executable(test_simulation
        ${CMAKE_SOURCE_DIR}/tests/test_simulation.c
    )
    
    target_link_libraries(test_simulation
        simulation
    )
    
    target_include_directories(test_simulation PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )
    
    # Add test to CTest
    add_test(
        NAME simulation_basic_test
        COMMAND test_simulation
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    
    # Set test properties
    set_tests_properties(simulation_basic_test PROPERTIES
        TIMEOUT 30
        LABELS "simulation;unit"
    )
endif()

# Add schema dependency for simulation tests
if(TARGET generate_schemas)
    add_dependencies(simulation generate_schemas)
    if(TARGET test_simulation)
        add_dependencies(test_simulation generate_schemas)
    endif()
endif()

# Installation rules for simulation library
install(TARGETS simulation
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES hardware_simulation.h
    DESTINATION include/simulation
)
