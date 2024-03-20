idf_component_register(
    SRCS
        "BLDC_speed.cpp"
        "BLDCPulseCalculator.cpp"
        "PwmGenerator.cpp"
        "Uart.cpp"
        # "BLDCPulseCalculator.hpp"
        # "PwmGenerator.hpp"
        # "Uart.hpp"
        # "OLEDFunction.hpp"
        # "U8g2lib.cpp"
        # "U8x8lib.cpp"
    INCLUDE_DIRS 
        "."
        # "/Users/shady/Documents/Arduino/libraries/U8g2/"
    REQUIRES
        "driver"
)
