#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

// Определяем пины для светодиодов и кнопки
#define LED1_PIN GPIO0
#define LED2_PIN GPIO1
#define BUTTON_PIN GPIO6

void vApplicationStackOverflowHook(TaskHandle_t *pxTask, char *pcTaskName) {
    (void)pxTask;
    (void)pcTaskName;
    for (;;);
}

void gpio_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO0 | GPIO1);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
    // Настраиваем GPIOB6 как вход с подтяжкой к VCC (кнопка)
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, BUTTON_PIN);
    gpio_set(GPIOB, BUTTON_PIN); // Включаем подтяжку к VCC
}

// Функция инициализации таймера для ШИМ
void pwm_setup(void) {
    rcc_periph_clock_enable(RCC_TIM3);

    // Настраиваем таймер 3, канал 3 и 4 для ШИМ
    timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_prescaler(TIM3, 72 - 1); // Частота таймера = 1 МГц
    timer_set_period(TIM3, 1000);  // Период ШИМ = 1 мс (частота 1 КГц)

    // Настраиваем каналы 3 и 4 как PWM1
    timer_set_oc_mode(TIM3, TIM_OC3, TIM_OCM_PWM1);
    timer_set_oc_mode(TIM3, TIM_OC4, TIM_OCM_PWM1);

    // Устанавливаем заполнение 50%
    timer_set_oc_value(TIM3, TIM_OC3, 500);
    timer_set_oc_value(TIM3, TIM_OC4, 500);

    // Выключаем каналы 3 и 4
    timer_disable_oc_output(TIM3, TIM_OC3);
    timer_disable_oc_output(TIM3, TIM_OC4);

    // Включаем таймер
    timer_enable_preload(TIM3);
    timer_enable_counter(TIM3);
}

static void button_handler(void *args __attribute((unused))) {
    while(1){
        static int state = 0;
        static int button_pressed = 0;

        // Проверяем состояние кнопки
        if (gpio_get(GPIOB, BUTTON_PIN) == 0) { // Кнопка нажата (логический "0")
            if (!button_pressed) { // Если кнопка только что нажата
                button_pressed = 1;
                gpio_clear(GPIOC, GPIO13);
                // Меняем состояние
                state = (state + 1) % 3;

                // Управляем ШИМ в зависимости от состояния
                switch (state) {
                    case 0:{
                        timer_disable_oc_output(TIM3, TIM_OC3);
                        timer_disable_oc_output(TIM3, TIM_OC4);
                        break;
                    }
                    case 1:{
                        timer_enable_oc_output(TIM3, TIM_OC3);
                        timer_disable_oc_output(TIM3, TIM_OC4);
                        break;
                    }
                    case 2:{
                        timer_disable_oc_output(TIM3, TIM_OC3);
                        timer_enable_oc_output(TIM3, TIM_OC4);
                        break;
                    }
                }
                vTaskDelay(pdMS_TO_TICKS(250));
            }
        } else {
            button_pressed = 0; // Кнопка отпущена
            gpio_set(GPIOC, GPIO13);
        }
    }
}

int main(void) {
    // Настройка тактирования
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    // Инициализация периферии
    gpio_setup();
    pwm_setup();

    xTaskCreate(button_handler,"button_handler",100,NULL, configMAX_PRIORITIES - 1,NULL);
    vTaskStartScheduler();
	for (;;);
    return 0;
}