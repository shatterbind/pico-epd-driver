add_library(canvas canvas.c)
target_include_directories(canvas PUBLIC .)
target_link_libraries(canvas PUBLIC common pico_stdlib)