/* LEDEffect-Rainbow-Defy - Rainbow LED effects for keyscanner sides.
 * Copyright (C) 2017-2018  Keyboard.io, Inc.
 * Copyright (C) 2023, 2024  DygmaLabs, S. L.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "LEDEffect.h"
#include "LedModeSerializable-Rainbow.h"
#include "LedModeSerializable-RainbowWave.h"
#include "LedModeCommunication.h"


class LEDRainbowEffectDefy : public LEDEffect,
                             public LedModeCommunication {
  public:
    LEDRainbowEffectDefy(void) : LEDEffect( LED_EFFECT_TYPE_RAINBOW )
    {
        led_mode.base_settings.brightness = 255;
        led_mode.base_settings.delay_ms = 40;
    }

    LedModeSerializable_Rainbow &led_mode = ledModeSerializableRainbow;

    void activate() final;

    void brightness(uint8_t);

    uint8_t brightness()
    {
        return led_mode.base_settings.brightness;
    }

    void update_delay(uint8_t);

    uint8_t update_delay(void)
    {
        return led_mode.base_settings.delay_ms;
    }
};


class LEDRainbowWaveEffectDefy : public LEDEffect,
                                 public LedModeCommunication {
  public:
    LEDRainbowWaveEffectDefy(void) : LEDEffect( LED_EFFECT_TYPE_RAINBOW_WAVE )
    {
        led_mode.base_settings.brightness = 255;
        led_mode.base_settings.delay_ms = 40;
    }

    LedModeSerializable_RainbowWave &led_mode = ledModeSerializableRainbowWave;

    void activate() final;

    void brightness(uint8_t);

    uint8_t brightness()
    {
        return led_mode.base_settings.brightness;
    }

    void update_delay(uint8_t);

    uint8_t update_delay(void)
    {
        return led_mode.base_settings.delay_ms;
    }
};

extern class LEDRainbowEffectDefy LEDRainbowEffectDefy;
extern class LEDRainbowWaveEffectDefy LEDRainbowWaveEffectDefy;
