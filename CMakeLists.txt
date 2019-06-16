cmake_minimum_required(VERSION 3.13)
project(mqtt_sensor.c C)

set(CMAKE_C_STANDARD 99)

add_executable(mqtt_sensor mqtt_sensor.c sensor.c sensor.h "data_format_util.c" "data_format_util.h" message_protocol.c message_protocol.h)

TARGET_LINK_LIBRARIES(mqtt_sensor json-c mosquitto)