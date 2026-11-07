add_library(epd_2_13 epd_2_13.c)
target_include_directories(epd_2_13 PUBLIC .)
target_link_libraries(epd_2_13 PUBLIC common hardware_spi)