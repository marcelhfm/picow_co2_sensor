cmake_minimum_required(VERSION 3.28)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_NAME "picow_co2_sensor")
set(BUILD_NUMBER "1")

set(APP_NAME "co2")
set(APP_VERSION_NUMBER "0.0.2")

set(APP_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/app")

add_compile_definitions($<$<CONFIG:Debug>:DEBUG=1>)

set(FREERTOS_CFG_DIRECTORY "${CMAKE_SOURCE_DIR}/config")
set(FREERTOS_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel")

include(pico_sdk_import.cmake)

project(${PROJECT_NAME}
    LANGUAGES C CXX ASM
    DESCRIPTION "PicoW CO2 Sensor"   
    )

  pico_sdk_init()

option(ENABLE_DEBUG_LOGS "Enable debug logging" OFF)

if (ENABLE_DEBUG_LOGS)
    add_definitions(-DENABLE_DEBUG_LOGS)
endif()

add_library(FreeRTOS STATIC
    ${FREERTOS_SRC_DIRECTORY}/event_groups.c
    ${FREERTOS_SRC_DIRECTORY}/list.c
    ${FREERTOS_SRC_DIRECTORY}/queue.c
    ${FREERTOS_SRC_DIRECTORY}/stream_buffer.c
    ${FREERTOS_SRC_DIRECTORY}/tasks.c
    ${FREERTOS_SRC_DIRECTORY}/timers.c
    ${FREERTOS_SRC_DIRECTORY}/portable/MemMang/heap_3.c
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0/port.c
)

target_include_directories(FreeRTOS PUBLIC
    ${FREERTOS_CFG_DIRECTORY}/
    ${FREERTOS_SRC_DIRECTORY}/include
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0
)

add_subdirectory(${APP_SRC_DIRECTORY})
