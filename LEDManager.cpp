/* LEDManager - Managing the LED states and processes like led effects, layers, communication etc.
 * Copyright (C) 2025 DygmaLabs, S. L.
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

#include "kbd_if_manager.h"
#include "LEDManager.h"

#include "LEDEffect-BatteryStatus-Defy.h"
#include "LEDEffect-Bluetooth-Pairing-Defy.h"
#include "LEDEffect-Rainbow-Defy.h"
#include "LEDEffect-SolidColor-Defy.h"
#include "LEDEffect-Stalker-Defy.h"

#include "LED-Palette-Theme-Defy.h"
#include "Kaleidoscope-FocusSerial.h"
//#include "Kaleidoscope-EEPROM-Settings.h"
#warning "Temporary use of LEDControlDygma outside of the kaleidoscope_adapter"
#include "LEDControlDygma.h"

const LEDManager::LEDEffect_list_t LEDManager::LEDEffect_list_regular =
{
    &LEDLayers, &LEDRainbowEffectDefy, &LEDRainbowWaveEffectDefy, &LEDStalkerDefy,
    &solidRedDefy, &solidGreenDefy, &solidBlueDefy, &solidWhiteDefy, &solidBlackDefy,
};

const LEDManager::LEDEffect_list_t LEDManager::LEDEffect_list_specific =
{
    &LEDBatteryStatusDefy, &LEDBluetoothPairingDefy,
};

result_t LEDManager::init( const LEDManager_config_t & config )
{
    result_t result = RESULT_ERR;

    /* Initialize the keyboard interface */
    result = kbdif_initialize( );
    EXIT_IF_ERR( result, "kbdif_initialize failed" );

    /* Initialize the keyscanner communication interface */
    result = comks_init( );
    EXIT_IF_ERR( result, "com_ks_init failed" );

    /* Initialize the led layers */
    result = LEDLayers.init( config.layers );
    EXIT_IF_ERR( result, "LEDLayers.init failed" );


    /* Initialize the default led effect to be the LEDLayers one */
    p_LEDEffect = &LEDLayers;

_EXIT:
    return result;
}

/****************************************************/
/*                    LED Control                   */
/****************************************************/

void LEDManager::update_brightness( brightness_led_effect_t led_effect, bool_t take_brightness_control, bool_t update_wired_brightness )
{
    comks_update_brightness( led_effect, take_brightness_control, update_wired_brightness );
}

/****************************************************/
/*         LED Effect control and navigation        */
/****************************************************/

LEDEffect * LEDManager::led_effect_search_type( const LEDEffect_list_t & effect_list, LEDEffect::led_effect_type_t effect_type )
{
    LEDEffect * p_led_effect;
    led_effect_id_t i;

    /* Search the regular LED effect list */
    for( i = 0; i < (led_effect_id_t)effect_list.size(); i++ )
    {
       p_led_effect = effect_list[i];

       if( p_led_effect->type_get() == effect_type )
       {
           return p_led_effect;
       }
    }

    return nullptr;
}

LEDEffect * LEDManager::led_effect_search_type( LEDEffect::led_effect_type_t effect_type )
{
    LEDEffect * p_led_effect;

    /* Search the regular LED effect list */
    p_led_effect = led_effect_search_type( LEDEffect_list_regular, effect_type );
    if( p_led_effect != nullptr )
    {
        return p_led_effect;
    }

    /* Search the specific LED effect list */
    p_led_effect = led_effect_search_type( LEDEffect_list_specific, effect_type );
    if( p_led_effect != nullptr )
    {
        return p_led_effect;
    }

    return nullptr;
}

void LEDManager::led_effect_set_type( LEDEffect::led_effect_type_t effect_type, bool prio )
{
    LEDEffect * p_led_effect;

    /* Check if there is a priority LED effect already active */
    if( p_LEDEffect_prio != nullptr )
    {
        return;
    }

    /* Search the led effect by type */
    p_led_effect = led_effect_search_type( effect_type );
    if( p_led_effect == nullptr )
    {
        ASSERT_DYGMA( false, "Invalid LED effect type detected" );
        return;
    }

    if( prio == true )
    {
        p_LEDEffect_prio = p_led_effect;
    }
    else
    {
        p_LEDEffect = p_led_effect;
    }

    led_effect_activate();
}

void LEDManager::led_effect_set_id( led_effect_id_t id )
{
//    /* Check if there is a priority LED effect currenty active */
//    if( p_LEDEffect_prio != nullptr )
//    {
//        return;
//    }

    /* Set the LED effect */
    p_LEDEffect = LEDEffect_list_regular[id];

    led_effect_activate( );
}

void LEDManager::led_effect_set_next( void )
{
    /* Check if there is a priority LED effect currenty active */
    if( p_LEDEffect_prio != nullptr )
    {
        return;
    }

    LEDEffect_id_regular++;
    if( LEDEffect_id_regular >= (led_effect_id_t)LEDEffect_list_regular.size() )
    {
        LEDEffect_id_regular = 0;
    }

    led_effect_set_id( LEDEffect_id_regular );
}

void LEDManager::led_effect_set_previous( void )
{
    /* Check if there is a priority LED effect currenty active */
    if( p_LEDEffect_prio != nullptr )
    {
        return;
    }

    LEDEffect_id_regular--;
    if( LEDEffect_id_regular < 0 )
    {
        LEDEffect_id_regular = LEDEffect_list_regular.size() - 1;
    }

    led_effect_set_id( LEDEffect_id_regular );
}

void LEDManager::led_effect_set( LEDEffect::led_effect_type_t effect_type )
{
    led_effect_set_type( effect_type, false );
}

void LEDManager::led_effect_set_prio( LEDEffect::led_effect_type_t effect_type )
{
    led_effect_set_type( effect_type, true );
}

void LEDManager::led_effect_reset_prio( void )
{
    p_LEDEffect_prio = nullptr;

    led_effect_activate();
}

LEDEffect::led_effect_type_t LEDManager::led_effect_get_active_type( void )
{
    return ( p_LEDEffect_prio != nullptr ) ? p_LEDEffect_prio->type_get() : p_LEDEffect->type_get();
}

void LEDManager::led_effect_activate( void )
{
    if( p_LEDEffect_prio != nullptr )
    {
        p_LEDEffect_prio->activate();
    }
    else if( p_LEDEffect != nullptr )
    {
        p_LEDEffect->activate();
    }
}

void LEDManager::led_effect_refresh( void )
{
    /* We call the led_effect_activate to refresh the current setup */
    led_effect_activate();
}

/****************************************************/
/*         LED Layers control and navigation        */
/****************************************************/

void LEDManager::led_layer_set( kbdapi_led_layer_id_t layer_id )
{
    LEDLayers.active_layer_id_set( layer_id );

    /* Attempt to activate new layer only if the "Layers led effect type is active" */
    if( led_effect_get_active_type( ) == LEDLayers.type_get() )
    {
        led_effect_activate( );
    }
}

/****************************************************/
/*                Keyboard interface                */
/****************************************************/

result_t LEDManager::kbdif_initialize()
{
    result_t result = RESULT_ERR;
    kbdif_conf_t config;

    /* Prepare the kbdif configuration */
    config.p_instance = this;
    config.handlers = &kbdif_handlers;

    /* Initialize the kbdif */
    result = kbdif_init( &p_kbdif, &config );
    EXIT_IF_ERR( result, "kbdif_init failed" );

    /* Add the kbdif into the kbdif manager */
    result = kbdifmgr_add( p_kbdif );
    EXIT_IF_ERR( result, "kbdifmgr_add failed" );

_EXIT:
    return result;
}

kbdapi_event_result_t LEDManager::kbdif_command_event_cb( void * p_instance, const char * p_command )
{
    LEDManager * p_LEDManager = ( LEDManager *)p_instance;
    kbdapi_event_result_t result = KBDAPI_EVENT_RESULT_IGNORED;

    result = p_LEDManager->command_led_process( p_command );
    EXIT_IF_KBDAPI_NOT_IGNORED( result );

    result = LEDLayers.command_process( p_command );
    EXIT_IF_KBDAPI_NOT_IGNORED( result );

_EXIT:
    return result;
}

kbdapi_event_result_t LEDManager::kbdif_led_layer_change_event_cb( void * p_instance, kbdapi_led_layer_id_t layer_id )
{
    LEDManager * p_LEDManager = ( LEDManager *)p_instance;

    p_LEDManager->led_layer_set( layer_id );

    return KBDAPI_EVENT_RESULT_CONSUMED;
}

kbdapi_event_result_t LEDManager::kbdif_led_effect_change_event_cb( void * p_instance, kbdapi_led_effect_action_t action )
{
    LEDManager * p_LEDManager = ( LEDManager *)p_instance;

    switch( action )
    {
        case KBDAPI_LED_EFFECT_ACTION_NONE:
            break;

        case KBDAPI_LED_EFFECT_ACTION_DISABLE:
            break;

        case KBDAPI_LED_EFFECT_ACTION_ENABLE:
            break;

        case KBDAPI_LED_EFFECT_ACTION_NEXT:

            p_LEDManager->led_effect_set_next();

            break;

        case KBDAPI_LED_EFFECT_ACTION_PREVIOUS:

            p_LEDManager->led_effect_set_previous();

            break;

        default:
            ASSERT_DYGMA( false, "Unhandled kbdapi led effect action detected" );
            break;
    }

    return KBDAPI_EVENT_RESULT_CONSUMED;
}

const kbdif_handlers_t LEDManager::kbdif_handlers =
{
    .command_event_cb = kbdif_command_event_cb,
    .led_layer_change_event_cb = kbdif_led_layer_change_event_cb,
    .led_effect_change_event_cb = kbdif_led_effect_change_event_cb,
};

/****************************************************/
/*             Keyscanner Communication             */
/****************************************************/

void LEDManager::comks_connected( Packet packet )
{
    LEDPaletteThemeDefy.updatePaletteCommunication(packet);
    LEDLayers.update_map_backlight( packet );
    LEDLayers.update_map_underglow( packet );
}

void LEDManager::comks_retry_layers( Packet packet )
{
    LEDPaletteThemeDefy.updatePaletteCommunication(packet);
    LEDLayers.update_map_backlight( packet );
    LEDLayers.update_map_underglow( packet );
    led_effect_refresh();
}

void LEDManager::comks_update_brightness( brightness_led_effect_t led_effect, bool_t take_brightness_control, bool_t update_wired_brightness )
{
    Packet packet;
    packet.header.command = BRIGHTNESS;
    packet.header.size = 4;
    brightness_message_t * p_message = (brightness_message_t *)packet.data;

    if ( update_wired_brightness )
    {
        p_message->backlight_brightness = LEDControl.getBrightness( );
        p_message->underglow_brightness = LEDControl.getBrightnessUG( );
    }
    else
    {
        p_message->backlight_brightness = LEDControl.getBrightnessWireless( );
        p_message->underglow_brightness = LEDControl.getBrightnessUGWireless( );
    }
    p_message->brightness_led_effect = led_effect;
    p_message->take_control = take_brightness_control;
    packet.header.device = UNKNOWN;
    Communications.sendPacket( packet );
}

result_t LEDManager::comks_init( void )
{
    Communications.callbacks.bind(CONNECTED,
                                  ([this](Packet packet) {
                                    comks_connected( packet );
                                  }));

    Communications.callbacks.bind(RETRY_LAYERS,
                                  ([this](Packet packet) {
                                    comks_retry_layers( packet );
                                  }));

    return RESULT_OK;
}

/****************************************************/
/*             Command Event Processes              */
/****************************************************/

kbdapi_event_result_t LEDManager::command_led_process( const char * p_command )
{
    enum
    {
        MODE,
        BRIGHTNESS_WIRED,
        BRIGHTNESS_UG_WIRED,
        BRIGHTNESS_WIRELESS,
        BRIGHTNESS_UG_WIRELESS,
        FADE_EFFECT
    } subCommand;

    if ( LEDLayers.leds_count_get() == 0 ) return KBDAPI_EVENT_RESULT_IGNORED;

    if (::Focus.handleHelp(p_command, "led.mode\n"
                                    "led.brightness\n"
                                    "led.brightnessUG\n"
                                    "led.brightness.wireless\n"
                                    "led.brightnessUG.wireless\n"
                                    "led.fade\n"))
        return KBDAPI_EVENT_RESULT_IGNORED;

    if (strncmp(p_command, "led.", 4) != 0) return KBDAPI_EVENT_RESULT_IGNORED;
    if (strcmp(p_command + 4, "mode") == 0)
        subCommand = MODE;
    else if (strcmp(p_command + 4, "brightness") == 0)
        subCommand = BRIGHTNESS_WIRED;
    else if (strcmp(p_command + 4, "brightnessUG") == 0)
        subCommand = BRIGHTNESS_UG_WIRED;
    else if (strcmp(p_command + 4, "brightness.wireless") == 0)
        subCommand = BRIGHTNESS_WIRELESS;
    else if (strcmp(p_command + 4, "brightnessUG.wireless") == 0)
        subCommand = BRIGHTNESS_UG_WIRELESS;
    else if (strcmp(p_command + 4, "fade") == 0)
        subCommand = FADE_EFFECT;
    else
        return KBDAPI_EVENT_RESULT_IGNORED;

    switch (subCommand)
    {
        case BRIGHTNESS_WIRED:
        {
            if (::Focus.isEOL())
            {
                ::Focus.send( LEDControl.getBrightness() );
            }
            else
            {
                uint8_t brightness;

                ::Focus.read(brightness);
                LEDControl.setBrightness(brightness);
            }
            break;
        }
        case BRIGHTNESS_UG_WIRED:
        {
            if (::Focus.isEOL())
            {
                ::Focus.send( LEDControl.getBrightnessUG() );
            }
            else
            {
                uint8_t brightness;

                ::Focus.read(brightness);
                LEDControl.setBrightnessUG(brightness);
            }
            break;
        }
        case BRIGHTNESS_WIRELESS:
        {
            if (::Focus.isEOL())
            {
                ::Focus.send( LEDControl.getBrightnessWireless() );
            }
            else
            {
                uint8_t brightness;

                ::Focus.read(brightness);
                LEDControl.setBrightnessWireless(brightness);
            }
            break;
        }
        case BRIGHTNESS_UG_WIRELESS:
        {
            if (::Focus.isEOL())
            {
                ::Focus.send( LEDControl.getBrightnessUGWireless() );
            }
            else
            {
                uint8_t brightness;

                ::Focus.read(brightness);
                LEDControl.setBrightnessUGWireless(brightness);
            }
            break;
        }
        case FADE_EFFECT:
        {
            if (::Focus.isEOL())
            {
                ::Focus.send( LEDLayers.fade_effect_is_enabled() );
            }
            else
            {
                uint8_t fade_effect_enable;

                ::Focus.read(fade_effect_enable);

                LEDControl.fade_effect_save( fade_effect_enable );
                LEDLayers.fade_effect_setup( fade_effect_enable );
            }
        }
        case MODE:
        {
            char peek = ::Focus.peek();
            if (peek == '\n')
            {
                ::Focus.send( p_LEDEffect->type_get() );
            }
            else if (peek == 'n')
            {
                LEDControl.next_mode();
            }
            else if (peek == 'p')
            {
                LEDControl.prev_mode();
            }
            else
            {
                uint8_t mode_id;

                ::Focus.read(mode_id);
                led_effect_set( (LEDEffect::led_effect_type_t)mode_id );
            }
            break;
        }
    }

    return KBDAPI_EVENT_RESULT_CONSUMED;
}

class LEDManager LEDManager;
