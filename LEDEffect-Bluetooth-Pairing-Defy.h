/* LEDEffect-Bluetooth-Pairing-Defy - Bluetooth Pairing LED representation of paired devices.
 * Copyright (C) 2023, 2024  DygmaLabs, S. L.
 *
 * The MIT License (MIT)
 * Copyright © 2024 <copyright holders>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the “Software”), to deal in the
 * Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "Kaleidoscope-LEDControl.h"
#include "LedModeSerializable-BluetoothPairing.h"
#include "LedModeCommunication.h"

namespace kaleidoscope {
namespace plugin {
class LEDBluetoothPairingDefy : public Plugin,
                                public LEDModeInterface,
                                public LedModeCommunication {
 public:
  LEDBluetoothPairingDefy() {
    led_mode.base_settings.delay_ms = 20;
  }
  LedModeSerializable_BluetoothPairing &led_mode = ledModeSerializableBluetoothPairing;

  void setPairedChannels(uint8_t channel);

  void setConnectedChannel(uint8_t channel);

  void setAvertisingModeOn(uint8_t advertising_id_);

  void setEreaseDone(uint8_t erease_status);

  void setDefyId(uint8_t defy_id);
  // This class' instance has dynamic lifetime
  //
  class TransientLEDMode : public LEDMode {
   public:
    // Please note that storing the parent ptr is only required
    // for those LED modes that require access to
    // members of their parent class. Most LED modes can do without.
    //
    explicit TransientLEDMode(LEDBluetoothPairingDefy *parent)
      : parent_(parent) {}

    void update() final;

   protected:
    void onActivate(void) final;

   private:
    const LEDBluetoothPairingDefy *parent_;
    uint8_t paired_channels_;
  };
};
}  // namespace plugin
}  // namespace kaleidoscope
extern kaleidoscope::plugin::LEDBluetoothPairingDefy ledBluetoothPairingDefy;