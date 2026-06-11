#include "buzzer.h"

#include <stddef.h>

#include "stm32f4xx_hal.h"
#include "tim.h"

typedef struct
{
    uint16_t frequency_hz;
    uint16_t duration_ms;
} buzzer_note_t;

typedef struct
{
    TIM_HandleTypeDef *timer;
    uint32_t channel;
    uint32_t timer_clock_hz;
    uint32_t counter_clock_hz;
    uint16_t max_duty_permille;
    uint16_t min_frequency_hz;
    uint16_t max_frequency_hz;
} buzzer_config_t;

static const buzzer_config_t s_buzzer_config = {
    .timer = &htim4,
    .channel = TIM_CHANNEL_3,
    .timer_clock_hz = 84000000UL,
    .counter_clock_hz = 1000000UL,
    .max_duty_permille = 500U,
    .min_frequency_hz = 20U,
    .max_frequency_hz = 20000U,
};

static const buzzer_note_t s_haruhi_kage_notes[] = {
    {1245U, 1238U / 2U},
    {1109U, 619U / 2U},
    {988U, 1238U / 2U},
    {1109U, 619U / 2U},
    {1245U, 928U / 2U},
    {1319U, 310U / 2U},
    {1245U, 619U / 2U},
    {1109U, 1857U / 2U},
};

static uint32_t clamp_u32(uint32_t value, uint32_t min_value, uint32_t max_value)
{
    if (value < min_value)
    {
        return min_value;
    }

    if (value > max_value)
    {
        return max_value;
    }

    return value;
}

static void buzzer_mute(void)
{
    __HAL_TIM_SET_COMPARE(s_buzzer_config.timer, s_buzzer_config.channel, 0U);
}

static void buzzer_set_tone(uint32_t frequency_hz, uint8_t volume_percent)
{
    uint32_t prescaler;
    uint32_t period;
    uint32_t compare;

    if ((frequency_hz == 0U) || (volume_percent == 0U))
    {
        buzzer_mute();
        return;
    }

    frequency_hz = clamp_u32(frequency_hz,
                             s_buzzer_config.min_frequency_hz,
                             s_buzzer_config.max_frequency_hz);
    volume_percent = (uint8_t)clamp_u32(volume_percent, 0U, 100U);

    prescaler = (s_buzzer_config.timer_clock_hz / s_buzzer_config.counter_clock_hz) - 1U;
    period = (s_buzzer_config.counter_clock_hz / frequency_hz) - 1U;
    compare = ((period + 1U) * volume_percent * s_buzzer_config.max_duty_permille) / 100000U;

    __HAL_TIM_SET_PRESCALER(s_buzzer_config.timer, prescaler);
    __HAL_TIM_SET_AUTORELOAD(s_buzzer_config.timer, period);
    __HAL_TIM_SET_COUNTER(s_buzzer_config.timer, 0U);
    __HAL_TIM_SET_COMPARE(s_buzzer_config.timer, s_buzzer_config.channel, compare);
    s_buzzer_config.timer->Instance->EGR = TIM_EGR_UG;
}

bool Buzzer_PlayHaruhiKage(uint8_t volume_percent)
{
    if (HAL_TIM_PWM_Start(s_buzzer_config.timer, s_buzzer_config.channel) != HAL_OK)
    {
        return false;
    }

    volume_percent = (uint8_t)clamp_u32(volume_percent, 0U, 100U);

    for (size_t i = 0U; i < (sizeof(s_haruhi_kage_notes) / sizeof(s_haruhi_kage_notes[0])); ++i)
    {
        buzzer_set_tone(s_haruhi_kage_notes[i].frequency_hz, volume_percent);
        HAL_Delay(s_haruhi_kage_notes[i].duration_ms);
    }

    buzzer_mute();
    return true;
}
