#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#define UART_PORT GPIOA
#define UART_TX_PIN GPIO9
#define UART_RCC RCC_USART1

#define ENCODER_PORT GPIOB
#define ENCODER_CLK GPIO8
#define ENCODER_DT GPIO7
#define ENCODER_SW GPIO6

#define PASSWORD_WORD "secret"
#define PASSWORD_NUMBER "1234"

// Прототипы функций
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
void uart_setup(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_printf(const char *fmt, ...);
char uart_recv_char(void);
void encoder_setup(void);
int encoder_read(void);
int button_read(void);
void led_setup(void);
void led_blink(int times);
void vTaskUSART(void *pvParameters);

// Обработчик переполнения стека
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    for (;;);
}

// Настройка UART
void uart_setup(void) {
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, 
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

    usart_set_baudrate(USART1, 38400);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

    usart_enable(USART1);
}

// Отправка символа по UART
void uart_putc(char c) {
    usart_send_blocking(USART1, c);
}

// Отправка строки по UART
void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

// Форматированный вывод (аналог printf)
void uart_printf(const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    uart_puts(buf);
}

char uart_recv_char(void) {
    while (!(USART_SR(USART1) & USART_SR_RXNE)); // Ждем, пока данные не будут получены
    return usart_recv(USART1);
}

// Настройка энкодера и кнопки
void encoder_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOB);

    // Настройка пинов энкодера (PB8 и PB7)
    gpio_set_mode(ENCODER_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, ENCODER_CLK | ENCODER_DT);
    gpio_clear(ENCODER_PORT, ENCODER_CLK | ENCODER_DT); // Подтяжка к питанию

    // Настройка пина кнопки (PB7)
    gpio_set_mode(ENCODER_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, ENCODER_SW);
    gpio_set(ENCODER_PORT, ENCODER_SW); // Подтяжка к питанию
}

// Чтение состояния энкодера
int encoder_read(void) {
    static int last_state = 0;
    int clk_state = gpio_get(ENCODER_PORT, ENCODER_CLK); // Чтение CLK
    int dt_state = gpio_get(ENCODER_PORT, ENCODER_DT);   // Чтение DT
    int state = (clk_state ? 1 : 0) | ((dt_state ? 1 : 0) << 1); // Формируем состояние
    int delta = 0.0;

    //  CLK==0 and DT==0 => state=0
    //  CLK==1 and DT==0 => state=1
    //  CLK==0 and DT==1 => state=2
    //  CLK==1 and DT==1 => state=3

    // clockwise 0->1->3->2->0
    // counterclockwise 0->2->3->1->0

    if (state != last_state) {
        if (last_state == 1 && state == 3) delta = 1;
        if (last_state == 3 && state == 2) delta = 1;
        if (last_state == 2 && state == 0) delta = 1;
        if (last_state == 0 && state == 1) delta = 1;

        if (last_state == 1 && state == 0) delta = -1;
        if (last_state == 0 && state == 2) delta = -1;
        if (last_state == 2 && state == 3) delta = -1;
        if (last_state == 3 && state == 1) delta = -1;

        last_state = state;
    }
    return delta;
}

// Чтение состояния кнопки
int button_read(void) {
    int state = !gpio_get(ENCODER_PORT, ENCODER_SW);
    return state;
}

// Настройка светодиода
void led_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

// Мигание светодиодом
void led_blink(int times) {
    for (int i = 0; i < times; i++) {
        gpio_toggle(GPIOC, GPIO13);
        vTaskDelay(pdMS_TO_TICKS(50));
        gpio_toggle(GPIOC, GPIO13);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Задача для работы с USART
void vTaskUSART(void *pvParameters) {
    uart_printf("vTaskUSART started\r\n"); // Отладочный вывод
    char buffer[32];
    int number = 0;
    int digit = 0;
    char password[5];

    uart_printf("Enter code word: ");
    while (1) {
        // Прием символа
        char c = uart_recv_char();

        // Если нажат Enter (символ '\r' или '\n')
        if (c == '\r' || c == '\n') {
            buffer[digit] = '\0'; // Завершаем строку
            //uart_printf("\r\nYou entered: %s\r\n", buffer); // Отладочный вывод

            // Проверяем, совпадает ли введенное слово с паролем
            if (strcmp(buffer, PASSWORD_WORD) == 0) {
                uart_printf("Password correct! Enter 4-digit number: \r\n"); // Отладочный вывод
                number = 0;
                digit = 0;
                //password = 0;
                memset(password, 0, sizeof(password));
                while (digit < 4) {
                    int delta = encoder_read();
                    if (delta) {
                        number += delta;
                        if (number < 0) number = 19;
                        if (number > 19) number = 0;
                        sprintf(buffer, "\r%d", number/2);
                        uart_printf(buffer);
                    }
                    if (button_read()) {
                        vTaskDelay(pdMS_TO_TICKS(50));
                        if(button_read()){
                            uart_printf(" - Button pressed\r\n");
                            //password += (1000 / pow(10, digit)) * number/2;
                            password[digit++] = '0' + (char) number/2;
                            while (button_read()); // Ждем отпускания кнопки
                        }
                    }
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
                if (strcmp(password, PASSWORD_NUMBER) == 0) {
                    led_blink(5);
                    uart_printf("\r\n%s", password);
                    uart_printf("\r\nAccess granted!");
                } else {
                    led_blink(1);
                    uart_printf("\r\n%s", password);
                    uart_printf("\r\nAccess denied!");
                }
            } else {
                uart_printf("\r\nInvalid code word!"); // Отладочный вывод
            }
            uart_printf("\r\nEnter code word: ");
            digit = 0; // Сброс счетчика символов
        } else if (digit < sizeof(buffer) - 1) {
            buffer[digit++] = c; // Сохраняем символ в буфер
        }
    }
}

int main(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    uart_setup();
    encoder_setup();
    led_setup();

    xTaskCreate(vTaskUSART, "USART", 256, NULL, 2, NULL);

    vTaskStartScheduler();

    while (1);
    return 0;
}