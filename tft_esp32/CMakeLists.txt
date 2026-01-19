idf_component_register(SRCS "ili9488.c" "receiver_logic.c" "main.c" "jpg_decode.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES espressif__esp_jpeg
                    )
