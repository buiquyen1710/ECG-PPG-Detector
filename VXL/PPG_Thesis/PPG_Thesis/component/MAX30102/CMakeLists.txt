set(app_src max30102.c)
set(pre_req i2cdev log esp_timer)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pre_req})