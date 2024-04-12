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

#include "LEDEffect-Bluetooth-Pairing-Defy.h"

namespace kaleidoscope {
namespace plugin {

void LEDBluetoothPairingDefy::TransientLEDMode::onActivate(void) {
  sendLedMode(parent_->led_mode);
}

void LEDBluetoothPairingDefy::TransientLEDMode::update(void) {
  parent_->led_mode.update();
}
void LEDBluetoothPairingDefy::setPairedChannels(uint8_t channel) {
  ledModeSerializableBluetoothPairing.paired_channels_ = channel;
}
void LEDBluetoothPairingDefy::setConnectedChannel(uint8_t channel) {
  ledModeSerializableBluetoothPairing.connected_channel_id_ = channel;
}
void LEDBluetoothPairingDefy::setAvertisingModeOn(uint8_t advertising_id_) {
  ledModeSerializableBluetoothPairing.advertising_id = advertising_id_;
}
void LEDBluetoothPairingDefy::setEreaseDone(uint8_t erease_status) {
  ledModeSerializableBluetoothPairing.erease_done = erease_status;
}
void LEDBluetoothPairingDefy::setDefyId(uint8_t defy_id) {
  ledModeSerializableBluetoothPairing.advertising_id = defy_id;
}
}  // namespace plugin
}  // namespace kaleidoscope
kaleidoscope::plugin::LEDBluetoothPairingDefy ledBluetoothPairingDefy;