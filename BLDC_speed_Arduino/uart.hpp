#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "oledFunctions.hpp"

#define TXD 4
#define RXD 5
#define RTS (UART_PIN_NO_CHANGE)
#define CTS (UART_PIN_NO_CHANGE)

#define UartPortNum 2
#define baudRate 115200
#define stackSize 2048


const uint16_t BUF_SIZE = (1024);

// char OLEDFunctions::currentArr1[6];
// char OLEDFunctions::currentArr2[6];
// char OLEDFunctions::voltageArr1[6];
// char OLEDFunctions::powerArr1[6];
// char OLEDFunctions::powerArr2[6];

//Prototype
void extractData();
void printReceivedData();

//FreeRtos functions
static void receiveDataTask(void *);


void extractData(char *data)
{
  // Serial.println(data);

  memset(OLEDFunctions::currentArr1, 0, 6);
  memset(OLEDFunctions::currentArr2, 0, 6);
  memset(OLEDFunctions::voltageArr1, 0, 6);
  memset(OLEDFunctions::powerArr1, 0, 6);
  memset(OLEDFunctions::powerArr2, 0, 6);


  uint8_t index = 0;
  int j=0,i=0;

  while(data[i] != '\0')
  {
    if(data[i] != ',')
    {
      switch(index)
      {
        case 0:
          OLEDFunctions::currentArr1[j++] = data[i];
          break;

        case 1:
          OLEDFunctions::currentArr2[j++] = data[i];
          break;

        case 2:
          OLEDFunctions::voltageArr1[j++] = data[i];
          break;

        case 3:
          OLEDFunctions::powerArr1[j++] = data[i];
          break;

        case 4:
          OLEDFunctions::powerArr2[j++] = data[i];
          break;
      }
    }
    else
    {
      switch(index)
      {
        case 0:
        OLEDFunctions::currentArr1[j] = '\0';
        break;

        case 1:
        OLEDFunctions::currentArr2[j] = '\0';
        break;

        case 2:
        OLEDFunctions::voltageArr1[j] = '\0';
        break;

        case 3:
        OLEDFunctions::powerArr1[j] = '\0';
        break;

        case 4:
        OLEDFunctions::powerArr1[j] = '\0';
        break;
      }
      index++;
      j=0;
    }
    i++;
  }
}

void uartInit(const BaseType_t app_cpu1)
{
  /* Configure parameters of an UART driver,
    * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = baudRate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

  ESP_ERROR_CHECK(uart_driver_install(UartPortNum, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(UartPortNum, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UartPortNum, TXD, RXD, RTS, CTS));

  xTaskCreatePinnedToCore(&receiveDataTask, "uart_echo_task", stackSize, NULL, 10, NULL, app_cpu1);
}

static void receiveDataTask(void *arg)
{
    // Configure a temporary buffer for the incoming data
    char data[1024];

    while (1)
    {
      // Read data from the UART
      int len = uart_read_bytes(UartPortNum, data, (BUF_SIZE - 1), 500 / portTICK_PERIOD_MS);
      // Write data back to the UART
      // uart_write_bytes(UartPortNum, (const char *) data, len);
      if (len) 
      {
          data[len] = '\0';
          // Serial.println(data);
          // ESP_LOGI("UART TEST", "Recv str: %s", (char *) data);
          extractData(data);
          // printReceivedData();
          OLEDFunctions::printUartData();
      }
      // vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}


void printReceivedData()
{
  Serial.println(OLEDFunctions::currentArr1);
  Serial.println(OLEDFunctions::currentArr2);
  Serial.println(OLEDFunctions::voltageArr1);
  Serial.println(OLEDFunctions::powerArr1);
  Serial.println(OLEDFunctions::powerArr2);
}
