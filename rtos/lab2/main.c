#include "FreeRTOS.h"
#include "task.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>

// LEDS PINS
#define LED_GREEN_GPIO_PORT GPIOC
#define LED_GREEN_GPIO_PIN GPIO13

#define LED_RED_GPIO_PORT GPIOB
#define LED_RED_GPIO_PIN GPIO0

#define LED_YELLOW_GPIO_PORT GPIOB
#define LED_BLUE_GPIO_PIN GPIO1

// LEDS FREQ
#define LED_RED_FREQ 2.0f
#define LED_YELLOW_FREQ 1.5f
#define LED_GREEN_FREQ 0.5f

#define TIMER_FREQ 1000

// FreeRtos something
void vApplicationStackOverflowHook(TaskHandle_t *pxTask, char *pcTaskName) {
    (void)pxTask;
    (void)pcTaskName;
    for (;;);
}

void setup_timer(uint32_t timer, float freq) {
    
    unsigned long long prescaler = rcc_apb1_frequency / TIMER_FREQ - 1;
    unsigned long long period = TIMER_FREQ / freq - 1;

    timer_set_prescaler(timer, prescaler);
    timer_set_period(timer, period);
    timer_enable_irq(timer, TIM_DIER_UIE);		//enable interruption
    timer_enable_counter(timer);
}

// interrupt handler RED
void tim2_isr(void) {
    if (timer_get_flag(TIM2, TIM_SR_UIF)) {			//if interrupt flag is on (timer overflow)
        timer_clear_flag(TIM2, TIM_SR_UIF);  		//clear interrupt flag
        gpio_toggle(LED_RED_GPIO_PORT, LED_RED_GPIO_PIN);  // led on/off
    }
}

// interrupt handler YELLOW
void tim3_isr(void) {
    if (timer_get_flag(TIM3, TIM_SR_UIF)) {
        timer_clear_flag(TIM3, TIM_SR_UIF);  
        gpio_toggle(LED_YELLOW_GPIO_PORT, LED_BLUE_GPIO_PIN);
    }
}

// interrupt handler GREEN
void tim4_isr(void) {
    if (timer_get_flag(TIM4, TIM_SR_UIF)) {
        timer_clear_flag(TIM4, TIM_SR_UIF);
        gpio_toggle(LED_GREEN_GPIO_PORT, LED_GREEN_GPIO_PIN);
    }
}

int main(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_TIM3);
    rcc_periph_clock_enable(RCC_TIM4);

    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_GREEN_GPIO_PIN);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_RED_GPIO_PIN);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_BLUE_GPIO_PIN);

    setup_timer(TIM2, LED_RED_FREQ);  
    setup_timer(TIM3, LED_YELLOW_FREQ);
    setup_timer(TIM4, LED_GREEN_FREQ); 

    // Cortex-m3 interruption
    nvic_enable_irq(NVIC_TIM2_IRQ);
    nvic_enable_irq(NVIC_TIM3_IRQ);
    nvic_enable_irq(NVIC_TIM4_IRQ);

    // FreeRTOS
    vTaskStartScheduler();
    for (;;);
    return 0;
}