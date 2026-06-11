#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * The RoboMaster Type-C board buzzer is wired to PD14 / TIM4_CH3.
 * Enable TIM4 PWM CH3 in CubeMX, then this module will use htim4 automatically.
 */
bool Buzzer_PlayHaruhiKage(uint8_t volume_percent);
