aux_source_directory(. canvas_src)

include_directories(../common)

add_library(canvas ${canvas_src})
target_link_libraries(canvas PUBLIC pico_stdlib hardware_pwm hardware_spi)