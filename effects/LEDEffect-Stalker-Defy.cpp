/* LEDEffect-Stalker-Defy - Solid color LED effects for keyscanner sides.
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

#include "LEDEffect-Stalker-Defy.h"

void LEDStalkerDefy::activate(void) {
    led_mode.r_ = 100;
    led_mode.g_ = 0;
    led_mode.b_ = 0;
    led_mode.w_ = 100;

    sendLedMode(led_mode);
}

class LEDStalkerDefy LEDStalkerDefy;
