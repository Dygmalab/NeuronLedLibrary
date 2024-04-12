/* Kaleidoscope-LEDEffect-Stalker - Solid color LED effects for Kaleidoscope.
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

#include "Kaleidoscope-LEDControl.h"
#include "LedModeSerializable-Stalker.h"
#include "LedModeCommunication.h"

namespace kaleidoscope {
namespace plugin {
class LEDStalkerDefy : public Plugin,
                       public LEDModeInterface,
                       public LedModeCommunication {
 public:
  LEDStalkerDefy() {
    led_mode.base_settings.delay_ms = 50;
  }
  LedModeSerializable_Stalker &led_mode = ledModeSerializableStalker;

  // This class' instance has dynamic lifetime
  //
  class TransientLEDMode : public LEDMode {
   public:
    // Please note that storing the parent ptr is only required
    // for those LED modes that require access to
    // members of their parent class. Most LED modes can do without.
    //
    explicit TransientLEDMode(LEDStalkerDefy *parent)
      : parent_(parent) {}

    void update() final;

   protected:
    void onActivate(void) final;

   private:
    const LEDStalkerDefy *parent_;
  };

 private:
};
}  // namespace plugin
}  // namespace kaleidoscope
