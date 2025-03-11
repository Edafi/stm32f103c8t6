/* Simple LED task demo, using timed delays:
 *
 * The LED on PC13 is toggled in task1.
 */
#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

extern void vApplicationStackOverflowHook(
	xTaskHandle *pxTask,
	signed portCHAR *pcTaskName);

void vApplicationStackOverflowHook(
  xTaskHandle *pxTask __attribute((unused)),
  signed portCHAR *pcTaskName __attribute((unused))
) {
	for(;;);	// Loop forever here..
}

static void task1(void *args __attribute((unused))) {
	for(;;){
		gpio_toggle(GPIOC, GPIO13);
		vTaskDelay(pdMS_TO_TICKS(1000));
		//gpio_toggle(GPIOC, GPIO13);
		
		gpio_toggle(GPIOB,GPIO0);
		vTaskDelay(pdMS_TO_TICKS(1000));
		//gpio_toggle(GPIOB,GPIO0);

		gpio_toggle(GPIOB,GPIO1);
		vTaskDelay(pdMS_TO_TICKS(1000));
		//gpio_toggle(GPIOB,GPIO1);

		//gpio_set(GPIOB,GPIO1);
		//gpio_set(GPIOB,GPIO0);
		//gpio_set(GPIOC,GPIO13);
		vTaskDelay(pdMS_TO_TICKS(1000));
		
		gpio_toggle(GPIOB,GPIO1);
		vTaskDelay(pdMS_TO_TICKS(1000));
		//gpio_toggle(GPIOB,GPIO1);

		gpio_toggle(GPIOB,GPIO0);
		vTaskDelay(pdMS_TO_TICKS(1000));
		//gpio_toggle(GPIOB,GPIO0);

		gpio_toggle(GPIOC,GPIO13);
		vTaskDelay(pdMS_TO_TICKS(1000));
		//gpio_toggle(GPIOC, GPIO13);		
	}
}

int main(void) {
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
		gpio_set(GPIOC, GPIO13);
		gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO0 | GPIO1);
		//gpio_set(GPIOB, GPIO0);
		//gpio_set(GPIOB, GPIO1);

	xTaskCreate(task1,"LEDS",100,NULL, configMAX_PRIORITIES - 1,NULL);
	
	vTaskStartScheduler();
	for (;;);
	return 0;
}

// End
