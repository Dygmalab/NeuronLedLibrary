/* -*- mode: c++ -*-
* Kaleidoscope-Colormap -- Per-layer colormap effect
* Copyright (C) 2016, 2017, 2018  Keyboard.io, Inc
* Copyright (C) 2023, 2024  DygmaLabs, S. L.
*
* This program is free software: you can redistribute it and/or modify it under it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation, version 3.
*
* This program is distributed in the hope that it will be useful, but WITHOUT but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with along with
* this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include "Colormap-Defy.h"
#include <Kaleidoscope-EEPROM-Settings.h>
#include <Kaleidoscope-FocusSerial.h>
#include "kaleidoscope/layers.h"
#include "LED-Palette-Theme-Defy.h"


namespace kaleidoscope {
    namespace plugin {

        uint16_t ColormapEffectDefy::map_base_;
        uint8_t ColormapEffectDefy::max_layers_;
        uint8_t ColormapEffectDefy::top_layer_;
        bool right_side;

        void ColormapEffectDefy::max_layers(uint8_t max_) {
            if (map_base_ != 0) {
                return;
            }

            max_layers_ = max_;
            map_base_ = ::LEDPaletteThemeDefy.reserveThemes(max_layers_);
        }

        void ColormapEffectDefy::TransientLEDMode::onActivate(void) {
            if (!Runtime.has_leds) {
                return;
            }

            parent_->top_layer_ = Layer.mostRecent();
            parent_->led_mode.layer = parent_->top_layer_;
            sendLedMode(parent_->led_mode);
            if (parent_->top_layer_ <= parent_->max_layers_) {
                ::LEDPaletteThemeDefy.updateHandler(parent_->map_base_, parent_->top_layer_);
            }
        }

        void ColormapEffectDefy::updateColorIndexAtPosition(uint8_t layer, uint16_t position, uint8_t palette_index) {
            if (layer >= max_layers_) {
                return;
            }

            uint16_t index = (Runtime.device().led_count) * layer + position;
            ::LEDPaletteThemeDefy.updateColorIndexAtPosition(map_base_, index, palette_index);
        }

        uint8_t ColormapEffectDefy::getColorIndexAtPosition(uint8_t layer, uint16_t position) {
            if (layer >= max_layers_) {
                return 0;
            }

            uint16_t index = (Runtime.device().led_count) * layer + position;

            return ::LEDPaletteThemeDefy.lookupColorIndexAtPosition(map_base_, index);
        }

        void ColormapEffectDefy::TransientLEDMode::refreshAt(KeyAddr key_addr) {
            if (parent_->top_layer_ <= parent_->max_layers_) {
                ::LEDPaletteThemeDefy.refreshAt(parent_->map_base_, parent_->top_layer_, key_addr);
            }
        }

        EventHandlerResult ColormapEffectDefy::onLayerChange() {
            if (::LEDControl.get_mode_index() == led_mode_id_) {
                ::LEDControl.get_mode<TransientLEDMode>()->onActivate();
            }

            return EventHandlerResult::OK;
        }

        void ColormapEffectDefy::setFadein(bool fade_status) {
            led_mode.fade_is_on = fade_status;
        }

        EventHandlerResult ColormapEffectDefy::onFocusEvent(const char *command) {
            const char *expected_command = "colormap.map";

            if (!Runtime.has_leds) {
                return EventHandlerResult::OK;
            }

            if (::Focus.handleHelp(command, expected_command)) {
                return EventHandlerResult::OK;
            }

            if (strcmp(command, expected_command) != 0) {
                return EventHandlerResult::OK;
            }

            uint16_t max_index = (max_layers_ * (Runtime.device().led_count / 2));

            if (::Focus.isEOL()) {
                for (uint16_t pos = 0; pos < max_index; pos++) {
                    uint8_t indexes = Runtime.storage().read(map_base_ + pos);

                    ::Focus.send((uint8_t)(indexes >> 4), indexes & ~0xf0);
                }

                return EventHandlerResult::EVENT_CONSUMED;
            }

            uint16_t pos = 0;

            while (!::Focus.isEOL() && (pos < max_index)) {
                uint8_t idx1, idx2;
                ::Focus.read(idx1);
                ::Focus.read(idx2);

                uint8_t indexes = (idx1 << 4) + idx2;

                Runtime.storage().update(map_base_ + pos, indexes);
                pos++;
            }

            Runtime.storage().commit();
            auto const &keyScanner = Runtime.device().keyScanner();
            auto deviceLeft = keyScanner.leftHandDevice();
            auto devicesRight = keyScanner.rightHandDevice();
            Packet packet{};
            packet.header.device = deviceLeft;
            updateKeyMapCommunications(packet);
            updateUnderGlowCommunications(packet);
            packet.header.device = devicesRight;
            updateKeyMapCommunications(packet);
            updateUnderGlowCommunications(packet);

            ::LEDControl.refreshAll();

            return EventHandlerResult::EVENT_CONSUMED;
        }

        void ColormapEffectDefy::getLayer(uint8_t layer, uint8_t output_buf[Runtime.device().led_count])
        {
            for (uint16_t i = 0; i < Runtime.device().led_count; ++i)
            {
                output_buf[i] = getColorIndexAtPosition(layer, i);
            }
        }

        uint8_t ColormapEffectDefy::getMaxLayers() {
            return max_layers_;
        }

        EventHandlerResult ColormapEffectDefy::onSetup() {
            Communications.callbacks.bind(CONNECTED,
                                          ([](Packet packet) {
                                              LEDPaletteThemeDefy::updatePaletteCommunication(packet);
                                          }));

            Communications.callbacks.bind(CONNECTED,
                                          ([this](Packet packet) {
                                              updateKeyMapCommunications(packet);
                                          }));

            Communications.callbacks.bind(CONNECTED,
                                          ([this](Packet packet) {
                                              updateUnderGlowCommunications(packet);
                                          }));
            Communications.callbacks.bind(RETRY_LAYERS,
                                          ([this](Packet packet) {
                                              updateKeyMapCommunications(packet);
                                          }));

            Communications.callbacks.bind(RETRY_LAYERS,
                                          ([this](Packet packet) {
                                              updateUnderGlowCommunications(packet);
                                          }));
            Communications.callbacks.bind(RETRY_LAYERS,
                                          ([this](Packet packet) {
                                              ::LEDControl.set_mode(::LEDControl.get_mode_index());
                                          }));
            return EventHandlerResult::OK;
        }

void ColormapEffectDefy::updateKeyMapCommunications(Packet &packet)
{
    union PaletteJoiner {
    struct {
      uint8_t firstColor : 4;
      uint8_t secondColor : 4;
    };
    uint8_t paletteColor;
  };

  uint8_t baseKeymapIndex;
  if (packet.header.device == KEYSCANNER_DEFY_RIGHT || packet.header.device == Communications_protocol::RF_DEFY_RIGHT || packet.header.device == Communications_protocol::BLE_DEFY_RIGHT) {
    baseKeymapIndex = Runtime.device().ledDriver().key_matrix_left;
    right_side = true;
  } else {
    baseKeymapIndex = 0;
    right_side = false;
  }

  for (uint8_t layer = 0; layer < max_layers_; ++layer) {
    if (right_side){

      uint8_t layerColors[Runtime.device().led_count];
      getLayer(layer, layerColors);
      packet.header.command       = LAYER_KEYMAP_COLORS;
      const uint8_t sizeofMessage = (Runtime.device().ledDriver().key_matrix_right)/ 2.0 + 0.5;
      PaletteJoiner message[sizeofMessage];
      packet.header.size = sizeof(message) + 1;
      packet.data[0]     = layer;
      uint8_t k{};
      bool swap = true;
      for (int j = 0; j < Runtime.device().ledDriver().key_matrix_right; ++j) {
        if (swap) {
          message[k].firstColor = layerColors[baseKeymapIndex + j];
        } else {
          message[k++].secondColor = layerColors[baseKeymapIndex + j];
        }
        swap = !swap;
      }
      memcpy(&packet.data[1], message, packet.header.size - 1);
      Communications.sendPacket(packet);
    } else {

      uint8_t layerColors[Runtime.device().led_count];
      getLayer(layer, layerColors);
      packet.header.command       = LAYER_KEYMAP_COLORS;
      const uint8_t sizeofMessage = Runtime.device().ledDriver().key_matrix_left / 2.0 + 0.5;
      PaletteJoiner message[sizeofMessage];
      packet.header.size = sizeof(message) + 1;
      packet.data[0]     = layer;
      uint8_t k{};
      bool swap = true;
      for (int j = 0; j < Runtime.device().ledDriver().key_matrix_left; ++j) {
        if (swap) {
          message[k].firstColor = layerColors[baseKeymapIndex + j];
        } else {
          message[k++].secondColor = layerColors[baseKeymapIndex + j];
        }
        swap = !swap;
      }
      memcpy(&packet.data[1], message, packet.header.size - 1);
      Communications.sendPacket(packet);
      right_side = false;
    }
  }
}

void ColormapEffectDefy::updateUnderGlowCommunications(Packet &packet)
{
    right_side = false;

  union PaletteJoiner {
    struct {
      uint8_t firstColor : 4;
      uint8_t secondColor : 4;
    };
    uint8_t paletteColor;
  };

  uint8_t baseUnderGlowIndex;
  if (packet.header.device == KEYSCANNER_DEFY_RIGHT || packet.header.device == Communications_protocol::RF_DEFY_RIGHT || packet.header.device == Communications_protocol::BLE_DEFY_RIGHT) {
    baseUnderGlowIndex = Runtime.device().ledDriver().key_matrix_left + Runtime.device().ledDriver().key_matrix_right +
                         Runtime.device().ledDriver().underglow_leds ;
    right_side = true;
  }
  else
  {
    baseUnderGlowIndex = Runtime.device().ledDriver().key_matrix_left + Runtime.device().ledDriver().key_matrix_right;
    right_side = false;
  }
  if (right_side)
  {
    uint8_t layerColors[Runtime.device().led_count];
    for (uint8_t layer = 0; layer < max_layers_; ++layer)
    {
      getLayer(layer, layerColors);
      packet.header.command       = Communications_protocol::LAYER_UNDERGLOW_COLORS;
      const uint8_t sizeofMessage = Runtime.device().ledDriver().underglow_leds_right / 2.0 + 0.5;
      PaletteJoiner message[sizeofMessage];
      packet.header.size = sizeof(message) + 1;
      packet.data[0]     = layer;
      bool swap          = true;
      uint8_t k{};
      for (int j = 0; j < Runtime.device().ledDriver().underglow_leds_right; ++j)
      {
        if (swap)
        {
          message[k].firstColor = layerColors[baseUnderGlowIndex + j];
        }
        else
        {
          message[k++].secondColor = layerColors[baseUnderGlowIndex + j];
        }
        swap = !swap;
      }
      memcpy(&packet.data[1], message, packet.header.size - 1);
      Communications.sendPacket(packet);
    }
  }
  else
  {
    uint8_t layerColors[Runtime.device().led_count];
    for (uint8_t layer = 0; layer < max_layers_; ++layer)
    {
      getLayer(layer, layerColors);
      packet.header.command       = Communications_protocol::LAYER_UNDERGLOW_COLORS;
      const uint8_t sizeofMessage = Runtime.device().ledDriver().underglow_leds / 2.0 + 0.5;
      PaletteJoiner message[sizeofMessage];
      packet.header.size = sizeof(message) + 1;
      packet.data[0]     = layer;
      bool swap          = true;
      uint8_t k{};
      for (int j = 0; j < Runtime.device().ledDriver().underglow_leds; ++j)
      {
        if (swap)
        {
          message[k].firstColor = layerColors[baseUnderGlowIndex + j];
        }
        else
        {
          message[k++].secondColor = layerColors[baseUnderGlowIndex + j];
        }
        swap = !swap;
      }
      memcpy(&packet.data[1], message, packet.header.size - 1);
      Communications.sendPacket(packet);
    }
  }
}

void ColormapEffectDefy::updateBrigthness(LedBrightnessControlEffect led_effect_id, bool take_brightness_handler, bool updateWiredBrightness)
{
  Packet packet;
  packet.header.command = BRIGHTNESS;
  packet.header.size    = 4;

  auto &ledDriver = Runtime.device().ledDriver();

  if (updateWiredBrightness) {
    packet.data[0] = ledDriver.getBrightness();
    packet.data[1] = ledDriver.getBrightnessUG();
  } else {
    packet.data[0] = ledDriver.getBrightnessWireless();
    packet.data[1] = ledDriver.getBrightnessUGWireless();
  }
  packet.data[2] = static_cast<uint8_t>(led_effect_id); //LED effect ID.
  packet.data[3] = take_brightness_handler; // Tell KS that we want to take (or left) brightness control.
  packet.header.device = UNKNOWN;
  Communications.sendPacket(packet);
}

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::ColormapEffectDefy ColormapEffectDefy;
