#!/bin/bash
# Install GoogleTest (C++ unit test framework)
GTEST_REPO=https://github.com/google/googletest.git
GTEST_DIR=/opt/googletest
if [ ! -d "$GTEST_DIR" ]; then
    git clone $GTEST_REPO $GTEST_DIR
    cd $GTEST_DIR
    cmake .
    make
fi
