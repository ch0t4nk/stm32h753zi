#!/bin/bash
# Install Unity (C unit test framework)
UNITY_REPO=https://github.com/ThrowTheSwitch/Unity.git
UNITY_DIR=/opt/unity
if [ ! -d "$UNITY_DIR" ]; then
    git clone $UNITY_REPO $UNITY_DIR
fi
