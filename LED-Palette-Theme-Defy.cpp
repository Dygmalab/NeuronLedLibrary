/* -*- mode: c++ -*-
 * Kaleidoscope-LED-Palette-Theme -- Palette-based LED theme foundation
 * Copyright (C) 2017, 2018, 2019  Keyboard.io, Inc
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

#include <LED-Palette-Theme-Defy.h>
#include <Kaleidoscope-EEPROM-Settings.h>
#include <Kaleidoscope-FocusSerial.h>
#include "Communications.h"

namespace kaleidoscope {
namespace plugin {

uint16_t LEDPaletteThemeDefy::palette_base_;
uint16_t LEDPaletteThemeDefy::leds_per_layer_in_memory_;

uint16_t LEDPaletteThemeDefy::reserveThemes(uint8_t max_themes) {
  if (!palette_base_)
    palette_base_ = ::EEPROMSettings.requestSlice(16 * sizeof(cRGB));
  //plus 2 so that we can have the neuron in a full position
  leds_per_layer_in_memory_ = (Runtime.device().led_count) / 2;
  return ::EEPROMSettings.requestSlice(max_themes * leds_per_layer_in_memory_);
}

void LEDPaletteThemeDefy::updateHandler(uint16_t theme_base, uint8_t theme) {
  if (!Runtime.has_leds)
    return;

  uint16_t map_base = theme_base + (theme * leds_per_layer_in_memory_);

  for (uint8_t pos = Runtime.device().led_count - 2; pos < Runtime.device().led_count; pos++) {
    cRGB color = lookupColorAtPosition(map_base, pos);
    ::LEDControl.setCrgbAt(pos, color);
  }
}

void LEDPaletteThemeDefy::refreshAt(uint16_t theme_base, uint8_t theme, KeyAddr key_addr) {
  if (!Runtime.has_leds)
    return;

  uint16_t map_base = theme_base + (theme * leds_per_layer_in_memory_);
  uint8_t pos       = Runtime.device().getLedIndex(key_addr);

  cRGB color = lookupColorAtPosition(map_base, pos);
  ::LEDControl.setCrgbAt(key_addr, color);
}

const uint8_t LEDPaletteThemeDefy::lookupColorIndexAtPosition(uint16_t map_base, uint16_t position) {
  uint8_t color_index;

  color_index = Runtime.storage().read(map_base + position / 2);
  if (position % 2)
    color_index &= ~0xf0;
  else
    color_index >>= 4;

  return color_index;
}

const cRGB LEDPaletteThemeDefy::lookupColorAtPosition(uint16_t map_base, uint16_t position) {
  uint8_t color_index = lookupColorIndexAtPosition(map_base, position);
  return lookupPaletteColor(color_index);
}

const cRGB LEDPaletteThemeDefy::lookupPaletteColor(uint8_t color_index) {
  cRGB color;

  Runtime.storage().get(palette_base_ + color_index * sizeof(cRGB), color);
  color.r ^= 0xff;
  color.g ^= 0xff;
  color.b ^= 0xff;
  color.w ^= 0xff;
  return color;
}

void LEDPaletteThemeDefy::updateColorIndexAtPosition(uint16_t map_base, uint16_t position, uint8_t color_index) {
  uint8_t indexes;

  indexes = Runtime.storage().read(map_base + position / 2);
  if (position % 2) {
    uint8_t other = indexes >> 4;
    indexes       = (other << 4) + color_index;
  } else {
    uint8_t other = indexes & ~0xf0;
    indexes       = (color_index << 4) + other;
  }
  Runtime.storage().update(map_base + position / 2, indexes);
}

EventHandlerResult LEDPaletteThemeDefy::onFocusEvent(const char *command) {
  if (!Runtime.has_leds)
    return EventHandlerResult::OK;

  const char *cmd = "palette";

  if (::Focus.handleHelp(command, cmd))
    return EventHandlerResult::OK;

  if (strcmp(command, cmd) != 0)
    return EventHandlerResult::OK;

  if (::Focus.isEOL()) {
    for (uint8_t i = 0; i < 16; i++) {
      cRGB color;

      color = lookupPaletteColor(i);
      ::Focus.send(color.r, color.g, color.b, color.w);
    }
    return EventHandlerResult::EVENT_CONSUMED;
  }

  uint8_t i = 0;
  while (i < 16 && !::Focus.isEOL()) {
    cRGB color;

    ::Focus.read(color.r);
    ::Focus.read(color.g);
    ::Focus.read(color.b);
    ::Focus.read(color.w);
    ::LEDPaletteThemeDefy.updatePaletteColor(i, color);
    i++;
  }

  Runtime.storage().commit();
  Packet packet{};
  packet.header.device = UNKNOWN;
  LEDPaletteThemeDefy::updatePaletteCommunication(packet);

  ::LEDControl.refreshAll();
  return EventHandlerResult::EVENT_CONSUMED;
}

void LEDPaletteThemeDefy::updatePaletteCommunication(Packet &packet) {
  packet.header.command = Communications_protocol::PALETTE_COLORS;
  packet.header.size    = sizeof(cRGB) * 6;
  cRGB palette[16];
  getColorPalette(palette);
  packet.data[0] = 0;
  memcpy(&packet.data[1], &palette[packet.data[0]], packet.header.size);
  Communications.sendPacket(packet);
  packet.data[0] = 6;
  memcpy(&packet.data[1], &palette[packet.data[0]], packet.header.size);
  Communications.sendPacket(packet);
  packet.header.size = sizeof(cRGB) * 4;
  packet.data[0]     = 12;
  memcpy(&packet.data[1], &palette[packet.data[0]], packet.header.size);
  Communications.sendPacket(packet);
}

void LEDPaletteThemeDefy::getColorPalette(cRGB output_palette[16]) {
  for (int i = 0; i < 16; ++i) {
    const cRGB &color = ::LEDPaletteThemeDefy.lookupPaletteColor(i);
    output_palette[i] = color;
  }
}

void LEDPaletteThemeDefy::updatePaletteColor(uint8_t palette_index, cRGB color) {
  color.r ^= 0xff;
  color.g ^= 0xff;
  color.b ^= 0xff;
  color.w ^= 0xff;

  Runtime.storage().put(palette_base_ + palette_index * sizeof(color), color);
}
}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::LEDPaletteThemeDefy LEDPaletteThemeDefy;
