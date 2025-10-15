/* LEDEffect-SolidColor-Defy - Solid color LED effects for keyscanner sides.
 * Copyright (C) 2017  Keyboard.io, Inc.
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
#include "LedModeSerializable-SolidColor.h"
#include "LedModeCommunication.h"

class LEDSolidColorDefy : public LEDEffect,
                          public LedModeCommunication {
public:
    LEDSolidColorDefy( led_effect_type_t type, uint8_t r, uint8_t g, uint8_t b, uint8_t w )
    : LEDEffect(type), r_(r), g_(g), b_(b), w_(w) {
        led_mode.base_settings.delay_ms = 100;
    }
    LedModeSerializable_SolidColor &led_mode = ledModeSerializableSolidColor;

    void activate() final;

  private:
    uint8_t r_, g_, b_, w_;
};

extern class LEDSolidColorDefy solidRedDefy;
extern class LEDSolidColorDefy solidGreenDefy;
extern class LEDSolidColorDefy solidBlueDefy;
extern class LEDSolidColorDefy solidWhiteDefy;
extern class LEDSolidColorDefy solidBlackDefy;
