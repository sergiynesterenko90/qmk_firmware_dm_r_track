/* Copyright 2020 Christopher Courtney <drashna@live.com> (@drashna)
 * 2020 Qurn
 * 2022 Pascal Jaeger / Schievel https://github.com/Schievel1/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tractyl_manuform.h"
#include "transactions.h"
#include <string.h>

#ifdef CONSOLE_ENABLE
#    include "print.h"
#endif  // CONSOLE_ENABLE

#ifdef POINTING_DEVICE_ENABLE
#    ifndef CHARYBDIS_MINIMUM_DEFAULT_DPI
#        define CHARYBDIS_MINIMUM_DEFAULT_DPI 400
#    endif  // CHARYBDIS_MINIMUM_DEFAULT_DPI

#    ifndef CHARYBDIS_DEFAULT_DPI_CONFIG_STEP
#        define CHARYBDIS_DEFAULT_DPI_CONFIG_STEP 200
#    endif  // CHARYBDIS_DEFAULT_DPI_CONFIG_STEP

#    ifndef CHARYBDIS_MINIMUM_SNIPING_DPI
#        define CHARYBDIS_MINIMUM_SNIPING_DPI 200
#    endif  // CHARYBDIS_MINIMUM_SNIPER_MODE_DPI

#    ifndef CHARYBDIS_SNIPING_DPI_CONFIG_STEP
#        define CHARYBDIS_SNIPING_DPI_CONFIG_STEP 100
#    endif  // CHARYBDIS_SNIPING_DPI_CONFIG_STEP

// Fixed DPI for drag-scroll.
#    ifndef CHARYBDIS_DRAGSCROLL_DPI
#        define CHARYBDIS_DRAGSCROLL_DPI 100
#    endif  // CHARYBDIS_DRAGSCROLL_DPI

#    ifndef CHARYBDIS_DRAGSCROLL_BUFFER_SIZE
#        define CHARYBDIS_DRAGSCROLL_BUFFER_SIZE 6
#    endif  // !CHARYBDIS_DRAGSCROLL_BUFFER_SIZE

// Fixed DPI for carret.
#    ifndef CHARYBDIS_CARRET_BUFFER
#        define CHARYBDIS_CARRET_BUFFER 40
#    endif  // CHARYBDIS_CARRET_BUFFER

#    ifndef CHARYBDIS_POINTER_ACCELERATION_FACTOR
#        define CHARYBDIS_POINTER_ACCELERATION_FACTOR 24
#    endif  // !CHARYBDIS_POINTER_ACCELERATION_FACTOR

// default keycodes for the custom mode
#    ifndef CUSTOM_FN_RIGHT
#        define CUSTOM_FN_RIGHT KC_BRIGHTNESS_UP
#    endif  // CUSTOM_FN_RIGHT

#    ifndef CUSTOM_FN_LEFT
#        define CUSTOM_FN_LEFT KC_BRIGHTNESS_DOWN
#    endif  // CUSTOM_FN_LEFT

#    ifndef CUSTOM_FN_UP
#        define CUSTOM_FN_UP KC_AUDIO_VOL_UP
#    endif  // CUSTOM_FN_UP

#    ifndef CUSTOM_FN_DOWN
#        define CUSTOM_FN_DOWN KC_AUDIO_VOL_DOWN
#    endif  // CUSTOM_FN_DOWN

typedef union {
    uint8_t raw;
    struct {
        uint8_t pointer_default_dpi : 8;  // 16 steps available.
        uint8_t pointer_sniping_dpi : 1;  // 4 steps available.
        bool    is_dragscroll_enabled : 1;
        bool    is_sniping_enabled : 1;
		bool    is_carret_enabled : 1;
		bool    is_custom_enabled : 1;
		bool    is_modemode_enabled : 1;
		bool    is_integ_enabled : 1;
    } __attribute__((packed));
} charybdis_config_t;

static charybdis_config_t g_charybdis_config = {0};

/**
 * \brief Set the value of `config` from EEPROM.
 *
 * Note that `is_dragscroll_enabled`, `is_sniping_enabled` etc. are purposefully
 * ignored since we do not want to persist this state to memory.  In practice,
 * this state is always written to maximize write-performances.  Therefore, we
 * explicitly set them to `false` in this function.
 */
static void read_charybdis_config_from_eeprom(charybdis_config_t* config) {
    config->raw                   = eeconfig_read_kb() & 0xff;
    config->is_dragscroll_enabled = false;
    config->is_sniping_enabled    = false;
    config->is_carret_enabled     = false;
    config->is_custom_enabled     = false;
    config->is_modemode_enabled   = false;
    config->is_integ_enabled      = false;
}

/**
 * \brief Save the value of `config` to eeprom.
 *
 * Note that all values are written verbatim, including whether drag-scroll,
 * sniper mode, carret mode or integration mode are enabled.
 * `read_charybdis_config_from_eeprom(…)` resets these values to `false`
 * since it does not make sense to persist these across reboots of the board.
 */
static void write_charybdis_config_to_eeprom(charybdis_config_t* config) { eeconfig_update_kb(config->raw); }

/** \brief Return the current value of the pointer's default DPI. */
static uint16_t get_pointer_default_dpi(charybdis_config_t* config) { return (uint16_t)config->pointer_default_dpi * CHARYBDIS_DEFAULT_DPI_CONFIG_STEP + CHARYBDIS_MINIMUM_DEFAULT_DPI; }

/** \brief Return the current value of the pointer's sniper-mode DPI. */
static uint16_t get_pointer_sniping_dpi(charybdis_config_t* config) { return (uint16_t)config->pointer_sniping_dpi * CHARYBDIS_SNIPING_DPI_CONFIG_STEP + CHARYBDIS_MINIMUM_SNIPING_DPI; }

/** \brief Set the appropriate DPI for the input config. */
static void maybe_update_pointing_device_cpi(charybdis_config_t* config) {
    if (config->is_dragscroll_enabled) {
			pointing_device_set_cpi(CHARYBDIS_DRAGSCROLL_DPI);
    } else if (config->is_sniping_enabled) {
			pointing_device_set_cpi(get_pointer_sniping_dpi(config));
    } else {
			pointing_device_set_cpi(get_pointer_default_dpi(config));
    }
}

/**
 * \brief Update the pointer's default DPI to the next or previous step.
 *
 * Increases the DPI value if `forward` is `true`, decreases it otherwise.
 * The increment/decrement steps are equal to CHARYBDIS_DEFAULT_DPI_CONFIG_STEP.
 */
static void step_pointer_default_dpi(charybdis_config_t* config, bool forward) {
    config->pointer_default_dpi += forward ? 1 : -1;
    maybe_update_pointing_device_cpi(config);
}

/**
 * \brief Update the pointer's sniper-mode DPI to the next or previous step.
 *
 * Increases the DPI value if `forward` is `true`, decreases it otherwise.
 * The increment/decrement steps are equal to CHARYBDIS_SNIPING_DPI_CONFIG_STEP.
 */
static void step_pointer_sniping_dpi(charybdis_config_t* config, bool forward) {
    config->pointer_sniping_dpi += forward ? 1 : -1;
    maybe_update_pointing_device_cpi(config);
}

uint16_t charybdis_get_pointer_default_dpi(void) { return get_pointer_default_dpi(&g_charybdis_config); }

uint16_t charybdis_get_pointer_sniping_dpi(void) { return get_pointer_sniping_dpi(&g_charybdis_config); }

void charybdis_cycle_pointer_default_dpi_noeeprom(bool forward) { step_pointer_default_dpi(&g_charybdis_config, forward); }

void charybdis_cycle_pointer_default_dpi(bool forward) {
    step_pointer_default_dpi(&g_charybdis_config, forward);
    write_charybdis_config_to_eeprom(&g_charybdis_config);
}

void charybdis_cycle_pointer_sniping_dpi_noeeprom(bool forward) { step_pointer_sniping_dpi(&g_charybdis_config, forward); }

void charybdis_cycle_pointer_sniping_dpi(bool forward) {
    step_pointer_sniping_dpi(&g_charybdis_config, forward);
    write_charybdis_config_to_eeprom(&g_charybdis_config);
}

bool charybdis_get_pointer_sniping_enabled(void) { return g_charybdis_config.is_sniping_enabled; }

void charybdis_set_pointer_sniping_enabled(bool enable) {
    g_charybdis_config.is_sniping_enabled = enable;
    maybe_update_pointing_device_cpi(&g_charybdis_config);
}

bool charybdis_get_pointer_dragscroll_enabled(void) { return g_charybdis_config.is_dragscroll_enabled; }

void charybdis_set_pointer_dragscroll_enabled(bool enable) {
	charybdis_set_pointer_disable_nonstacking();
    g_charybdis_config.is_dragscroll_enabled = enable;
    maybe_update_pointing_device_cpi(&g_charybdis_config);
}

bool charybdis_get_pointer_carret_enabled(void) { return g_charybdis_config.is_carret_enabled; }

void charybdis_set_pointer_carret_enabled(bool enable) {
	charybdis_set_pointer_disable_nonstacking();
    g_charybdis_config.is_carret_enabled = enable;
    maybe_update_pointing_device_cpi(&g_charybdis_config);
}

bool charybdis_get_pointer_custom_enabled(void) { return g_charybdis_config.is_custom_enabled; }

void charybdis_set_pointer_custom_enabled(bool enable) {
	charybdis_set_pointer_disable_nonstacking();
    g_charybdis_config.is_custom_enabled = enable;
    maybe_update_pointing_device_cpi(&g_charybdis_config);
}

bool charybdis_get_pointer_modemode_enabled(void) { return g_charybdis_config.is_modemode_enabled; }

void charybdis_set_pointer_modemode_enabled(bool enable) {
    g_charybdis_config.is_modemode_enabled = enable;
    maybe_update_pointing_device_cpi(&g_charybdis_config);
}

bool charybdis_get_pointer_integ_enabled(void) { return g_charybdis_config.is_integ_enabled; }

void charybdis_set_pointer_integ_enabled(bool enable) {
    g_charybdis_config.is_integ_enabled = enable;
    maybe_update_pointing_device_cpi(&g_charybdis_config);
}

void charybdis_set_pointer_disable_nonstacking(void) {
    g_charybdis_config.is_dragscroll_enabled = false;
    g_charybdis_config.is_carret_enabled = false;
    g_charybdis_config.is_custom_enabled = false;
}

void pointing_device_init_kb(void) { maybe_update_pointing_device_cpi(&g_charybdis_config); }

#    ifndef CONSTRAIN_HID
#        define CONSTRAIN_HID(value) ((value) < -127 ? -127 : ((value) > 127 ? 127 : (value)))
#    endif  // !CONSTRAIN_HID

void tap_tb(uint8_t keycode0, uint8_t keycode1, uint8_t keycode2, uint8_t keycode3, int16_t *move_buffer_x, int16_t *move_buffer_y);

/**
 * \brief Add optional acceleration effect.
 *
 * If `CHARYBDIS_POINTER_ACCELERATION_ENABLE` is defined, add a simple and naive
 * acceleration effect to the provided value.  Return the value unchanged
 * otherwise.
 */
#    ifndef DISPLACEMENT_WITH_ACCELERATION
#        ifdef CHARYBDIS_POINTER_ACCELERATION_ENABLE
#            define DISPLACEMENT_WITH_ACCELERATION(d) (CONSTRAIN_HID(d > 0 ? d * d / CHARYBDIS_POINTER_ACCELERATION_FACTOR + d : -d * d / CHARYBDIS_POINTER_ACCELERATION_FACTOR + d))
#        else  // !CHARYBDIS_POINTER_ACCELERATION_ENABLE
#            define DISPLACEMENT_WITH_ACCELERATION(d) (d)
#        endif  // CHARYBDIS_POINTER_ACCELERATION_ENABLE
#    endif      // !DISPLACEMENT_WITH_ACCELERATION

void tap_code_fast(uint8_t code) {
  register_code(code);
  unregister_code(code);
}

int max(int num1, int num2) { return (num1 > num2) ? num1 : num2; }
int min(int num1, int num2) { return (num1 > num2) ? num2 : num1; }

void tap_tb(uint8_t keycode0, uint8_t keycode1, uint8_t keycode2, uint8_t keycode3, int16_t *move_buffer_x, int16_t *move_buffer_y) {
    uint16_t local_carret_dpi = g_charybdis_config.is_integ_enabled? CHARYBDIS_CARRET_BUFFER * 20 : CHARYBDIS_CARRET_BUFFER;
	local_carret_dpi = g_charybdis_config.is_sniping_enabled? local_carret_dpi : local_carret_dpi / 4;
    if (abs(*move_buffer_x) + abs(*move_buffer_y) < local_carret_dpi) { return; }
    if ((abs(*move_buffer_x) > abs(*move_buffer_y)) && (*move_buffer_x > 0)) {
		for (int8_t i = 0; i <= (abs(*move_buffer_x) + abs(*move_buffer_y)) / local_carret_dpi; i++) {
			tap_code_fast(keycode0);
			*move_buffer_x = max(*move_buffer_x - local_carret_dpi, 0);
		}
		*move_buffer_y = 0;
		return;
	}
    if ((abs(*move_buffer_x) > abs(*move_buffer_y)) && (*move_buffer_x <= 0)) {
		for (int8_t i = 0; i <= (abs(*move_buffer_x) + abs(*move_buffer_y)) / local_carret_dpi; i++) {
			tap_code_fast(keycode1);
			*move_buffer_x = min(*move_buffer_x + local_carret_dpi, 0);
		}
		*move_buffer_y = 0;
		return;
	}
    if ((abs(*move_buffer_x) <= abs(*move_buffer_y)) && (*move_buffer_y > 0)) {
        for (int8_t i = 0; i <= (abs(*move_buffer_x) + abs(*move_buffer_y)) / local_carret_dpi; i++) {
			tap_code_fast(keycode2);
			*move_buffer_y = max(*move_buffer_y - local_carret_dpi, 0);
		}
		*move_buffer_x = 0;
		return;
	}
    if ((abs(*move_buffer_x) <= abs(*move_buffer_y)) && (*move_buffer_y <= 0)) {
		for (int8_t i = 0; i <= (abs(*move_buffer_x) + abs(*move_buffer_y)) / local_carret_dpi; i++) {
			tap_code_fast(keycode3);
			*move_buffer_y = min(*move_buffer_y + local_carret_dpi, 0);
		}
		*move_buffer_x = 0;
		return;
	}
}

void tap_modes(int16_t *move_buffer_x, int16_t *move_buffer_y) {
    if (abs(*move_buffer_x) + abs(*move_buffer_y) < CHARYBDIS_CARRET_BUFFER) { return; }
    if ((abs(*move_buffer_x) > abs(*move_buffer_y)) && (*move_buffer_x > 0)) {
		for (int8_t i = 0; i <= (abs(*move_buffer_x) + abs(*move_buffer_y)) / CHARYBDIS_CARRET_BUFFER; i++) {
			charybdis_set_pointer_dragscroll_enabled(true);
			*move_buffer_x = max(*move_buffer_x - CHARYBDIS_CARRET_BUFFER, 0);
		}
		*move_buffer_y = 0;
		return;
	}
    if ((abs(*move_buffer_x) > abs(*move_buffer_y)) && (*move_buffer_x <= 0)) {
		for (int8_t i = 0; i <= (abs(*move_buffer_x) + abs(*move_buffer_y)) / CHARYBDIS_CARRET_BUFFER; i++) {
			charybdis_set_pointer_carret_enabled(true);
			*move_buffer_x = min(*move_buffer_x + CHARYBDIS_CARRET_BUFFER, 0);
		}
		*move_buffer_y = 0;
		return;
	}
    if ((abs(*move_buffer_x) <= abs(*move_buffer_y)) && (*move_buffer_y > 0)) {
        for (int8_t i = 0; i <= (abs(*move_buffer_x) + abs(*move_buffer_y)) / CHARYBDIS_CARRET_BUFFER; i++) {
			charybdis_set_pointer_disable_nonstacking();
			charybdis_set_pointer_integ_enabled(false);
			*move_buffer_y = max(*move_buffer_y - CHARYBDIS_CARRET_BUFFER, 0);
		}
		*move_buffer_x = 0;
		return;
	}
    if ((abs(*move_buffer_x) <= abs(*move_buffer_y)) && (*move_buffer_y <= 0)) {
		for (int8_t i = 0; i <= (abs(*move_buffer_x) + abs(*move_buffer_y)) / CHARYBDIS_CARRET_BUFFER; i++) {
			charybdis_set_pointer_integ_enabled(true);
			*move_buffer_y = min(*move_buffer_y + CHARYBDIS_CARRET_BUFFER, 0);
		}
		*move_buffer_x = 0;
		return;
	}
}

/**
 * \brief Augment the pointing device behavior.
 *
 * Implement the Charybdis-specific features for pointing devices:
 *   - Drag-scroll
 *   - Sniping
 *   - Carret
 *   - Acceleration
 */
static void pointing_device_task_charybdis(report_mouse_t* mouse_report) {
    static int16_t move_buffer_x = 0;
    static int16_t move_buffer_y = 0;
    static int16_t local_mouse_report_x;
    static int16_t local_mouse_report_y;
    uint16_t local_dragscroll_buffer_size;
	if (g_charybdis_config.is_integ_enabled) {
		local_mouse_report_x += mouse_report->x;
		local_mouse_report_y += mouse_report->y;
        local_dragscroll_buffer_size = CHARYBDIS_DRAGSCROLL_BUFFER_SIZE * 300;
    } else {
        local_mouse_report_x = mouse_report->x;
        local_mouse_report_y = mouse_report->y;
        local_dragscroll_buffer_size = CHARYBDIS_DRAGSCROLL_BUFFER_SIZE;
    }
    if (g_charybdis_config.is_modemode_enabled) {
        move_buffer_x += local_mouse_report_x;
		move_buffer_y -= local_mouse_report_y;
		tap_modes(&move_buffer_x, &move_buffer_y);
        mouse_report->x = 0;
        mouse_report->y = 0;
    } else if (g_charybdis_config.is_dragscroll_enabled) {
#    ifdef CHARYBDIS_DRAGSCROLL_REVERSE_X
        move_buffer_x -= local_mouse_report_x;
#    else
        move_buffer_x += local_mouse_report_x;
#    endif  // CHARYBDIS_DRAGSCROLL_REVERSE_X
#    ifdef CHARYBDIS_DRAGSCROLL_REVERSE_Y
        move_buffer_y -= local_mouse_report_y;
#    else
        move_buffer_y += local_mouse_report_y;
#    endif  // CHARYBDIS_DRAGSCROLL_REVERSE_Y
        mouse_report->x = 0;
        mouse_report->y = 0;
        if (abs(move_buffer_x) > local_dragscroll_buffer_size) {
            mouse_report->h = move_buffer_x > 0 ? 1 : -1;
            move_buffer_x = 0;
        }
        if (abs(move_buffer_y) > local_dragscroll_buffer_size) {
            mouse_report->v = move_buffer_y > 0 ? 1 : -1;
            move_buffer_y = 0;
        }
	} else if (g_charybdis_config.is_carret_enabled) {
#       ifdef CHARYBDIS_CARRET_REVERSE_X
        move_buffer_x -= local_mouse_report_x;
#       else
        move_buffer_x += local_mouse_report_x;
#       endif
#       ifdef CHARYBDIS_CARRET_REVERSE_Y
		move_buffer_y -= local_mouse_report_y;
#       else
		move_buffer_y += local_mouse_report_y;
#       endif
		tap_tb(KC_RIGHT, KC_LEFT, KC_UP, KC_DOWN, &move_buffer_x, &move_buffer_y);
        mouse_report->x = 0;
        mouse_report->y = 0;
    } else if (g_charybdis_config.is_custom_enabled) {
        move_buffer_x += local_mouse_report_x;
		move_buffer_y -= local_mouse_report_y;
		tap_tb(CUSTOM_FN_RIGHT, CUSTOM_FN_LEFT, CUSTOM_FN_UP, CUSTOM_FN_DOWN, &move_buffer_x, &move_buffer_y);
        mouse_report->x = 0;
        mouse_report->y = 0;
    } else if (!g_charybdis_config.is_sniping_enabled) {
        mouse_report->x = DISPLACEMENT_WITH_ACCELERATION(mouse_report->x);
        mouse_report->y = DISPLACEMENT_WITH_ACCELERATION(mouse_report->y);
}
}


report_mouse_t pointing_device_task_kb(report_mouse_t mouse_report) {
    pointing_device_task_charybdis(&mouse_report);
    mouse_report = pointing_device_task_user(mouse_report);

    return mouse_report;
}

#    if defined(POINTING_DEVICE_ENABLE) && !defined(NO_CHARYBDIS_KEYCODES)
/** \brief Whether SHIFT mod is enabled. */
static bool has_shift_mod(void) {
#        ifdef NO_ACTION_ONESHOT
    return mod_config(get_mods()) & MOD_MASK_SHIFT;
#        else
    return mod_config(get_mods() | get_oneshot_mods()) & MOD_MASK_SHIFT;
#        endif  // NO_ACTION_ONESHOT
}
#    endif  // POINTING_DEVICE_ENABLE && !NO_CHARYBDIS_KEYCODES

/**
 * \brief Outputs the Charybdis configuration to console.
 *
 * Prints the in-memory configuration structure to console, for debugging.
 * Includes:
 *   - raw value
 *   - drag-scroll: on/off
 *   - sniping: on/off
 *   - carret: on/off
 *   - integration mode: on/ off
 *   - default DPI: internal table index/actual DPI
 *   - sniping DPI: internal table index/actual DPI
 */
__attribute__((unused)) static void debug_charybdis_config_to_console(charybdis_config_t* config) {
#    ifdef CONSOLE_ENABLE
    dprintf("(charybdis) process_record_kb: config = {\n"
            "\traw = 0x%04X,\n"
            "\t{\n"
            "\t\tis_dragscroll_enabled=%b\n"
            "\t\tis_sniping_enabled=%b\n"
            "\t\tis_carret_enabled=%b\n"
            "\t\tis_custom_enabled=%b\n"
            "\t\tis_modemode_enabled=%b\n"
            "\t\tis_integ_enabled=%b\n"
            "\t\tdefault_dpi=0x%02X (%ld)\n"
            "\t\tsniping_dpi=0x%01X (%ld)\n"
            "\t}\n"
            "}\n",
            config->raw, config->is_dragscroll_enabled, config->is_sniping_enabled, config->is_carret_enabled, config->is_custom_enabled, config->is_modemode_enabled, config->is_integ_enabled,  config->pointer_default_dpi, get_pointer_default_dpi(config), config->pointer_sniping_dpi, get_pointer_sniping_dpi(config));
#    endif  // CONSOLE_ENABLE
}

bool process_record_kb(uint16_t keycode, keyrecord_t* record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }
#    ifndef NO_CHARYBDIS_KEYCODES
    switch (keycode) {
        case POINTER_DEFAULT_DPI_FORWARD:
            if (record->event.pressed) {
                // Step backward if shifted, forward otherwise.
                charybdis_cycle_pointer_default_dpi(/* forward= */ !has_shift_mod());
            }
            break;
        case POINTER_DEFAULT_DPI_REVERSE:
            if (record->event.pressed) {
                // Step forward if shifted, backward otherwise.
                charybdis_cycle_pointer_default_dpi(/* forward= */ has_shift_mod());
            }
            break;
        case POINTER_SNIPING_DPI_FORWARD:
            if (record->event.pressed) {
                // Step backward if shifted, forward otherwise.
                charybdis_cycle_pointer_sniping_dpi(/* forward= */ !has_shift_mod());
            }
            break;
        case POINTER_SNIPING_DPI_REVERSE:
            if (record->event.pressed) {
                // Step forward if shifted, backward otherwise.
                charybdis_cycle_pointer_sniping_dpi(/* forward= */ has_shift_mod());
            }
            break;
        case SNIPING_MODE:
            charybdis_set_pointer_sniping_enabled(record->event.pressed);
            break;
        case SNIPING_MODE_TOGGLE:
            if (record->event.pressed) {
                charybdis_set_pointer_sniping_enabled(!charybdis_get_pointer_sniping_enabled());
            }
            break;
        case DRAGSCROLL_MODE:
            charybdis_set_pointer_dragscroll_enabled(record->event.pressed);
            break;
        case DRAGSCROLL_MODE_TOGGLE:
            if (record->event.pressed) {
                charybdis_set_pointer_carret_enabled(!charybdis_get_pointer_carret_enabled());
            }
            break;
        case CARRET_MODE:
            charybdis_set_pointer_carret_enabled(record->event.pressed);
            break;
        case CARRET_MODE_TOGGLE:
            if (record->event.pressed) {
                charybdis_set_pointer_carret_enabled(!charybdis_get_pointer_carret_enabled());
            }
            break;
        case CUSTOM_MODE:
            charybdis_set_pointer_custom_enabled(record->event.pressed);
            break;
        case CUSTOM_MODE_TOGGLE:
            if (record->event.pressed) {
                charybdis_set_pointer_custom_enabled(!charybdis_get_pointer_custom_enabled());
            }
            break;
        case MODE_MODE:
            charybdis_set_pointer_modemode_enabled(record->event.pressed);
            break;
        case MODE_MODE_TOGGLE:
            if (record->event.pressed) {
                charybdis_set_pointer_modemode_enabled(!charybdis_get_pointer_modemode_enabled());
            }
            break;
        case INTEG_MODE:
            charybdis_set_pointer_integ_enabled(record->event.pressed);
            break;
        case INTEG_MODE_TOGGLE:
            if (record->event.pressed) {
                charybdis_set_pointer_integ_enabled(!charybdis_get_pointer_integ_enabled());
            }
            break;
    }
#    endif  // !NO_CHARYBDIS_KEYCODES
#    ifndef MOUSEKEY_ENABLE
    // Simulate mouse keys if full support is not enabled (reduces firmware size
    // while maintaining support for mouse keys).
    if (IS_MOUSEKEY_BUTTON(keycode)) {
        report_mouse_t mouse_report = pointing_device_get_report();
        mouse_report.buttons        = pointing_device_handle_buttons(mouse_report.buttons, record->event.pressed, keycode - KC_MS_BTN1);
        pointing_device_set_report(mouse_report);
        pointing_device_send();
    }
#    endif  // !MOUSEKEY_ENABLE
    return true;
}

void eeconfig_init_kb(void) {
    g_charybdis_config.raw = 0;
    write_charybdis_config_to_eeprom(&g_charybdis_config);
    maybe_update_pointing_device_cpi(&g_charybdis_config);
    eeconfig_init_user();
}

void matrix_power_up(void) { pointing_device_task(); }

void charybdis_config_sync_handler(uint8_t initiator2target_buffer_size, const void* initiator2target_buffer, uint8_t target2initiator_buffer_size, void* target2initiator_buffer) {
    if (initiator2target_buffer_size == sizeof(g_charybdis_config)) {
        memcpy(&g_charybdis_config, initiator2target_buffer, sizeof(g_charybdis_config));
    }
}

void keyboard_post_init_kb(void) {
    transaction_register_rpc(RPC_ID_KB_CONFIG_SYNC, charybdis_config_sync_handler);

    keyboard_post_init_user();
}

void housekeeping_task_kb(void) {
    if (is_keyboard_master()) {
        // Keep track of the last state, so that we can tell if we need to propagate to slave
        static charybdis_config_t last_charybdis_config = {0};
        static uint32_t           last_sync             = 0;
        bool                      needs_sync            = false;

        // Check if the state values are different
        if (memcmp(&g_charybdis_config, &last_charybdis_config, sizeof(g_charybdis_config))) {
            needs_sync = true;
            memcpy(&last_charybdis_config, &g_charybdis_config, sizeof(g_charybdis_config));
        }
        // Send to slave every 500ms regardless of state change
        if (timer_elapsed32(last_sync) > 500) {
            needs_sync = true;
        }

        // Perform the sync if requested
        if (needs_sync) {
            if (transaction_rpc_send(RPC_ID_KB_CONFIG_SYNC, sizeof(g_charybdis_config), &g_charybdis_config)) {
                last_sync = timer_read32();
            }
        }
    }
    // no need for user function, is called already
}

#endif  // POINTING_DEVICE_ENABLE

__attribute__((weak)) void matrix_init_sub_kb(void) {}
void                       matrix_init_kb(void) {
#ifdef POINTING_DEVICE_ENABLE
    read_charybdis_config_from_eeprom(&g_charybdis_config);
#endif  // POINTING_DEVICE_ENABLE
    matrix_init_sub_kb();
    matrix_init_user();
}

__attribute__((weak)) void matrix_scan_sub_kb(void) {}
void                       matrix_scan_kb(void) {
    matrix_scan_sub_kb();
    matrix_scan_user();
}
