#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <FreeRTOS.h>
#include <task.h>
#include "enc28j60_spi.h"
#include "enc28j60.h"  // Библиотека от IOsetting

#define LED_PORT GPIOC
#define LED_PIN  GPIO13

static void net_task(void *arg) {
    (void)arg;

    uint8_t buffer[ENC28J60_MAXFRAME];
    uint16_t len;

    while (1) {
        len = enc28j60_packetReceive(ENC28J60_MAXFRAME, buffer);
        if (len > 0) {
            if (strstr((char*)buffer, "Led on")) {
                gpio_clear(LED_PORT, LED_PIN);
            } else if (strstr((char*)buffer, "Led off")) {
                gpio_set(LED_PORT, LED_PIN);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main(void) {
    // Тактирование
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_GPIOC);

    // Настройка LED (PC13)
    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
    gpio_set(LED_PORT, LED_PIN);

    // Инициализация SPI и ENC28J60
    enc28j60_spi_init();
    enc28j60_init((uint8_t[]){0x00, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E}); // MAC-адрес

    // Создание задачи обработки сетевых пакетов
    xTaskCreate(net_task, "NetTask", 256, NULL, 1, NULL);

    // Запуск FreeRTOS
    vTaskStartScheduler();

    while (1);
}
