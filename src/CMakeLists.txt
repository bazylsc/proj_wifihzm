idf_component_register(SRCS 
    "hello_world_main.c" 
    "teleplot_udp.c" 
    "ssd1306_display.c"
    INCLUDE_DIRS 
    "../include")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
