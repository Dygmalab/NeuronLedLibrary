/* LEDDevice-Remote - Class for specifying and controlling remote devices containing leds
 * Copyright (C) 2025  DygmaLabs, S. L.
 *
 * The MIT License (MIT)
 * Copyright © 2025 <copyright holders>
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

#include "LEDDevice.h"
#include "LEDTypes.h"
#include "Communications_protocol.h"

class LEDDeviceRemote : public LEDDevice
{
  public:
    /* Constructor */
    LEDDeviceRemote( led_device_type_t led_device_type, uint16_t led_count );

    void init( uint16_t ledmap_pos ) final;
    void update_map( Packet packet );

  private:

    typedef struct
    {
        uint8_t first : 4;
        uint8_t second : 4;
    } PACK layer_message_color_t;

    typedef struct
    {
        uint8_t layer_id;
        layer_message_color_t colors[];
    } PACK layer_message_t;

  private:

    Communications_protocol::Commands message_command;
    uint16_t message_size;  /* Size of the message inserted to the Packet */
};
