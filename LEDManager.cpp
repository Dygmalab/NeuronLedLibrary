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

#include "Config_manager.h"
#include "kbd_if_manager.h"
#include "LEDManager.h"

#include "LEDEffect-BatteryStatus-Defy.h"
#include "LEDEffect-Bluetooth-Pairing-Defy.h"
#include "LEDEffect-Rainbow-Defy.h"
#include "LEDEffect-SolidColor-Defy.h"
#include "LEDEffect-Stalker-Defy.h"
#include "LEDPalette.h"

#include "Kaleidoscope-FocusSerial.h"
//#include "Kaleidoscope-EEPROM-Settings.h"

const LEDManager::LEDEffect_list_t LEDManager::LEDEffect_list_regular =
{
    &LEDLayers, &LEDRainbowEffectDefy, &LEDRainbowWaveEffectDefy, &LEDStalkerDefy,
    &solidRedDefy, &solidGreenDefy, &solidBlueDefy, &solidWhiteDefy, &solidBlackDefy,
};

const LEDManager::LEDEffect_list_t LEDManager::LEDEffect_list_specific =
{
    &LEDBatteryStatusDefy, &LEDBluetoothPairingDefy,
};

const LEDManager::idleleds_conf_t LEDManager::idleleds_conf_default =
{
    .true_sleep_enabled = false,
    .true_sleep_time_ms = 600000,       /* 10 minutes */
    .leds_off_wired_time_ms = 300000,   /*  5 minutes */
    .leds_off_wireless_time_ms = 60000, /*  1 minute  */
};

const LEDManager::brightness_conf_t LEDManager::brightness_conf_default =
{
    .wired_backlight = 255,
    .wired_underglow = 255,
    .wireless_backlight = 125,
    .wireless_underglow = 125,
    .is_valid = 0x00,                   /* The valid brightness config when is_valid == 0x00 */
};

result_t LEDManager::init( const LEDManager_config_t & config )
{
    result_t result = RESULT_ERR;

    /* Initialize the keyboard interface */
    result = kbdif_initialize( );
    EXIT_IF_ERR( result, "kbdif_initialize failed" );

    /* Initialize the Idle LEDs */
    result = idleleds_init();
    EXIT_IF_ERR( result, "idleleds_init failed" );

    /* Initialize the LEDs brightness */
    result = brightness_init();
    EXIT_IF_ERR( result, "brightness_init failed" );

    /* Initialize the keyscanner communication interface */
    result = comks_init( );
    EXIT_IF_ERR( result, "com_ks_init failed" );

    /* Initialize the led color palette */
    result = palette_init( config );
    EXIT_IF_ERR( result, "palette_init failed" );

    /* Initialize the led layers */
    result = layers_init( config );
    EXIT_IF_ERR( result, "layers_init failed" );

    /* Get the fade effect confuration */
    result = ConfigManager.config_item_request( ConfigManager::CFG_ITEM_TYPE_LEDS_FADE_EFFECT, (const void **)&p_fade_effect_conf );

    /* Check if the config is cleared */
    if( *p_fade_effect_conf == 0xFF )
    {
        cfgmem_fade_effect_config_save( 0 );
    }

    LEDLayers.fade_effect_setup( *p_fade_effect_conf );

    /* Initialize the default led effect to be the LEDLayers one */
    p_LEDEffect = &LEDLayers;

_EXIT:
    return result;
}

result_t LEDManager::palette_init( const LEDManager_config_t & config )
{
    result_t result = RESULT_ERR;

    ASSERT_DYGMA( config.p_LEDPalette != nullptr, "Missing LEDPalette instance" );

    /* Save the LED Palette instance */
    p_LEDPalette = config.p_LEDPalette;

    result = p_LEDPalette->init();
    EXIT_IF_ERR( result, "p_LEDPalette->init failed" );

_EXIT:
    return result;
}

result_t LEDManager::layers_init( const LEDManager_config_t & config )
{
    result_t result = RESULT_ERR;
    LEDLayers::LEDLayers_config_t layers_config;

    /* Prepare the LEDLayers configuration */
    layers_config.p_LEDDevice_list = config.p_LEDDevice_list;
    layers_config.layers_count = config.layers_count;

    /* Initialize the led layers */
    result = LEDLayers.init( layers_config );
    EXIT_IF_ERR( result, "LEDLayers.init failed" );

_EXIT:
    return result;
}

/****************************************************/
/*                   Config Memory                  */
/****************************************************/

void LEDManager::cfgmem_fade_effect_config_save( uint8_t fade_effect )
{
    result_t result = RESULT_ERR;

    ConfigManager.config_item_update( p_fade_effect_conf, &fade_effect, sizeof( uint8_t) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void LEDManager::cfgmem_idleleds_config_save( const idleleds_conf_t * p_new_conf )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( p_idleleds_conf, p_new_conf, sizeof( idleleds_conf_t) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void LEDManager::cfgmem_idleleds_true_sleep_enabled_save( bool_t true_sleep_enabled )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_idleleds_conf->true_sleep_enabled, &true_sleep_enabled, sizeof( p_idleleds_conf->true_sleep_enabled) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void LEDManager::cfgmem_idleleds_true_sleep_time_ms_save( uint32_t true_sleep_time_ms )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_idleleds_conf->true_sleep_time_ms, &true_sleep_time_ms, sizeof( p_idleleds_conf->true_sleep_time_ms) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void LEDManager::cfgmem_idleleds_leds_off_wired_time_ms_save( uint32_t leds_off_wired_time_ms )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_idleleds_conf->leds_off_wired_time_ms, &leds_off_wired_time_ms, sizeof( p_idleleds_conf->leds_off_wired_time_ms) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void LEDManager::cfgmem_idleleds_leds_off_wireless_time_ms_save( uint32_t leds_off_wireless_time_ms )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_idleleds_conf->leds_off_wireless_time_ms, &leds_off_wireless_time_ms, sizeof( p_idleleds_conf->leds_off_wireless_time_ms) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void LEDManager::cfgmem_brightness_config_save( const brightness_conf_t * p_new_conf )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( p_brightness_conf, p_new_conf, sizeof( brightness_conf_t) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void LEDManager::cfgmem_brightness_wired_backlight_save( uint8_t wired_backlight )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_brightness_conf->wired_backlight, &wired_backlight, sizeof( p_brightness_conf->wired_backlight) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void LEDManager::cfgmem_brightness_wired_underglow_save( uint8_t wired_underglow )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_brightness_conf->wired_underglow, &wired_underglow, sizeof( p_brightness_conf->wired_underglow) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void LEDManager::cfgmem_brightness_wireless_backlight_save( uint8_t wireless_backlight )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_brightness_conf->wireless_backlight, &wireless_backlight, sizeof( p_brightness_conf->wireless_backlight) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void LEDManager::cfgmem_brightness_wireless_underglow_save( uint8_t wireless_underglow )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_brightness_conf->wireless_underglow, &wireless_underglow, sizeof( p_brightness_conf->wireless_underglow) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

/****************************************************/
/*                    LED Control                   */
/****************************************************/

void LEDManager::leds_enable( void )
{
    leds_enabled_flag = true;

    update_brightness( BRIGHTNESS_LED_EFFECT_NONE, true );
}

void LEDManager::leds_disable( void )
{
    leds_enabled_flag = false;

    update_brightness( BRIGHTNESS_LED_EFFECT_NONE, true );
}

void LEDManager::leds_toggle( void )
{
    if( leds_enabled_flag == true )
    {
        leds_disable();
    }
    else
    {
        leds_enable();
    }
}

bool_t LEDManager::leds_enabled( void )
{
    return leds_enabled_flag;
}

void LEDManager::com_mode_set( bool_t wireless )
{
    bool_t com_mode_changed = ( com_mode_wired_flag != wireless ) ? true : false;

    com_mode_wired_flag = wireless;

    if( com_mode_changed == true )
    {
        idleleds_reset();
    }
}

void LEDManager::com_new_connection_set( void )
{
    idleleds_new_connection = true;
}

void LEDManager::update_brightness( brightness_led_effect_t led_effect, bool_t take_brightness_control )
{
    /*
     * NOTE: Instead of directly sending the brightness message, we set a timeout for which we wait if there come other
     *       brightness requests. This is for preventing possible led flickering due to high-rate brightness changes.
     */

    /* Save the brightness control flags */
    brightness_led_effect = led_effect;
    brightness_take_control = take_brightness_control;

    /* Request the Update of the brightness after the timeout elapses */
    timer_set_ms( &brightness_update_timer, BRIGHTNESS_UPDATE_TIMEOUT_MS );
    brightness_update_flag = true;
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

void LEDManager::led_effect_set_type( LEDEffect::led_effect_type_t effect_type, bool_t prio )
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
    if( leds_enabled_flag == false )
    {
        return;
    }

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
    if( leds_enabled_flag == false )
    {
        return;
    }

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

kbdapi_event_result_t LEDManager::kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key )
{
    LEDManager * p_LEDManager = ( LEDManager *)p_instance;

    if ( p_LEDManager->idleleds_state == IDLELEDS_STATE_OFF )
    {
        p_LEDManager->leds_enable();
    }

    p_LEDManager->idleleds_reset_timer();

    return KBDAPI_EVENT_RESULT_IGNORED;
}

kbdapi_event_result_t LEDManager::kbdif_command_event_cb( void * p_instance, const char * p_command )
{
    LEDManager * p_LEDManager = ( LEDManager *)p_instance;
    kbdapi_event_result_t result = KBDAPI_EVENT_RESULT_IGNORED;

    result = p_LEDManager->command_led_process( p_command );
    EXIT_IF_KBDAPI_NOT_IGNORED( result );

    result = p_LEDManager->command_idleleds_process( p_command );
    EXIT_IF_KBDAPI_NOT_IGNORED( result );

    result = p_LEDManager->p_LEDPalette->command_process( p_command );
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

            p_LEDManager->leds_disable();

            break;

        case KBDAPI_LED_EFFECT_ACTION_ENABLE:

            p_LEDManager->leds_enable();

            break;

        case KBDAPI_LED_EFFECT_ACTION_TOGGLE:

            p_LEDManager->leds_toggle();

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
    .key_event_cb = kbdif_key_event_cb,
    .command_event_cb = kbdif_command_event_cb,
    .led_layer_change_event_cb = kbdif_led_layer_change_event_cb,
    .led_effect_change_event_cb = kbdif_led_effect_change_event_cb,
};

/****************************************************/
/*             Keyscanner Communication             */
/****************************************************/

void LEDManager::comks_connected( Packet packet )
{
    p_LEDPalette->update_palette( packet );
    LEDLayers.update_map_backlight( packet );
    LEDLayers.update_map_underglow( packet );
    led_effect_refresh();
}

void LEDManager::comks_retry_layers( Packet packet )
{
    p_LEDPalette->update_palette( packet );
    LEDLayers.update_map_backlight( packet );
    LEDLayers.update_map_underglow( packet );
    led_effect_refresh();
}

void LEDManager::comks_update_brightness( void )
{
    Packet packet;
    packet.header.command = BRIGHTNESS;
    packet.header.size = 4;
    brightness_message_t * p_message = (brightness_message_t *)packet.data;

    if ( leds_enabled_flag == false )
    {
        p_message->backlight_brightness = 0;
        p_message->underglow_brightness = 0;
    }
    else if ( com_mode_wired_flag == true )
    {
        p_message->backlight_brightness = p_brightness_conf->wired_backlight;
        p_message->underglow_brightness = p_brightness_conf->wired_underglow;
    }
    else
    {
        p_message->backlight_brightness = p_brightness_conf->wireless_backlight;
        p_message->underglow_brightness = p_brightness_conf->wireless_underglow;
    }
    p_message->brightness_led_effect = brightness_led_effect;
    p_message->take_control = brightness_take_control;
    packet.header.device = UNKNOWN;
    Communications.sendPacket( packet );
}

void LEDManager::comks_sleep_send( void )
{
    Packet packet;

    packet.header.command = SLEEP;
    packet.header.device = UNKNOWN;
    packet.header.size = 0;

    Communications.sendPacket( packet );
}


result_t LEDManager::comks_init( void )
{
    Communications.callbacks.bind(CONNECTED,
                                  ([this](Packet packet) {
                                    comks_connected( packet );
                                    idleleds_connected_handle();
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
                ::Focus.send( p_brightness_conf->wired_backlight );
            }
            else
            {
                uint8_t brightness;

                ::Focus.read(brightness);
                cfgmem_brightness_wired_backlight_save( brightness );

                update_brightness( LEDManager::BRIGHTNESS_LED_EFFECT_NONE, true );
            }
            break;
        }
        case BRIGHTNESS_UG_WIRED:
        {
            if (::Focus.isEOL())
            {
                ::Focus.send( p_brightness_conf->wired_underglow );
            }
            else
            {
                uint8_t brightness;

                ::Focus.read(brightness);
                cfgmem_brightness_wired_underglow_save( brightness );

                update_brightness( LEDManager::BRIGHTNESS_LED_EFFECT_NONE, true );
            }
            break;
        }
        case BRIGHTNESS_WIRELESS:
        {
            if (::Focus.isEOL())
            {
                ::Focus.send( p_brightness_conf->wireless_backlight );
            }
            else
            {
                uint8_t brightness;

                ::Focus.read(brightness);
                cfgmem_brightness_wireless_backlight_save( brightness );

                update_brightness( LEDManager::BRIGHTNESS_LED_EFFECT_NONE, true );
            }
            break;
        }
        case BRIGHTNESS_UG_WIRELESS:
        {
            if (::Focus.isEOL())
            {
                ::Focus.send( p_brightness_conf->wireless_underglow );
            }
            else
            {
                uint8_t brightness;

                ::Focus.read(brightness);
                cfgmem_brightness_wireless_underglow_save( brightness );

                update_brightness( LEDManager::BRIGHTNESS_LED_EFFECT_NONE, true );
            }
            break;
        }
        case FADE_EFFECT:
        {
            if (::Focus.isEOL())
            {
                uint8_t fade_effect_enable = LEDLayers.fade_effect_is_enabled();

                ::Focus.send( fade_effect_enable );
            }
            else
            {
                uint8_t fade_effect_enable;

                ::Focus.read(fade_effect_enable);

                cfgmem_fade_effect_config_save( fade_effect_enable );

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
                led_effect_set_next();
            }
            else if (peek == 'p')
            {
                led_effect_set_previous();
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

kbdapi_event_result_t LEDManager::command_idleleds_process( const char * p_command )
{
    result_t result = RESULT_ERR;

    /*
        idleleds.time_limit         --> Set power off time for LEDs, when the n2 is in USB mode [seconds].
        idleleds.wireless           --> Set power off time for LEDs, when the n2 is in BLE mode [seconds].
        idleleds.true_sleep         --> Activate/Deactivate put to sleep the keyboard sides [bool].
        idleleds.true_sleep_time    --> Set the time to put sleep the keyboard sides [seconds].
     */

    if (::Focus.handleHelp(p_command, "idleleds.true_sleep\nidleleds.true_sleep_time\nidleleds.time_limit\nidleleds.wireless"))
    {
        return KBDAPI_EVENT_RESULT_IGNORED;
    }

    if (strncmp(p_command, "idleleds.", 9) != 0)
    {
        return KBDAPI_EVENT_RESULT_IGNORED;
    }

    if (strcmp(p_command + 9, "true_sleep") == 0)
    {
        if (::Focus.isEOL())
        {
            uint8_t enabled = ( p_idleleds_conf->true_sleep_enabled == true ) ? 1 : 0;
            ::Focus.send( enabled );
        }
        else
        {
            uint8_t enabled;
            bool_t true_sleep_enabled;
            ::Focus.read(enabled);

            true_sleep_enabled = ( enabled == 0 ) ? false : true;
            cfgmem_idleleds_true_sleep_enabled_save( true_sleep_enabled );
        }
    }

    if (strcmp(p_command + 9, "true_sleep_time") == 0)
    {
        if (::Focus.isEOL())
        {
            uint16_t true_sleep_time_sec = p_idleleds_conf->true_sleep_time_ms / 1000;   /* Convert from ms to seconds. */
            ::Focus.send( true_sleep_time_sec );
        }
        else
        {
            uint16_t true_sleep_time_sec;
            uint32_t true_sleep_time_ms;
            ::Focus.read(true_sleep_time_sec);

            true_sleep_time_ms = true_sleep_time_sec * 1000;  /* Convert from seconds to ms. */
            cfgmem_idleleds_true_sleep_time_ms_save( true_sleep_time_ms );
        }
    }

    if (strcmp(p_command + 9, "time_limit") == 0)
    {
        if (::Focus.isEOL())
        {
            uint16_t leds_off_wired_time_sec = p_idleleds_conf->leds_off_wired_time_ms / 1000;    /* Convert from ms to seconds. */
            ::Focus.send( leds_off_wired_time_sec );
        }
        else
        {
            uint16_t leds_off_wired_time_sec;
            uint32_t leds_off_wired_time_ms;
            ::Focus.read(leds_off_wired_time_sec);

            leds_off_wired_time_ms = leds_off_wired_time_sec * 1000;  /* Convert from seconds to ms. */
            cfgmem_idleleds_leds_off_wired_time_ms_save( leds_off_wired_time_ms );
        }
    }

    if (strcmp(p_command + 9, "wireless") == 0)
    {
        if (::Focus.isEOL())
        {
            uint16_t leds_off_wireless_time_sec = p_idleleds_conf->leds_off_wireless_time_ms / 1000;    /* Convert from ms to seconds. */
            ::Focus.send( leds_off_wireless_time_sec );
        }
        else
        {
            uint16_t leds_off_wireless_time_sec;
            uint32_t leds_off_wireless_time_ms;
            ::Focus.read(leds_off_wireless_time_sec);

            leds_off_wireless_time_ms = leds_off_wireless_time_sec * 1000; /* Convert from seconds to ms. */
            cfgmem_idleleds_leds_off_wireless_time_ms_save( leds_off_wireless_time_ms );
        }
        return KBDAPI_EVENT_RESULT_CONSUMED;
    }

    return KBDAPI_EVENT_RESULT_CONSUMED;

    UNUSED(result);
}

/****************************************************/
/*                    Idle Leds                     */
/****************************************************/

INLINE result_t LEDManager::idleleds_init( void )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_request( ConfigManager::CFG_ITEM_TYPE_LEDS_IDLELEDS, (const void **)&p_idleleds_conf );
    EXIT_IF_ERR( result, "ConfigManager.config_item_request failed" );

    /* Check if the config is cleared */
    if( (uint8_t)p_idleleds_conf->true_sleep_enabled == 0xFF )
    {
        cfgmem_idleleds_config_save( &idleleds_conf_default );
    }

    /* After startup, we are in OFF state, waiting for an external event */
    idleleds_state_set_off( false );

_EXIT:
    return result;
}

INLINE void LEDManager::idleleds_reset( void )
{
    /* After reset, we set the OFF state. */
    idleleds_state_set_off( false );
}

INLINE void LEDManager::idleleds_reset_timer( void )
{
    timer_set_ms( &idleleds_timer, idleleds_timeout_ms );
}

INLINE void LEDManager::idleleds_connected_handle( void )
{
    /* Handle the new connection situation */
    if( idleleds_new_connection == true )
    {
        idleleds_new_connection = false;
        idleleds_reset_timer();
    }
}

INLINE void LEDManager::idleleds_state_set( idleleds_state_t state )
{
    idleleds_state = state;
}

void LEDManager::idleleds_state_set_on( void )
{
    idleleds_leds_off_enabled = false;
    idleleds_timeout_ms = 0;

    if( com_mode_wired_flag == true && p_idleleds_conf->leds_off_wired_time_ms != 0 )
    {
        idleleds_leds_off_enabled = true;
        idleleds_timeout_ms = p_idleleds_conf->leds_off_wired_time_ms;
    }
    else if( com_mode_wired_flag == false && p_idleleds_conf->leds_off_wireless_time_ms != 0 )
    {
        idleleds_leds_off_enabled = true;
        idleleds_timeout_ms = p_idleleds_conf->leds_off_wireless_time_ms;
    }

    /* Set the idle leds timeout and move to the ON state */
    timer_set_ms( &idleleds_timer, idleleds_timeout_ms );
    idleleds_state_set( IDLELEDS_STATE_ON );
}

void LEDManager::idleleds_state_set_off( bool_t forced )
{
    idleleds_state_t state_off_type = ( forced == true ) ? IDLELEDS_STATE_OFF_FORCED : IDLELEDS_STATE_OFF;

    /* In wired mode, the true sleep is not set */
    idleleds_true_sleep_enabled = ( com_mode_wired_flag == false ) ? p_idleleds_conf->true_sleep_enabled : false;
    idleleds_timeout_ms = p_idleleds_conf->true_sleep_time_ms;

    /* Set the true sleep timeout and move to the OFF state */
    timer_set_ms( &idleleds_timer, idleleds_timeout_ms );
    idleleds_state_set( state_off_type );
}

INLINE void LEDManager::idleleds_state_on( void )
{
    if( leds_enabled_flag == false )
    {
        /* The LEDs were disabled externally to this state machine - immediately move to the forced OFF state */
        idleleds_state_set_off( true );
        return;
    }
    else if( idleleds_leds_off_enabled == false || timer_check( &idleleds_timer ) == false )
    {
        return;
    }

    /* Disable the leds */
    leds_disable();

    /* Move to the OFF state */
    idleleds_state_set_off( false );
}

INLINE void LEDManager::idleleds_state_off( void )
{
    if( leds_enabled_flag == true )
    {
        /* Move to the ON state */
        idleleds_state_set_on();
        return;
    }
    else if( idleleds_true_sleep_enabled == false || timer_check( &idleleds_timer ) == false )
    {
        return;
    }

    /* Send the True Sleep command */
    comks_sleep_send();

    idleleds_state_set( IDLELEDS_STATE_TRUE_SLEEP );
}

INLINE void LEDManager::idleleds_state_true_sleep( void )
{
    if( leds_enabled_flag == false )
    {
        return;
    }

    /* Move to the ON state */
    idleleds_state_set_on();
}

INLINE void LEDManager::idleleds_machine( void )
{
    switch( idleleds_state )
    {

        case IDLELEDS_STATE_ON:

            idleleds_state_on();

            break;

        case IDLELEDS_STATE_OFF:
        case IDLELEDS_STATE_OFF_FORCED:

            idleleds_state_off();

            break;

        case IDLELEDS_STATE_TRUE_SLEEP:

            idleleds_state_true_sleep();

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled Idle LEDs state" );

            break;
    }
}

/****************************************************/
/*               LED Brightness Leds                */
/****************************************************/

INLINE result_t LEDManager::brightness_init( void )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_request( ConfigManager::CFG_ITEM_TYPE_LEDS_BRIGHTNESS, (const void **)&p_brightness_conf );
    EXIT_IF_ERR( result, "ConfigManager.config_item_request failed" );

    /* Check if the config is cleared */
    if( p_brightness_conf->is_valid != 0xFF )
    {
        cfgmem_brightness_config_save( &brightness_conf_default );
    }

_EXIT:
    return result;
}

/****************************************************/
/*                     Machine                      */
/****************************************************/

INLINE void LEDManager::machine_state_set( led_manager_state_t state )
{
    machine_state = state;
}

INLINE void LEDManager::machine_state_idle( void )
{
    if( brightness_update_flag == true && timer_check( &brightness_update_timer ) == true )
    {
        brightness_update_flag = false;
        machine_state_set( LED_MANAGER_STATE_BRIGHTNESS_UPDATE );
    }
}

INLINE void LEDManager::machine_state_brightness_update( void )
{
    /* Update the brightness and return to the IDLE state */
    comks_update_brightness();
    machine_state_set( LED_MANAGER_STATE_IDLE );
}

INLINE void LEDManager::machine( void )
{
    switch( machine_state )
    {
        case LED_MANAGER_STATE_IDLE:

            machine_state_idle();

            break;

        case LED_MANAGER_STATE_BRIGHTNESS_UPDATE:

            machine_state_brightness_update();

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled led_manager_state_t state" );

            break;
    }
}

void LEDManager::run( void )
{
    idleleds_machine();
    machine();
}

class LEDManager LEDManager;
