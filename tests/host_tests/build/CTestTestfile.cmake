# CMake generated Testfile for 
# Source directory: /workspaces/code/tests/host_tests
# Build directory: /workspaces/code/tests/host_tests/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(HALAbstractionBasicTest "/workspaces/code/tests/host_tests/build/test_hal_abstraction_basic_corrected")
set_tests_properties(HALAbstractionBasicTest PROPERTIES  LABELS "hal-abstraction;basic;architecture" TIMEOUT "30" _BACKTRACE_TRIPLES "/workspaces/code/tests/host_tests/CMakeLists.txt;51;add_test;/workspaces/code/tests/host_tests/CMakeLists.txt;0;")
