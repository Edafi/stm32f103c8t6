#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>  // Для отладочного вывода

#define LED_PIN GPIO13
#define LED_PORT GPIOC
#define LED_RED GPIO0
#define RED_PORT GPIOB
#define LED_PIN GPIO13
#define LED_PORT GPIOC

#define LDR_PIN GPIO1
#define LDR_PORT GPIOA
#define ADC_CHANNEL 1
#define LIGHT_THRESHOLD 500  // Порог освещённости
#define TIMER_DELAY 100      // Время задержки в миллисекундах

// Прототипы функций
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
void vTaskReadLight(void *pvParameters);
void vTaskControlLED(void *pvParameters);

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;adc_read_regular;
    (void)pcTaskName;
    for (;;);
}

#include <libopencm3/stm32/usart.h>

#define UART_PORT GPIOA
#define UART_TX_PIN GPIO9
#define UART_RCC RCC_USART1

void uart_setup(void) {
    // Включаем тактирование USART1 и GPIOA
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_GPIOA);

    // Настраиваем TX (PA9) на альтернативный push-pull
    gpio_set_mode(UART_PORT, GPIO_MODE_OUTPUT_50_MHZ, 
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

    // Настраиваем USART1: 38400 бод, 8 бит, 1 стоп-бит, без четности
    usart_set_baudrate(USART1, 38400);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

    // Включаем USART1
    usart_enable(USART1);
}

// Функция для отправки символа по UART
void uart_putc(char c) {
    usart_send_blocking(USART1, c);
}

// Функция для отправки строки
void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

// Форматированный вывод (как printf)
#include <stdarg.h>
void uart_printf(const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    uart_puts(buf);
}

static void adc_setup(void) {
    rcc_periph_clock_enable(RCC_ADC1);
    rcc_periph_clock_enable(RCC_GPIOA);

    // Настроим GPIOA1 как аналоговый вход
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, LDR_PIN);
    //uart_puts("GPIOA1 set as analog input\n\r");
    rcc_peripheral_enable_clock(&RCC_APB2ENR,RCC_APB2ENR_ADC1EN);
	adc_power_off(ADC1);
	rcc_peripheral_reset(&RCC_APB2RSTR,RCC_APB2RSTR_ADC1RST);
	rcc_peripheral_clear_reset(&RCC_APB2RSTR,RCC_APB2RSTR_ADC1RST);
	rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV6);	// Set. 12MHz, Max. 14MHz
	adc_set_dual_mode(ADC_CR1_DUALMOD_IND);		// Independent mode
	adc_disable_scan_mode(ADC1);
	adc_set_right_aligned(ADC1);
	adc_set_single_conversion_mode(ADC1);
	adc_set_sample_time(ADC1,ADC_CHANNEL_TEMP,ADC_SMPR_SMP_239DOT5CYC);
	adc_set_sample_time(ADC1,ADC_CHANNEL_VREF,ADC_SMPR_SMP_239DOT5CYC);
	adc_enable_temperature_sensor();
	adc_power_on(ADC1);
	adc_reset_calibration(ADC1);
	adc_calibrate_async(ADC1);
	while ( adc_is_calibrating(ADC1) );
}

static uint16_t read_adc() {
    char channel = 1; 
	adc_set_sample_time(ADC1,ADC_CHANNEL ,ADC_SMPR_SMP_239DOT5CYC);
	adc_set_regular_sequence(ADC1, 1, &channel);
	adc_start_conversion_direct(ADC1);
	while ( !adc_eoc(ADC1) )
		__asm__("nop");
	return adc_read_regular(ADC1);
}


static void led_setup(void) {
    // Включение тактирования GPIOC
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
}

void vTaskReadLight(void *pvParameters) {
    (void)pvParameters;
    uint16_t light_level;
    TaskHandle_t xTaskToNotify = (TaskHandle_t)pvParameters;

    while (1) {
        light_level = read_adc();
        uart_printf("ADC value: %u\n\r", light_level);
        if (light_level > LIGHT_THRESHOLD) {
            //xTaskNotifyGive(xTaskToNotify);
            gpio_set(LED_PORT, LED_PIN);  // Включаем светодиод
            vTaskDelay(pdMS_TO_TICKS(TIMER_DELAY));  // Ждём заданное время
        }
        else
            gpio_clear(LED_PORT, LED_PIN);  // Выключаем светодиод
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vTaskControlLED(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        gpio_set(LED_PORT, LED_PIN);  // Включаем светодиод
        vTaskDelay(pdMS_TO_TICKS(TIMER_DELAY));  // Ждём заданное время
        gpio_clear(LED_PORT, LED_PIN);  // Выключаем светодиод
    }
}

int main(void) {
    // Настройка тактирования
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    adc_setup();
    led_setup();
    uart_setup();
    uart_puts("UART ready!\n\r");
    uart_printf("ADC setup done\n\r");
    TaskHandle_t xTaskControlLEDHandle = NULL;

    // Создаём задачи
    //xTaskCreate(vTaskControlLED, "LED_TASK", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskControlLEDHandle);
    xTaskCreate(vTaskReadLight, "LIGHT_TASK", configMINIMAL_STACK_SIZE, NULL/*(void *)xTaskControlLEDHandle*/, 1, NULL);

    // Запуск планировщика FreeRTOS
    vTaskStartScheduler();

    // Программа никогда не должна сюда попасть
    while (1);
}