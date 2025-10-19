aux_source_directory(. common_src)

add_library(common ${common_src})
target_link_libraries(common PUBLIC pico_stdlib hardware_pwm)