/* Kaleidoscope-LEDEffect-SolidColor - Solid color LED effects for Kaleidoscope.
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

#include "LEDEffect-Breathe-Defy.h"

namespace kaleidoscope {
namespace plugin {

void LEDBreatheDefy::TransientLEDMode::onActivate(void) {
  parent_->led_mode.breatheHue             = parent_->hue_;
  parent_->led_mode.breatheSaturation      = parent_->saturation_;
  parent_->led_mode.base_settings.delay_ms = 50;
  sendLedMode(parent_->led_mode);
}

void LEDBreatheDefy::TransientLEDMode::update(void) {
  parent_->led_mode.update();
}
}  // namespace plugin
}  // namespace kaleidoscope
