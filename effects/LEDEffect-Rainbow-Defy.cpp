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

#include "LEDEffect-Rainbow-Defy.h"
#include "Arduino.h"

void LEDRainbowEffectDefy::activate(void) {
    sendLedMode(led_mode);
}

void LEDRainbowEffectDefy::brightness(uint8_t brightness) {
  led_mode.base_settings.brightness = brightness;
}

void LEDRainbowEffectDefy::update_delay(uint8_t delay) {
  led_mode.base_settings.delay_ms = delay;
}

// ---------

void LEDRainbowWaveEffectDefy::activate(void) {
    sendLedMode(led_mode);
}

void LEDRainbowWaveEffectDefy::brightness(uint8_t brightness) {
  led_mode.base_settings.brightness = brightness;
}

void LEDRainbowWaveEffectDefy::update_delay(uint8_t delay) {
  led_mode.base_settings.delay_ms = delay;
}

class LEDRainbowEffectDefy LEDRainbowEffectDefy;
class LEDRainbowWaveEffectDefy LEDRainbowWaveEffectDefy;
