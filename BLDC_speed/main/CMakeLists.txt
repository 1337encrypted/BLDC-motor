idf_component_register(
    SRCS
        "GLOBALS.hpp"
        "BLDC_speed.cpp"
        "BLDCPulseCalculator/BLDCPulseCalculator.cpp"
        "Uart/Uart.cpp"
        "U8g2lib.cpp"
        "U8x8lib.cpp"
        "OLEDFunction.hpp"
    INCLUDE_DIRS 
        "."
        "BLDCPulseCalculator"
        "Uart"

    REQUIRES
        "driver" 
        "freertos"
        "u8g2"
        "esp_timer"
)

