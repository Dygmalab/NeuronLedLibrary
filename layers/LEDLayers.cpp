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

#include "Config_manager.h"
#include "LEDLayers.h"
#include "LEDManager.h"

#include "Kaleidoscope-FocusSerial.h"

#define LED_POS( layer_id, layer_led_id ) ( ( APP_LEDS_CNT * layer_id ) + layer_led_id )
#define COLORMAP_DICOLOR_POS( layer_id, layer_led_id ) ( LED_POS(layer_id, layer_led_id) >> 1 )

result_t LEDLayers::init( const LEDLayers_config_t & config )
{
    result_t result = RESULT_ERR;

    LEDDevice * p_LEDDevice;
    uint16_t device_leds_count;
    uint16_t ledmap_pos = 0;
    uint16_t leds_count = 0;
    uint16_t i;

    p_LEDDevice_list = config.p_LEDDevice_list;

    /* Process the device list */
    for( i = 0; i < p_LEDDevice_list->size(); i++ )
    {
        /* Get an instance of the LEDDevice from the list */
        p_LEDDevice = p_LEDDevice_list->data()[i];

        /* Set the ledmap position of this device */
        p_LEDDevice->init( ledmap_pos );

        /* Get the number of leds in this LED device */
        device_leds_count = p_LEDDevice->led_count_get();

        /* Update the number of all leds and the position within the ledmap */
        leds_count += device_leds_count;
        ledmap_pos += device_leds_count;
    }

    if( leds_count != APP_LEDS_CNT )
    {
        ASSERT_DYGMA( false, "Inconsistent LEDDevice_list configuration" );
        return RESULT_ERR;
    }

    /* Prepare the layer colormap space */
    layer_colormap.resize( leds_count );

    /* Get the colormap configuration */
    result = ConfigManager.config_item_request( ConfigManager::CFG_ITEM_TYPE_LEDS_COLORMAP, (const void **)&p_colormap_conf );
    EXIT_IF_ERR( result, "ConfigManager.config_item_request failed" );

_EXIT:
    return result;
}

uint8_t LEDLayers::layers_count_get( void )
{
    return APP_LAYERS_CNT;
}

uint16_t LEDLayers::leds_count_get( void )
{
    return APP_LEDS_CNT;
}

void LEDLayers::fade_effect_setup( bool_t enable )
{
    led_mode.fade_is_on = ( enable == true ) ? 1 : 0;
}

bool_t LEDLayers::fade_effect_is_enabled( void )
{
    return led_mode.fade_is_on;
}

/***********************************/
/*      LEDDevice navigation       */
/***********************************/

LEDDevice * LEDLayers::led_device_bl_get( Communications_protocol::Devices com_device )
{
    LEDDevice * p_LEDDevice;
    uint16_t i;

    for ( i = 0; i < p_LEDDevice_list->size(); i++ )
    {
        p_LEDDevice = p_LEDDevice_list->data()[i];

        if( ( com_device == KEYSCANNER_DEFY_LEFT || com_device == RF_DEFY_LEFT || com_device == BLE_DEFY_LEFT ) &&
                         ( p_LEDDevice->type_get() == LEDDevice::LED_DEVICE_TYPE_LEFT_BL || p_LEDDevice->type_get() == LEDDevice::LED_DEVICE_TYPE_KS_BL ) )
        {
            /*
             * NOTE: Until the communication protocol changes, we accept that for regular keyboards the _LEFT messages are used.
             */

            return p_LEDDevice;
        }
        else if( ( com_device == KEYSCANNER_DEFY_RIGHT || com_device == RF_DEFY_RIGHT || com_device == BLE_DEFY_RIGHT ) &&
                p_LEDDevice->type_get() == LEDDevice::LED_DEVICE_TYPE_RIGHT_BL )
        {
            return p_LEDDevice;
        }
    }

    return nullptr;
}

LEDDevice * LEDLayers::led_device_ug_get( Communications_protocol::Devices com_device )
{
    LEDDevice * p_LEDDevice;
    uint16_t i;

    for ( i = 0; i < p_LEDDevice_list->size(); i++ )
    {
        p_LEDDevice = p_LEDDevice_list->data()[i];

        if( ( com_device == KEYSCANNER_DEFY_LEFT || com_device == RF_DEFY_LEFT || com_device == BLE_DEFY_LEFT ) &&
                         ( p_LEDDevice->type_get() == LEDDevice::LED_DEVICE_TYPE_LEFT_UG || p_LEDDevice->type_get() == LEDDevice::LED_DEVICE_TYPE_KS_UG ) )
        {
            /*
             * NOTE: Until the communication protocol changes, we accept that for regular keyboards the _LEFT messages are used.
             */

            return p_LEDDevice;
        }
        else if( ( com_device == KEYSCANNER_DEFY_RIGHT || com_device == RF_DEFY_RIGHT || com_device == BLE_DEFY_RIGHT ) &&
                p_LEDDevice->type_get() == LEDDevice::LED_DEVICE_TYPE_RIGHT_UG )
        {
            return p_LEDDevice;
        }
    }

    return nullptr;
}

/***********************************/
/*       LED map processing        */
/***********************************/

void LEDLayers::update_map_backlight( Packet packet )
{
    LEDDevice * p_LEDDevice = led_device_bl_get( packet.header.device );

    if( p_LEDDevice != nullptr )
    {
        p_LEDDevice->update_map( packet );
    }
}

void LEDLayers::update_map_underglow( Packet packet )
{
    LEDDevice * p_LEDDevice = led_device_ug_get( packet.header.device );

    if( p_LEDDevice != nullptr )
    {
        p_LEDDevice->update_map( packet );
    }
}

/***********************************/
/*           LED Layers            */
/***********************************/

uint8_t LEDLayers::led_color_get( uint8_t layer_id, uint16_t layer_led_id )
{
    uint16_t led_pos = LED_POS( layer_id, layer_led_id );
    uint16_t colormap_dicolor_pos = COLORMAP_DICOLOR_POS( layer_id, layer_led_id );
    const colormap_dicolor_t * p_dicolor;

    /* Get the dicolor */
    p_dicolor = &p_colormap_conf->dicolors[colormap_dicolor_pos];

    if ( led_pos % 2 )
    {
        return p_dicolor->second;
    }
    else
    {
        return p_dicolor->first;
    }
}

LEDLayers::LEDLayers_layer_colormap_t * LEDLayers::layer_colormap_get( uint8_t layer_id )
{
    uint16_t led_id;

    ASSERT_DYGMA( layer_id < layers_count, "LEDLayers layer_id exceeds the actual number of layers" );
    if( layer_id >= APP_LAYERS_CNT )
    {
        /* Nullify the colormap */
        std::fill(layer_colormap.begin(), layer_colormap.end(), 0);

        return &layer_colormap;
    }

    /* Fill the layer colormap */
    for( led_id = 0; led_id < layer_colormap.size(); led_id++ )
    {
        layer_colormap.data()[led_id] = led_color_get( layer_id, led_id );
    }

    return &layer_colormap;
}

/***********************************/
/*      LED Layers activation      */
/***********************************/

void LEDLayers::active_layer_id_set( kbdapi_led_layer_id_t layer_id )
{
    led_mode.layer = layer_id;
}

void LEDLayers::activate(void)
{
    /* If the host is not connected, the layers will not be activated */
    if( Communications.is_host_connected() == false )
    {
        return;
    }
    sendLedMode(led_mode);
}

/***********************************/
/*        Command processing       */
/***********************************/
kbdapi_event_result_t LEDLayers::command_process( const char * p_command )
{
    const char *expected_command = "colormap.map";

    if ( APP_LEDS_CNT == 0 )
    {
        return KBDAPI_EVENT_RESULT_IGNORED;
    }

    if ( ::Focus.handleHelp( p_command, expected_command ) )
    {
        return KBDAPI_EVENT_RESULT_IGNORED;
    }

    if ( strcmp( p_command, expected_command ) != 0 )
    {
        return KBDAPI_EVENT_RESULT_IGNORED;
    }

    if ( ::Focus.isEOL( ) )
    {
        for( uint16_t i = 0; i < COLORMAP_DICOLORS_CNT; i++ )
        {
            const colormap_dicolor_t * p_dicolor = &p_colormap_conf->dicolors[i];
            ::Focus.send( p_dicolor->first, p_dicolor->second );
        }

        return KBDAPI_EVENT_RESULT_CONSUMED;
    }

    uint16_t dicolor_id = 0;

    while ( !::Focus.isEOL( ) && ( dicolor_id < COLORMAP_DICOLORS_CNT ) )
    {
        uint8_t color1, color2;
        colormap_dicolor_t dicolor;
        ::Focus.read( color1 );
        ::Focus.read( color2 );

        dicolor.first = color1;
        dicolor.second = color2;

        cfgmem_dicolor_save( dicolor_id, &dicolor );

        dicolor_id++;
    }

    auto const &keyScanner = kaleidoscope::Runtime.device().keyScanner();
    auto deviceLeft = keyScanner.leftHandDevice();
    auto devicesRight = keyScanner.rightHandDevice();
    Packet packet{};
    packet.header.device = deviceLeft;
    update_map_backlight( packet );
    update_map_underglow( packet );
    packet.header.device = devicesRight;
    update_map_backlight( packet );
    update_map_underglow( packet );

    LEDManager.led_effect_refresh();

    return KBDAPI_EVENT_RESULT_CONSUMED;
}

/****************************************************/
/*                   Config Memory                  */
/****************************************************/

void LEDLayers::cfgmem_dicolor_save( uint16_t dicolor_id, colormap_dicolor_t * p_dicolor )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_colormap_conf->dicolors[dicolor_id], p_dicolor, sizeof(colormap_dicolor_t) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

class LEDLayers LEDLayers;
