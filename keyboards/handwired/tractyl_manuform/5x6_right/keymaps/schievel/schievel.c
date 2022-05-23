#include "schievel.h"


#define _LOWER 1
#define _RAISE 2


/***************************
 * Sticky key related defines
 **************************/

// identify keycombinations
bool is_alt_active = false;
bool is_sft_active = false;
bool is_ctl_active = false;
bool is_gui_active = false;
bool is_low_active = false;
bool is_rai_active = false;

bool activate_alt = false;
bool activate_sft = false;
bool activate_ctl = false;
bool activate_gui = false;
bool activate_low = false;
bool activate_rai = false;

bool sticky_key = false;
uint32_t sticky_timer = 0;
#define STICKY_TERM 500

/***************************
 * Tap dance 1 of 2
 **************************/

// define `ACTION_TAP_DANCE_FN_ADVANCED()` for each tapdance keycode, passing in
// `finished` and `reset` functions
qk_tap_dance_action_t tap_dance_actions[] = {
  [ALT_TM] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, alttm_finished, alttm_reset),
  [SFT_TM] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, sfttm_finished, sfttm_reset),
  [CTL_TM] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, ctltm_finished, ctltm_reset),
  [GUI_TM] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, guitm_finished, guitm_reset),
  [RAI_TM] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, raitm_finished, raitm_reset),
  [LOW_TM] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, lowtm_finished, lowtm_reset),
  [ESC_TM] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, esctm_finished, esctm_reset),
};

// define a type containing as many tapdance states as you need
typedef enum { SINGLE_TAP, SINGLE_HOLD } td_state_t;

// create a global instance of the tapdance state type
static td_state_t td_state;

// function to determine the current tapdance state
int cur_dance(qk_tap_dance_state_t *state);

/* /\*************************** */
/*  * Mouse pressed */
/*  **************************\/ */

/* void on_mouse_button(uint8_t mouse_button, bool pressed) { */
/*   report_mouse_t report = pointing_device_get_report(); */

/*   if (pressed) */
/*     report.buttons |= mouse_button; */
/*   else */
/*     report.buttons &= ~mouse_button; */
/*   pointing_device_set_report(report); */
/*   pointing_device_send(); */
/* } */

/* /\*************************** */
/*  * Combos */
/*  **************************\/ */

/* enum combos_events { RS_MOUSE, ST_MOUSE, RT_MOUSE }; */

/* const uint16_t PROGMEM rsm_combo[] = {KC_R, KC_S, COMBO_END}; */
/* const uint16_t PROGMEM stm_combo[] = {KC_S, KC_T, COMBO_END}; */
/* const uint16_t PROGMEM rtm_combo[] = {KC_R, KC_T, COMBO_END}; */

/* combo_t key_combos[COMBO_COUNT] = { */
/*   [RS_MOUSE] = COMBO_ACTION(rsm_combo), */
/*   [ST_MOUSE] = COMBO_ACTION(stm_combo), */
/*   [RT_MOUSE] = COMBO_ACTION(rtm_combo), */
/* }; */

/* void process_combo_event(uint16_t combo_index, bool pressed) { */
/*   switch (combo_index) { */
/*   case RS_MOUSE: */
/*     on_mouse_button(MOUSE_BTN2, pressed); */
/*     break; */
/*   case ST_MOUSE: */
/*     on_mouse_button(MOUSE_BTN1, pressed); */
/*     break; */
/*   case RT_MOUSE: */
/*     on_mouse_button(MOUSE_BTN3, pressed); */
/*     break; */
/*   } */
/* } */

/***************************
 * Tap dance 2 of 2
 **************************/

// determine the tapdance state to return
int cur_dance(qk_tap_dance_state_t *state) {
  if (state->count == 1) {
    // if (state->interrupted || !state->pressed) { return SINGLE_TAP; }
    // //interrupted sends SINGLE_TAP
    if (!state->pressed) {
      return SINGLE_TAP;
    } // interrupted sends SINGLE_HOLD
    else {
      return SINGLE_HOLD;
    }
  } else {
    return 2;
  } // any number higher than the maximum state value you return above
}

void unstick_keys(void) {
  if (MOD_BIT(KC_LALT)) {
    unregister_mods(MOD_BIT(KC_LALT));
    is_alt_active = false;
  }
  if (MOD_BIT(KC_LSFT)) {
    unregister_mods(MOD_BIT(KC_LSFT));
    is_sft_active = false;
  }
  if (MOD_BIT(KC_LCTL)) {
    unregister_mods(MOD_BIT(KC_LCTL));
    is_ctl_active = false;
  }
  if (MOD_BIT(KC_LGUI)) {
    unregister_mods(MOD_BIT(KC_LGUI));
    is_gui_active = false;
  }
  if (is_rai_active) {
    layer_off(_RAISE);
    is_rai_active = false;
  }
  if (is_low_active) {
    layer_off(_LOWER);
    charybdis_set_pointer_sniping_enabled(false);
    is_low_active = false;
  }
}

void activate_stkeys(void) {
  if (activate_alt) {
    register_mods(MOD_BIT(KC_LALT));
    activate_alt = false;
    is_alt_active = true;
  }
  if (activate_sft) {
    register_mods(MOD_BIT(KC_LSFT));
    activate_sft = false;
    is_sft_active = true;
  }
  if (activate_ctl) {
    register_mods(MOD_BIT(KC_LCTL));
    activate_ctl = false;
    is_ctl_active = true;
  }
  if (activate_gui) {
    register_mods(MOD_BIT(KC_LGUI));
    activate_gui = false;
    is_gui_active = true;
  }
  if (activate_rai) {
    layer_on(_RAISE);
    activate_rai = false;
    is_rai_active = true;
  }
  if (activate_low) {
    layer_on(_LOWER);
    charybdis_set_pointer_sniping_enabled(true);
    activate_low = false;
    is_low_active = true;
  }
}

void alttm_finished(qk_tap_dance_state_t *state, void *user_data) {
  td_state = cur_dance(state);
  sticky_timer = timer_read32();
  activate_alt = true;
  sticky_key = true;
  switch (td_state) {
  case SINGLE_TAP:
    charybdis_set_pointer_dragscroll_enabled(true);
    break;
  case SINGLE_HOLD:
    activate_stkeys();
    sticky_key = false;
  }
}
void alttm_reset(qk_tap_dance_state_t *state, void *user_data) {
  switch (td_state) {
  case SINGLE_TAP:
    break;
  case SINGLE_HOLD:
    unstick_keys();
  }
}
void sfttm_finished(qk_tap_dance_state_t *state, void *user_data) {
  td_state = cur_dance(state);
  sticky_timer = timer_read32();
  activate_sft = true;
  sticky_key = true;
  switch (td_state) {
  case SINGLE_TAP:
    break;
  case SINGLE_HOLD:
    activate_stkeys();
    sticky_key = false;
  }
}
void sfttm_reset(qk_tap_dance_state_t *state, void *user_data) {
  switch (td_state) {
  case SINGLE_TAP:
    break;
  case SINGLE_HOLD:
    unstick_keys();
  }
}
void ctltm_finished(qk_tap_dance_state_t *state, void *user_data) {
  td_state = cur_dance(state);
  sticky_timer = timer_read32();
  activate_ctl = true;
  sticky_key = true;
  switch (td_state) {
  case SINGLE_TAP:
    charybdis_set_pointer_sniping_enabled(false);
    charybdis_set_pointer_dragscroll_enabled(false);
    break;
  case SINGLE_HOLD:
    activate_stkeys();
    sticky_key = false;
  }
}
void ctltm_reset(qk_tap_dance_state_t *state, void *user_data) {
  switch (td_state) {
  case SINGLE_TAP:
    break;
  case SINGLE_HOLD:
    unstick_keys();
  }
}
void guitm_finished(qk_tap_dance_state_t *state, void *user_data) {
  td_state = cur_dance(state);
  sticky_timer = timer_read32();
  activate_gui = true;
  sticky_key = true;
  switch (td_state) {
  case SINGLE_TAP:
    break;
  case SINGLE_HOLD:
    activate_stkeys();
    sticky_key = false;
  }
}
void guitm_reset(qk_tap_dance_state_t *state, void *user_data) {
  switch (td_state) {
  case SINGLE_TAP:
    break;
  case SINGLE_HOLD:
    unstick_keys();
  }
}
void raitm_finished(qk_tap_dance_state_t *state, void *user_data) {
  td_state = cur_dance(state);
  sticky_timer = timer_read32();
  activate_rai = true;
  is_rai_active = true;
  sticky_key = true;
  layer_on(_RAISE);
  switch (td_state) {
  case SINGLE_TAP:
    break;
  case SINGLE_HOLD:
    activate_stkeys();
    sticky_key = false;
  }
}
void raitm_reset(qk_tap_dance_state_t *state, void *user_data) {
  switch (td_state) {
  case SINGLE_TAP:
    break;
  case SINGLE_HOLD:
    unstick_keys();
  }
}
void lowtm_finished(qk_tap_dance_state_t *state, void *user_data) {
  td_state = cur_dance(state);
  sticky_timer = timer_read32();
  activate_low = true;
  is_low_active = true;
  sticky_key = true;
  layer_on(_LOWER);
  switch (td_state) {
  case SINGLE_TAP:
    break;
  case SINGLE_HOLD:
    activate_stkeys();
    sticky_key = false;
    // prev_track_mode = track_mode;
    // track_mode = SND_BRIGHT_MODE;
  }
}
void lowtm_reset(qk_tap_dance_state_t *state, void *user_data) {
  switch (td_state) {
  case SINGLE_TAP:
    break;
  case SINGLE_HOLD:
    unstick_keys();
    // track_mode = prev_track_mode;
  }
}
void esctm_finished(qk_tap_dance_state_t *state, void *user_data) {
  td_state = cur_dance(state);
  sticky_timer = timer_read32();
  switch (td_state) {
  case SINGLE_TAP:
    charybdis_set_pointer_sniping_enabled(false);
    charybdis_set_pointer_dragscroll_enabled(false);
    tap_code(KC_ESC);
    break;
  case SINGLE_HOLD:
    break;
  }
}
void esctm_reset(qk_tap_dance_state_t *state, void *user_data) {
  switch (td_state) {
  case SINGLE_TAP:
    break;
  case SINGLE_HOLD:
    unstick_keys();
  }
}
