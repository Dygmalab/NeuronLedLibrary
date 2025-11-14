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

#pragma once

#include <vector>

#include "kbd_if.h"
#include "LEDEffect.h"
#include "LEDLayers.h"
#include "LEDPalette.h"
#include "LEDTypes.h"
#include "Time_counter.h"

class LEDManager {

  public:

    typedef enum : uint8_t
    {
        BRIGHTNESS_LED_EFFECT_NONE = 0,
        /* Currently unused enums. Keeping them because they were specified in past and (possibly) used in communications. Their values
         * are setting the enum values of still-valid BATTERY_STATUS and BT_LED_EFFECT. */
//        BRIGHTNESS_LED_EFFECT_BATTERY_MANAGMENT = 1,
//        BRIGHTNESS_LED_EFFECT_FADE_EFFECT = 2,
        BRIGHTNESS_LED_EFFECT_BATTERY_STATUS = 3,
        BRIGHTNESS_LED_EFFECT_BT_LED_EFFECT = 4,
    }brightness_led_effect_t;

    typedef struct
    {
        /* Palette */
        LEDPalette * p_LEDPalette;               /* The instance of the palette for the current project */

        /* Layers and Colormap */
        const LEDDevice_list_t * p_LEDDevice_list;  /* List of LED controlled devices */
        uint8_t layers_count;                       /* Number of layers to be operated */
    } LEDManager_config_t;

    typedef struct PACK
    {
        bool_t true_sleep_enabled;          /* Flag signaling if the true sleep is enabled */
        uint8_t reserve[3];                 /* Reserve space to fit the legacy size in the memory */
        uint32_t true_sleep_time_ms;        /* Timeout in miliseconds until the device goes to sleep */
        uint32_t leds_off_wired_time_ms;    /* Timeout in miliseconds until the device switches off the leds when working wired */
        uint32_t leds_off_wireless_time_ms; /* Timeout in miliseconds until the device switches off the leds when working wireless */
    } idleleds_conf_t;

  public:

    result_t init( const LEDManager_config_t & config );
    result_t palette_init( const LEDManager_config_t & config );
    result_t layers_init( const LEDManager_config_t & config );

    void com_mode_set( bool_t wired );
    void com_new_connection_set( void );
    void update_brightness( brightness_led_effect_t led_effect, bool_t take_brightness_control );

    void leds_enable( void );
    void leds_disable( void );
    void leds_toggle( void );
    bool_t leds_enabled( void );

    void led_effect_set( LEDEffect::led_effect_type_t effect_type );
    void led_effect_set_prio( LEDEffect::led_effect_type_t effect_type );
    void led_effect_reset_prio( void );
    void led_effect_refresh( void );

    void run( void );

  private:

    typedef struct
    {
        uint8_t backlight_brightness;
        uint8_t underglow_brightness;
        brightness_led_effect_t brightness_led_effect;
        uint8_t take_control;                       /* Tell KS that we want to take or give brightness control. */
    } PACK brightness_message_t;

    typedef int16_t led_effect_id_t;
    typedef std::vector<LEDEffect*> LEDEffect_list_t;

  private:

    const uint8_t * p_fade_effect_conf;

    kbdif_t * p_kbdif = nullptr;
    LEDPalette * p_LEDPalette = nullptr;

#warning "These will be used when the EEPROM issues are solved"
//    uint8_t brightness_bl_wired = 0;
//    uint8_t brightness_bl_wireless = 0;
//    uint8_t brightness_ug_wired = 0;
//    uint8_t brightness_ug_wireless = 0;

    LEDEffect * p_LEDEffect = nullptr;
    LEDEffect * p_LEDEffect_prio = nullptr;         /* This is a top priority effect which is forcefully being active until set to null */
    led_effect_id_t LEDEffect_id_regular = 0;

    bool_t leds_enabled_flag = false;
    bool_t com_mode_wired_flag = true;

    brightness_led_effect_t brightness_led_effect = BRIGHTNESS_LED_EFFECT_NONE;
    bool_t brightness_take_control = false;

    result_t kbdif_initialize( void );

    void cfgmem_fade_effect_config_save( uint8_t fade_effect );
    void cfgmem_idleleds_config_save( const idleleds_conf_t * p_idleleds_conf );
    void cfgmem_idleleds_true_sleep_enabled_save( bool_t true_sleep_enabled );
    void cfgmem_idleleds_true_sleep_time_ms_save( uint32_t true_sleep_time_ms );
    void cfgmem_idleleds_leds_off_wired_time_ms_save( uint32_t leds_off_wired_time_ms );
    void cfgmem_idleleds_leds_off_wireless_time_ms_save( uint32_t leds_off_wireless_time_ms );

    result_t comks_init( void );
    void comks_connected( Packet packet );
    void comks_retry_layers( Packet packet );
    void comks_update_brightness( void );
    void comks_sleep_send( void );

    LEDEffect * led_effect_search_type( const LEDEffect_list_t & effect_list, LEDEffect::led_effect_type_t effect_type );
    LEDEffect * led_effect_search_type( LEDEffect::led_effect_type_t effect_type );

    void led_effect_set_type( LEDEffect::led_effect_type_t effect_type, bool_t prio );
    void led_effect_set_id( led_effect_id_t id );
    void led_effect_set_next( void );
    void led_effect_set_previous( void );

    LEDEffect::led_effect_type_t led_effect_get_active_type( void );
    void led_effect_activate( void );

    void led_layer_set( kbdapi_led_layer_id_t layer_id );

    kbdapi_event_result_t command_led_process( const char * p_command );
    kbdapi_event_result_t command_idleleds_process( const char * p_command );


    /****************************************************/
    /*                    Idle Leds                     */
    /****************************************************/

  private:

    typedef enum
    {
        IDLELEDS_STATE_ON = 1,
        IDLELEDS_STATE_OFF,
        IDLELEDS_STATE_OFF_FORCED,
        IDLELEDS_STATE_TRUE_SLEEP,
    } idleleds_state_t;

    static constexpr idleleds_conf_t idleleds_conf_default =
    {
        .true_sleep_enabled = false,
        .true_sleep_time_ms = 600000,       /* 10 minutes */
        .leds_off_wired_time_ms = 300000,   /*  5 minutes */
        .leds_off_wireless_time_ms = 60000, /*  1 minute  */
    };

    const idleleds_conf_t * p_idleleds_conf;
    idleleds_state_t idleleds_state = IDLELEDS_STATE_OFF;
    bool_t idleleds_true_sleep_enabled = false;
    bool_t idleleds_leds_off_enabled = false;
    bool_t idleleds_new_connection = false;

    dl_timer_t idleleds_timer = 0;
    uint32_t idleleds_timeout_ms = 0;

    INLINE result_t idleleds_init( void );
    INLINE void idleleds_reset( void );
    INLINE void idleleds_reset_timer( void );
    INLINE void idleleds_connected_handle( void );

    INLINE void idleleds_state_set( idleleds_state_t state );
    void idleleds_state_set_on( void );
    void idleleds_state_set_off( bool_t forced );

    INLINE void idleleds_state_on( void );
    INLINE void idleleds_state_off( void );
    INLINE void idleleds_state_true_sleep( void );
    INLINE void idleleds_machine( void );

    /****************************************************/
    /*                     Machine                      */
    /****************************************************/

  private:

#define BRIGHTNESS_UPDATE_TIMEOUT_MS    20

    typedef enum
    {
        LED_MANAGER_STATE_IDLE = 1,
        LED_MANAGER_STATE_BRIGHTNESS_UPDATE,
    } led_manager_state_t;

    led_manager_state_t machine_state = LED_MANAGER_STATE_IDLE;
    dl_timer_t brightness_update_timer = 0;

    bool_t brightness_update_flag = false;

    INLINE void machine_state_set( led_manager_state_t state );
    INLINE void machine_state_idle( void );
    INLINE void machine_state_brightness_update( void );
    INLINE void machine( void );

  private:

    static const kbdif_handlers_t kbdif_handlers;
    static const LEDEffect_list_t LEDEffect_list_regular;
    static const LEDEffect_list_t LEDEffect_list_specific;

    static kbdapi_event_result_t kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key );
    static kbdapi_event_result_t kbdif_command_event_cb( void * p_instance, const char * p_command );
    static kbdapi_event_result_t kbdif_led_layer_change_event_cb( void * p_instance, kbdapi_led_layer_id_t layer_id );
    static kbdapi_event_result_t kbdif_led_effect_change_event_cb( void * p_instance, kbdapi_led_effect_action_t action );
};

extern class LEDManager LEDManager;
