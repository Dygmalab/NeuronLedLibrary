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

#include "LEDEffect-SolidColor-Defy.h"

void LEDSolidColorDefy::activate(void) {
    led_mode.r_ = r_;
    led_mode.g_ = g_;
    led_mode.b_ = b_;
    led_mode.w_ = w_;

    sendLedMode(led_mode);
}

class LEDSolidColorDefy solidRedDefy(LEDEffect::LED_EFFECT_TYPE_SOLID_RED, 255, 0, 0, 0);
class LEDSolidColorDefy solidGreenDefy(LEDEffect::LED_EFFECT_TYPE_SOLID_GREEN, 0, 255, 0, 0);
class LEDSolidColorDefy solidBlueDefy(LEDEffect::LED_EFFECT_TYPE_SOLID_BLUE, 0, 0, 255, 0);
class LEDSolidColorDefy solidWhiteDefy(LEDEffect::LED_EFFECT_TYPE_SOLID_WHITE, 0, 0, 0, 255);
class LEDSolidColorDefy solidBlackDefy(LEDEffect::LED_EFFECT_TYPE_SOLID_BLACK, 0, 0, 0, 0);
