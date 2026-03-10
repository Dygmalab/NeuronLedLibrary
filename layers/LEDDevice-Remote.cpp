/* LEDDevice-Remote - Class for specifying and controlling external devices containing leds
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

#include "LEDDevice-Remote.h"
#include "LEDLayers.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

LEDDeviceRemote::LEDDeviceRemote( led_device_type_t led_device_type, uint16_t led_count ) : LEDDevice( led_device_type, led_count )
{
    /* Determine the message command */
    switch( led_device_type )
    {
        /* Device types for regular keyboards (e.g. Sonshi) */
        case LED_DEVICE_TYPE_KS_BL:
        case LED_DEVICE_TYPE_LEFT_BL:
        case LED_DEVICE_TYPE_RIGHT_BL:

            message_command = LAYER_KEYMAP_COLORS;

            break;

        case LED_DEVICE_TYPE_KS_UG:
        case LED_DEVICE_TYPE_LEFT_UG:
        case LED_DEVICE_TYPE_RIGHT_UG:

            message_command = LAYER_UNDERGLOW_COLORS;

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled external LEDDevice type" );

            break;
    }
}

void LEDDeviceRemote::init( uint16_t ledmap_pos )
{
    LEDDevice::init( ledmap_pos );

    /* Get the message size */
    message_size = ( led_count / 2 ) + ( led_count % 2 );
}

void LEDDeviceRemote::update_map( Packet packet )
{
    uint8_t layer_id;
    uint16_t led_id;
    bool_t message_color_first;
    LEDLayers::LEDLayers_layer_colormap_t * p_layer_colormap;
    layer_message_t * p_layer_message;

    // Maximum LEDs that fit in one packet: (MAX_DATA_SIZE - 1) * 2
    // MAX_DATA_SIZE = 28, so max = (28 - 1) * 2 = 54 LEDs
    const uint16_t MAX_LEDS_PER_PACKET = (sizeof(packet.data) - 1) * 2;

    for ( layer_id = 0; layer_id < LEDLayers.layers_count_get(); layer_id++ )
    {
        p_layer_colormap = LEDLayers.layer_colormap_get( layer_id );
        ASSERT_DYGMA( ( (ledmap_pos + led_count) <= p_layer_colormap->size() ), "Current led device map exceeds the full layer colormap space" );

        uint16_t leds_sent = 0;
        
        while (leds_sent < led_count)
        {
            uint16_t leds_in_this_packet = (led_count - leds_sent > MAX_LEDS_PER_PACKET) ? MAX_LEDS_PER_PACKET : (led_count - leds_sent);
            uint16_t this_message_size = (leds_in_this_packet / 2) + (leds_in_this_packet % 2);
            
            // Preserve device before clearing header
            Communications_protocol::Devices target_device = packet.header.device;
            
            // Clear the entire header to avoid residual state from previous packets
            memset(&packet.header, 0, sizeof(packet.header));
            
            // Restore device to ensure packet is routed to correct side
            packet.header.device = target_device;
            packet.header.command = message_command;
            
            // Write directly to bit fields to avoid compiler reinterpretation issues
            bool has_more = ((leds_sent + leds_in_this_packet) < led_count);
            uint8_t size_value = 1 + this_message_size;
            
            // Write directly to the bit fields instead of raw byte
            packet.header.size = size_value;

            p_layer_message = (layer_message_t *)&packet.data[0];
            p_layer_message->layer_id = layer_id;

            message_color_first = true;

            for( led_id = 0; led_id < leds_in_this_packet; led_id++ )
            {
                uint16_t message_colors_id = ( led_id >> 1 );
                uint16_t global_led_id = leds_sent + led_id;

                if( message_color_first == true )
                {
                    p_layer_message->colors[message_colors_id].first = p_layer_colormap->data()[ ledmap_pos + global_led_id ];
                    message_color_first = false;
                }
                else
                {
                    p_layer_message->colors[message_colors_id].second = p_layer_colormap->data()[ ledmap_pos + global_led_id ];
                    message_color_first = true;
                }
            }

            Communications.sendPacket( packet );
            
            leds_sent += leds_in_this_packet;
        }
    }
}
