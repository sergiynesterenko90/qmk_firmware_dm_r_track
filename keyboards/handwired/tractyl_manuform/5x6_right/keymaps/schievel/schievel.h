#include "tractyl_manuform.h"

// tapdance keycodes
enum td_keycodes { ALT_TM, SFT_TM, CTL_TM, GUI_TM, RAI_TM, LOW_TM, ESC_TM };
// function to determine the current tapdance state
int cur_dance(qk_tap_dance_state_t *state);
//// `finished` and `reset` functions for each tapdance keycode
void alttm_finished(qk_tap_dance_state_t *state, void *user_data);
void alttm_reset(qk_tap_dance_state_t *state, void *user_data);
void sfttm_finished(qk_tap_dance_state_t *state, void *user_data);
void sfttm_reset(qk_tap_dance_state_t *state, void *user_data);
void ctltm_finished(qk_tap_dance_state_t *state, void *user_data);
void ctltm_reset(qk_tap_dance_state_t *state, void *user_data);
void guitm_finished(qk_tap_dance_state_t *state, void *user_data);
void guitm_reset(qk_tap_dance_state_t *state, void *user_data);
void raitm_finished(qk_tap_dance_state_t *state, void *user_data);
void raitm_reset(qk_tap_dance_state_t *state, void *user_data);
void lowtm_finished(qk_tap_dance_state_t *state, void *user_data);
void lowtm_reset(qk_tap_dance_state_t *state, void *user_data);
void esctm_finished(qk_tap_dance_state_t *state, void *user_data);
void esctm_reset(qk_tap_dance_state_t *state, void *user_data);

/***************************
 * Custom Keycodes and Layers
 **************************/
enum custom_keycodes {
  KC_RAISE,
  KC_LOWER,
};
