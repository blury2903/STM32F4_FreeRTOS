/*
 * ledeffect.c
 *
 *  Created on: Jun 20, 2024
 *      Author: blury
 */

#include "main.h"


void led_effect_stop(void)
{
	for(int i = 0; i < 4; ++i){
		xTimerStop(handle_led_timer[i], portMAX_DELAY);
	}

}


void led_effect(uint32_t n)
{
	led_effect_stop();
	xTimerStart(handle_led_timer[n - 1], portMAX_DELAY);

}

void turn_off_all_leds(void)
{
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
}

void turn_on_all_leds(void)
{
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET);

}

void turn_on_even_leds(void)
{
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET);
}

void turn_on_odd_leds(void)
{
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);

}

void led_control(int value, int id)
{
	if(id == 3){
		for(int i = 0; i < 4; ++i){
			HAL_GPIO_WritePin(GPIOD, (LD4_Pin << i), ((value >> i) & 0x1));
		}
	}

	if(id == 4){
		for(int i = 3; i >= 0; --i){
			HAL_GPIO_WritePin(GPIOD, (LD4_Pin << i), ((value >> i) & 0x1));
		}
	}
}

void LED_effect1(void)
{
	static int flag = 1;
	(flag ^= 1) ? turn_off_all_leds() : turn_on_all_leds();
}

void LED_effect2(void)
{
	static int flag = 1;
	(flag ^= 1) ? turn_on_even_leds() : turn_on_odd_leds();
}

void LED_effect3(void)
{
	static int i = 0;
	led_control(0x1 << (i++ % 4), 3);
}

void LED_effect4(void)
{
	static int i = 0;
	led_control(0x8 >> (i++ % 4), 4);		//1000
}
