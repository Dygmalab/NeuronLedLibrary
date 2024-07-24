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

#pragma once

#include <Kaleidoscope-LEDControl.h>
#include <Kaleidoscope-LED-Palette-Theme.h>
#include "LedModeSerializable-Layer.h"
#include "LedModeCommunication.h"
//#include "SPISlave.h"

using namespace Communications_protocol;

namespace kaleidoscope {
namespace plugin {
class ColormapEffectDefy : public Plugin,
                           public LEDModeInterface,
                           public LedModeCommunication,
                           public AccessTransientLEDMode {
 public:
  ColormapEffectDefy(void) = default;
  EventHandlerResult onSetup();

  void max_layers(uint8_t max_);
  enum class LedBrightnessControlEffect {
    NONE,
    BATTERY_MANAGMENT,
    FADE_EFFECT,
    BATTERY_STATUS,
    BT_LED_EFFECT,
  };
  LedBrightnessControlEffect no_led_effect = LedBrightnessControlEffect::NONE;
  EventHandlerResult onLayerChange();
  EventHandlerResult onFocusEvent(const char *command);
  void updateColorIndexAtPosition(uint8_t layer, uint16_t position, uint8_t palette_index);
  uint8_t getColorIndexAtPosition(uint8_t layer, uint16_t position);
  LedModeSerializable_Layer &led_mode = ledModeSerializableLayer;
  void getLayer(uint8_t layer, uint8_t output_buf[Runtime.device().led_count]);
  void setFadein(bool fade_status);
  static void updateBrigthness(LedBrightnessControlEffect led_effect_id = LedBrightnessControlEffect::NONE, bool take_brightness_handler = true, bool updateWiredBrightness = true);
  // This class' instance has dynamic lifetime
  //
  class TransientLEDMode : public LEDMode {
   public:
    // Please note that storing the parent ptr is only required
    // for those LED modes that require access to
    // members of their parent class. Most LED modes can do without.
    //
    explicit TransientLEDMode(ColormapEffectDefy *parent)
      : parent_(parent) {}

   protected:
    friend class ColormapEffectDefy;

    void onActivate(void) final;
    void refreshAt(KeyAddr key_addr) final;

   private:
    ColormapEffectDefy *parent_;
  };

 private:
  static uint8_t top_layer_;

 public:
  static uint8_t getMaxLayers();
  static void setSideStatus(Communications_protocol::Devices side);

 private:
  static uint8_t max_layers_;
  static uint16_t map_base_;
  void updateKeyMapCommunications(Packet &packet);
  void updateUnderGlowCommunications(Packet &packet);
};
}  // namespace plugin
}  // namespace kaleidoscope

extern kaleidoscope::plugin::ColormapEffectDefy ColormapEffectDefy;
