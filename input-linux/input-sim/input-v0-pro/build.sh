#!/bin/bash
# build.sh

echo "Building Input Subsystem Simulator..."

gcc -o input_simulator \
    main_test.c \
    input_core.c \
    evdev_handler.c \
    joydev_handler.c \
    keyboard_driver.c \
    user_space_test.c \
    -lpthread -g

if [ $? -eq 0 ]; then
    echo "Build successful! Running simulator..."
    echo "=========================================="
    ./input_simulator
else
    echo "Build failed!"
fi
