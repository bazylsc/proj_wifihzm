idf_component_register(SRCS "hello_world_main.c" "teleplot_udp.c" "mqtt_task.c"
                    INCLUDE_DIRS "../include")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
