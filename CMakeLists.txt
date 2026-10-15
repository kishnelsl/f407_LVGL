cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME f407_LVGL)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
# 智能环境与安防终端使用的板级驱动。
# W25Q64驱动故意不列入这里，因此不会参与本项目的编译与运行。
set(USER_DSHANMCU_F407_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/DshanMCU-F407/driver_color_led_f407.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/DshanMCU-F407/driver_cst816d_f407.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/DshanMCU-F407/driver_dht11_f407.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/DshanMCU-F407/driver_ir_receiver_f407.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/DshanMCU-F407/driver_mpu6050_f407.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/DshanMCU-F407/driver_timer_f407.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/DshanMCU-F407/driver_st7789_f407.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/DshanMCU-F407/driver_ultrasonic_sr04_f407.c)

# First application version: DHT11 -> ControlTask -> LVGL UI.
set(BUSINESS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Business/Src/environment_service.c)

set(APPLICATION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/Src/app_runtime.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/Src/app_sensor_task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/Src/app_measure_task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/Src/app_input_task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/Src/app_control_task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/Src/app_indicator_task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/Src/app_ui_task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/UI/Src/ui_app.c)

set(LVGL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/lvgl)
file(GLOB LVGL_SOURCES CONFIGURE_DEPENDS
    ${LVGL_ROOT}/src/*.c
    ${LVGL_ROOT}/src/core/*.c
    ${LVGL_ROOT}/src/debugging/sysmon/*.c
    ${LVGL_ROOT}/src/display/*.c
    ${LVGL_ROOT}/src/draw/*.c
    ${LVGL_ROOT}/src/draw/convert/*.c
    ${LVGL_ROOT}/src/draw/sw/*.c
    ${LVGL_ROOT}/src/draw/sw/blend/*.c
    ${LVGL_ROOT}/src/font/*.c
    ${LVGL_ROOT}/src/font/fmt_txt/*.c
    ${LVGL_ROOT}/src/indev/*.c
    ${LVGL_ROOT}/src/layouts/*.c
    ${LVGL_ROOT}/src/layouts/flex/*.c
    ${LVGL_ROOT}/src/layouts/grid/*.c
    ${LVGL_ROOT}/src/libs/bin_decoder/*.c
    ${LVGL_ROOT}/src/misc/*.c
    ${LVGL_ROOT}/src/misc/cache/*.c
    ${LVGL_ROOT}/src/misc/cache/class/*.c
    ${LVGL_ROOT}/src/misc/cache/instance/*.c
    ${LVGL_ROOT}/src/osal/lv_os.c
    ${LVGL_ROOT}/src/osal/lv_freertos.c
    ${LVGL_ROOT}/src/stdlib/lv_mem.c
    ${LVGL_ROOT}/src/stdlib/builtin/*.c
    ${LVGL_ROOT}/src/themes/lv_theme.c
    ${LVGL_ROOT}/src/themes/default/*.c
    ${LVGL_ROOT}/src/tick/*.c
    ${LVGL_ROOT}/src/widgets/button/*.c
    ${LVGL_ROOT}/src/widgets/label/*.c)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Project-specific LVGL display/touch port.
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/f407_lvgl.c

    # 第一版实际使用的板级驱动。
    ${USER_DSHANMCU_F407_SOURCES}
    ${BUSINESS_SOURCES}
    ${APPLICATION_SOURCES}
    ${LVGL_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/DshanMCU-F407
    ${CMAKE_CURRENT_SOURCE_DIR}/Business/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/UI/Inc
    ${LVGL_ROOT}
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
