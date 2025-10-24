/* LEDLayers - Module for controlling, navigating and communicating the LED layers
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

#include "kbd_core.h"
#include "LEDDevice.h"
#include "LEDEffect.h"
#include "LedModeSerializable-Layer.h"
#include "LedModeCommunication.h"

class LEDLayers : public LEDEffect,
                  public LedModeCommunication {

  public:
    typedef struct
    {
        const LEDDevice_list_t * p_LEDDevice_list;  /* List of LED controlled devices */
        uint8_t layers_count;                       /* Number of layers to be operated */
    } LEDLayers_config_t;

    typedef std::vector<uint8_t> LEDLayers_layer_colormap_t;

  public:
    LEDLayers() : LEDEffect( LED_EFFECT_TYPE_DEFAULT ){}

    LedModeSerializable_Layer &led_mode = ledModeSerializableLayer;

    result_t init( const LEDLayers_config_t & config );
    uint8_t layers_count_get( void );
    uint16_t leds_count_get( void );         /* Returns the sum of all LEDs over all LEDDevices */

    void fade_effect_setup( bool_t enable );
    bool_t fade_effect_is_enabled( void );

    LEDLayers_layer_colormap_t * layer_colormap_get( uint8_t layer_id );

    void update_map_backlight( Packet packet );
    void update_map_underglow( Packet packet );

    /* LED Layers activation */
    void active_layer_id_set( kbdapi_led_layer_id_t layer_id );
    void activate() final;

    /* Command processing (external configuration) */
    kbdapi_event_result_t command_process( const char * p_command );

  private:
    const LEDDevice_list_t * p_LEDDevice_list;
    uint8_t layers_count = 0;
    uint16_t leds_count = 0;    /* The sum of all LEDs over all LEDDevices */
    uint16_t colormap_memory_pos = 0;

    LEDLayers_layer_colormap_t layer_colormap;      /* Colormap variable used for providing the layer colors outside of the module */

    LEDDevice * led_device_bl_get( Communications_protocol::Devices com_device );
    LEDDevice * led_device_ug_get( Communications_protocol::Devices com_device );

    uint8_t led_color_get( uint8_t layer, uint16_t layer_led_id );
};

extern class LEDLayers LEDLayers;
