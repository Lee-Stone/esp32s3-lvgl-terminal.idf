file(GLOB_RECURSE UI_SOURCES "ui/*.c")

idf_component_register(SRCS ${UI_SOURCES}
                    INCLUDE_DIRS "include" "ui"
                    REQUIRES lvgl__lvgl task_set)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-value)