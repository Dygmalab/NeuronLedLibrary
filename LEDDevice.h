/* LEDDevice - Class for specifying and controlling a device containing leds
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


#include <stdint.h>
#include <vector>
#include "Communications.h"

class LEDDevice
{
  public:

    typedef enum
    {
        /* Local devices */
        LED_DEVICE_TYPE_NEURON = 1,

        /* Device types for regular keyboards (e.g. Sonshi) */
        LED_DEVICE_TYPE_KS_BL,
        LED_DEVICE_TYPE_KS_UG,

        /* Device types for split keyboards (e.g. Defy or Raise 2) */
        LED_DEVICE_TYPE_LEFT_BL,
        LED_DEVICE_TYPE_LEFT_UG,
        LED_DEVICE_TYPE_RIGHT_BL,
        LED_DEVICE_TYPE_RIGHT_UG,

        /* Add more device types here */
//        LED_DEVICE_TYPE_,
//        LED_DEVICE_TYPE_,
    } led_device_type_t;

    /* Constructor */
    explicit LEDDevice( led_device_type_t led_device_type, uint16_t led_count ) : led_device_type(led_device_type), led_count(led_count) {}


    virtual void init( uint16_t ledmap_pos ) { this->ledmap_pos = ledmap_pos; }

    led_device_type_t type_get( void ) { return led_device_type; }
    uint16_t led_count_get( void ) { return led_count; }

    virtual void update_map( Packet packet ) {}

  protected:
    led_device_type_t led_device_type;
    uint16_t led_count = 0;
    uint16_t ledmap_pos = 0;
};

typedef std::vector<LEDDevice*> LEDDevice_list_t;
