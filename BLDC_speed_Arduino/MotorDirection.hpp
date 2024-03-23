#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class MotorDirection {
  private:
  gpio_num_t relayPin1;
  gpio_num_t relayPin2;

  public:

  MotorDirection(gpio_num_t relayPin1, gpio_num_t relayPin2) :
  relayPin1(relayPin1),
  relayPin2(relayPin2) 
  {}

  void begin() {
    if(this->relayPin1 != GPIO_NUM_NC && this->relayPin2 != GPIO_NUM_NC) {
      gpio_config_t gpioOutputConfigure = {
        .pin_bit_mask = (1ULL << this->relayPin1) | (1ULL << this->relayPin2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
      };
      ESP_ERROR_CHECK(gpio_config(&gpioOutputConfigure));
    } else {
        ESP_LOGI("Relay Setup", "Relay pin set to GPIO_NUM_NC");
    }
  }

  void front() {
    gpio_set_level(relayPin1, 1);
    gpio_set_level(relayPin2, 1);

    ESP_LOGI("front", "Relays are HIGH");
  }

  void back() {
    gpio_set_level(relayPin1, 0);
    gpio_set_level(relayPin2, 0);
    
    ESP_LOGI("front", "Relays are LOW");
  }
};