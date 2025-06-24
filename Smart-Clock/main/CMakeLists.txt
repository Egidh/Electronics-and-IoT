idf_component_register(SRCS "./src/wifi_manager.c" 
                            "./src/nvs_storage.c" 
                            "./src/clock.c" 
                            "./src/lvgl_UI.c"
                            "./assets/WifiFont.c"
                            "main.c"
                    INCLUDE_DIRS "." "./include" "./assets")