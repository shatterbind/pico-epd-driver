add_library(common INTERFACE)
target_include_directories(common INTERFACE .)
target_link_libraries(common INTERFACE pico_stdlib hardware_pwm hardware_spi)