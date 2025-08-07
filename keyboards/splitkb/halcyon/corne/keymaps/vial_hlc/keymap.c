#include QMK_KEYBOARD_H

// Tap dance implementation for Vial
void dance_0_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        register_code(KC_TAB);
    } else if (state->count == 2) {
        register_code(KC_CAPS);
    }
}

void dance_0_reset(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        unregister_code(KC_TAB);
    } else if (state->count == 2) {
        unregister_code(KC_CAPS);
    }
}


// RGB underglow colors for layers (HSV format)
#define HSV_LAYER_0_RGB   0, 0, 120        // Dim white for base layer
#define HSV_LAYER_1_RGB   30, 255, 200    // Bright orange for navigation/media
#define HSV_LAYER_2_RGB   180, 255, 200   // Bright cyan for numpad
#define HSV_LAYER_3_RGB   0, 255, 200     // Bright red for RGB/settings
#define HSV_LAYER_4_RGB   60, 255, 200    // Bright yellow for symbols
#define HSV_LAYER_5_RGB   220, 255, 200   // Bright magenta for mouse
#define HSV_LAYER_6_RGB   120, 255, 200   // Bright green for functions/numbers
#define HSV_LAYER_7_RGB   210, 255, 200   // Bright blue-cyan for layer 7

// Layer-based and modifier-based RGB matrix
#ifdef RGB_MATRIX_ENABLE

// Function to update RGB based on current state
void update_rgb_for_current_state(void) {
    uint8_t layer = get_highest_layer(layer_state);
    uint8_t brightness = rgb_matrix_get_val();
    
    // Check for Caps Lock first - highest priority with breathing effect
    if (host_keyboard_led_state().caps_lock) {
        rgb_matrix_mode(RGB_MATRIX_BREATHING);
        rgb_matrix_sethsv(17, 191, brightness); // Orange breathing effect for Caps Lock (matches TFT)
        return;
    }
    
    // Set solid color mode for normal operation
    rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
    
    // Check for active modifiers - they override layer colors (match TFT colors)
    uint8_t mods = get_mods() | get_weak_mods() | get_oneshot_mods();
    if (mods & MOD_MASK_CTRL) {
        rgb_matrix_sethsv(30, 191, brightness);  // Orange for CTRL (preserve brightness)
    } else if (mods & MOD_MASK_ALT) {
        rgb_matrix_sethsv(180, 191, brightness); // Cyan for ALT (preserve brightness)
    } else if (mods & MOD_MASK_GUI) {
        rgb_matrix_sethsv(120, 191, brightness); // Green for GUI (preserve brightness)
    } else if (mods & MOD_MASK_SHIFT) {
        rgb_matrix_sethsv(240, 191, brightness); // Purple for SHIFT (preserve brightness)
    } else {
        // No modifiers active, use layer-based colors (preserve brightness)
        uint8_t brightness = rgb_matrix_get_val();
        switch (layer) {
            case 0:
                rgb_matrix_sethsv(0, 0, brightness);        // Dim white for base layer
                break;
            case 1:
                rgb_matrix_sethsv(30, 255, brightness);     // Bright orange for navigation/media
                break;
            case 2:
                rgb_matrix_sethsv(180, 255, brightness);    // Bright cyan for numpad
                break;
            case 3:
                rgb_matrix_sethsv(0, 255, brightness);      // Bright red for RGB/settings
                break;
            case 4:
                rgb_matrix_sethsv(60, 255, brightness);     // Bright yellow for symbols
                break;
            case 5:
                rgb_matrix_sethsv(220, 255, brightness);    // Bright magenta for mouse
                break;
            case 6:
                rgb_matrix_sethsv(120, 255, brightness);    // Bright green for functions/numbers
                break;
            case 7:
                rgb_matrix_sethsv(210, 255, brightness);    // Bright blue-cyan for layer 7
                break;
            default:
                rgb_matrix_sethsv(0, 0, brightness);        // Default to base layer
                break;
        }
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    update_rgb_for_current_state();
    return state;
}

// Catch modifier and layer changes immediately
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    bool should_update_rgb = false;
    
    // Check if it's a modifier key
    switch (keycode) {
        case KC_LCTL:
        case KC_RCTL:
        case KC_LSFT:
        case KC_RSFT:
        case KC_LALT:
        case KC_RALT:
        case KC_LGUI:
        case KC_RGUI:
            should_update_rgb = true;
            break;
        default:
            // Check for mod-tap keys
            if ((keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) ||
                (keycode >= LCTL_T(KC_A) && keycode <= RGUI_T(KC_SCLN))) {
                should_update_rgb = true;
            }
            // Check for layer keys
            else if ((keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) ||
                     (keycode >= QK_MOMENTARY && keycode <= QK_MOMENTARY_MAX) ||
                     (keycode >= QK_TOGGLE_LAYER && keycode <= QK_TOGGLE_LAYER_MAX) ||
                     (keycode >= QK_TO && keycode <= QK_TO_MAX) ||
                     (keycode >= QK_LAYER_MOD && keycode <= QK_LAYER_MOD_MAX)) {
                should_update_rgb = true;
            }
            // Check specific layer-tap keys from your layout
            else if ((keycode >= LT(0,KC_NO) && keycode <= LT(7,KC_NO))) {
                should_update_rgb = true;
            }
            break;
    }
    
    // Update RGB immediately when modifier or layer keys are pressed/released
    if (should_update_rgb) {
        // Small delay to ensure state is updated
        wait_ms(1);
        update_rgb_for_current_state();
    }
    
    return true;
}

// Also update RGB during matrix scan for real-time updates
void matrix_scan_user(void) {
    static uint8_t last_mods = 0;
    static layer_state_t last_layer_state = 0;
    static bool last_caps_lock_state = false;
    
    uint8_t current_mods = get_mods() | get_weak_mods() | get_oneshot_mods();
    layer_state_t current_layer_state = layer_state;
    bool current_caps_lock_state = host_keyboard_led_state().caps_lock;
    
    // Update RGB if mods, layer state, or caps lock changed
    if (current_mods != last_mods || 
        current_layer_state != last_layer_state ||
        current_caps_lock_state != last_caps_lock_state) {
        update_rgb_for_current_state();
        last_mods = current_mods;
        last_layer_state = current_layer_state;
        last_caps_lock_state = current_caps_lock_state;
    }
}

// Handle caps lock changes
bool led_update_user(led_t led_state) {
    // Update RGB when caps lock state changes
    update_rgb_for_current_state();
    return true;
}
#endif