TAP_DANCE_ENABLE = yes

# for changing what shift does to a key
KEY_OVERRIDE_ENABLE = yes

EXTRAFLAGS += -flto -mcall-prologues
SRC += tapdance.c
