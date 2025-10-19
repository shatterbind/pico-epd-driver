aux_source_directory(. epd_2_13_src)
add_library(epd_2_13 ${epd_2_13_src})

include_directories(../common)

target_link_libraries(epd_2_13 PUBLIC common canvas hardware_spi)