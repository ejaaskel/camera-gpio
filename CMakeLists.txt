cmake_minimum_required (VERSION 2.8.11)
project (camera-gpio)

add_executable (camera-gpio src/camera_gpio.c)

install(TARGETS camera-gpio DESTINATION bin)
