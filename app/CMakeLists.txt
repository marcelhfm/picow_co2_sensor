cmake_minimum_required(VERSION 3.28)

add_compile_definitions(APP_NAME="${APP_NAME}")
add_compile_definitions(APP_VERSION="${APP_VERSION}")
add_compile_definitions(BUILD_NUM=${BUILD_NUM})

add_executable(${APP_NAME}
  main.c
  ssd1306/ssd1306.c
  ssd1306/framebuffer.c
  ssd1306/display.c
  network/wifi.c
  tasks/update_display_task.c
  tasks/read_data_task.c
  i2c/i2c.c
  scd40/scd40.c
  )


target_compile_definitions(${APP_NAME} PRIVATE
    WIFI_SSID="$ENV{WIFI_SSID}"
    WIFI_PASSWORD="$ENV{WIFI_PASSWORD}"
)

target_link_libraries(${APP_NAME} LINK_PUBLIC
  pico_stdlib
  hardware_i2c
  hardware_adc
  FreeRTOS
  pico_cyw43_arch_lwip_threadsafe_background
)

# Uncomment if you want to flash your board via picotool.
pico_add_extra_outputs(co2)
pico_enable_stdio_usb(co2 1)
pico_enable_stdio_uart(co2 0)


