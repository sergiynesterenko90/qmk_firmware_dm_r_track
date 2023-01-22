/* Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
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

#include QMK_KEYBOARD_H

enum custom_layers {
    _QWERTY,
    _LAYER_F1,
    _LAYER_F2,
    _LAYER_F3,
};

// layer activation behavior:
// https://github.com/qmk/qmk_firmware/blob/master/docs/feature_layers.md

#define LAYER_F1 MO(_LAYER_F1)
#define LAYER_F2 MO(_LAYER_F2)
#define LAYER_F3 MO(_LAYER_F3)

// Key code definitions:
// https://github.com/qmk/qmk_firmware/blob/master/docs/keycodes.md

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_QWERTY] = LAYOUT_5x6_right(
     KC_ESC , KC_1  , KC_2  , KC_3  , KC_4  , KC_5  ,                         KC_6  , KC_7  , KC_8  , KC_9  , KC_0  ,KC_BSPC,
     KC_TAB , KC_Q  , KC_W  , KC_E  , KC_R  , KC_T  ,                         KC_Y  , KC_U  , KC_I  , KC_O  , KC_P  ,KC_MINS,
     KC_LSFT, KC_A  , KC_S  , KC_D  , KC_F  , KC_G  ,                         KC_H  , KC_J  , KC_K  , KC_L  ,KC_SCLN,KC_QUOT,
     KC_LCTL, KC_Z  , KC_X  , KC_C  , KC_V  , KC_B  ,                         KC_N  , KC_M  ,KC_COMM,KC_DOT ,LAYER_F3,KC_BSLASH,
                      KC_LBRC,KC_RBRC,                                                       KC_EQL, KC_SLSH,
                                      LAYER_F2,KC_SPC,                                    LAYER_F1,
                                      KC_TAB,KC_HOME,                                  KC_ENT,
                                      KC_BSPC, KC_GRV,                        KC_LGUI, KC_DEL
  ),

  [_LAYER_F1] = LAYOUT_5x6_right(
     KC_TILD,KC_EXLM, KC_AT ,KC_HASH,KC_DLR ,KC_PERC,                        KC_CIRC,KC_AMPR,KC_ASTR,KC_LPRN,KC_RPRN,KC_DEL,
     _______,_______,_______,_______,_______,KC_LBRC,                        KC_RBRC, KC_P7 , KC_P8 , KC_P9 ,_______,KC_PLUS,
     _______,KC_HOME,KC_PGUP,KC_PGDN,KC_END ,KC_LPRN,                        KC_RPRN, KC_P4 , KC_P5 , KC_P6 ,KC_MINS,KC_PIPE,
     _______,_______,_______,_______,_______,_______,                        _______, KC_P1 , KC_P2 , KC_P3 ,KC_EQL ,KC_UNDS,
                     _______,KC_PSCR,                                                        _______, KC_P0,
                                             KC_LEFT_ALT,_______,                _______,
                                             _______,_______,                _______,
                                             _______,_______,        _______,_______

  ),

  [_LAYER_F2] = LAYOUT_5x6_right(
       KC_F12 , KC_F1 , KC_F2 , KC_F3 , KC_F4 , KC_F5 ,                        KC_F6  , KC_F7 , KC_F8 , KC_F9 ,KC_F10 ,KC_F11 ,
       _______,_______,_______,_______,_______,KC_LBRC,                        KC_RBRC,_______,KC_NLCK,KC_INS ,KC_SLCK,KC_MUTE,
       _______,KC_LEFT,KC_UP  ,KC_DOWN,KC_RGHT,KC_LPRN,                        KC_RPRN,KC_MPRV,KC_MPLY,KC_MNXT,_______,KC_VOLU,
       _______,_______,_______,_______,_______,_______,                        _______,_______,_______,_______,_______,KC_VOLD,
                       _______,_______,                                                        KC_EQL ,_______,
                                               _______,_______,                _______,
                                               _______,_______,                _______,
                                               _______,_______,         KC_LALT,_______
  ),

  [_LAYER_F3] = LAYOUT_5x6_right(
       _______,_______,_______,_______,_______,_______,                        _______,_______,_______,_______,_______,_______,
       _______,_______,_______,_______,_______,_______,                        CUSTOM_MODE,CARRET_MODE,DRAGSCROLL_MODE,_______,_______,_______,
       _______,_______,_______,_______,_______,_______,                        KC_MS_BTN3,KC_MS_BTN1,KC_MS_BTN2,SNIPING,_______,_______,
       _______,_______,_______,_______,_______,_______,                        KC_WWW_BACK,KC_WWW_FORWARD,LCTL(KC_PGUP),LCTL(KC_PGDN),_______,_______,
                       _______,_______,                                                        _______ ,_______,
                                               _______,_______,                _______,
                                               _______,_______,                _______,
                                               _______,_______,         KC_SPC,LCTL(KC_W)
  ),
};
