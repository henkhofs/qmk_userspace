// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "halcyon.h"
#include "transactions.h"
#include "split_util.h"
#include "_wait.h"

__attribute__((weak)) void module_suspend_power_down_kb(void);
__attribute__((weak)) void module_suspend_wakeup_init_kb(void);

__attribute__((weak)) bool module_post_init_kb(void) {
    return module_post_init_user();
}
__attribute__((weak)) bool module_housekeeping_task_kb(void) {
    return module_housekeeping_task_user();
}
__attribute__((weak)) bool display_module_housekeeping_task_kb(bool second_display) {
    return display_module_housekeeping_task_user(second_display);
}

__attribute__((weak)) bool module_post_init_user(void) {
    return true;
}
__attribute__((weak)) bool module_housekeeping_task_user(void) {
    return true;
}
__attribute__((weak)) bool display_module_housekeeping_task_user(bool second_display) {
    return true;
}

module_t module_master;
module_t module;
#ifdef HLC_NONE
module_t module = hlc_none;
#endif
#ifdef HLC_CIRQUE_TRACKPAD
module_t module = hlc_cirque_trackpad;
#endif
#ifdef HLC_ENCODER
module_t module = hlc_encoder;
#endif
#ifdef HLC_TFT_DISPLAY
module_t module = hlc_tft_display;
#endif

bool backlight_off = false;

// Timeout handling
void backlight_wakeup(void) {
    backlight_off = false;
    backlight_enable();
    if (get_backlight_level() == 0) {
        backlight_level(BACKLIGHT_LEVELS);
    }
}

// Timeout handling
void backlight_suspend(void) {
    backlight_off = true;
    backlight_disable();
}

// Last time the slave received a module sync, doubles as a heartbeat
static volatile uint32_t last_heartbeat = 0;
static volatile bool     heartbeat_seen = false;

void module_sync_slave_handler(uint8_t initiator2target_buffer_size, const void *initiator2target_buffer, uint8_t target2initiator_buffer_size, void *target2initiator_buffer) {
    if (initiator2target_buffer_size == sizeof(module)) {
        memcpy(&module_master, initiator2target_buffer, sizeof(module_master));
        last_heartbeat = timer_read32();
        heartbeat_seen = true;
    }
}

void suspend_power_down_kb(void) {
    module_suspend_power_down_kb();

    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {
    module_suspend_wakeup_init_kb();

    suspend_wakeup_init_user();
}

void keyboard_post_init_kb(void) {
    // Register module sync split transaction
    transaction_register_rpc(MODULE_SYNC, module_sync_slave_handler);

    // Do any post init for modules
    module_post_init_kb();

    // User post init
    keyboard_post_init_user();
}

void housekeeping_task_kb(void) {
    if (is_keyboard_master()) {
        static bool     synced    = false;
        static uint32_t last_sync = 0;
        static uint32_t lost_since = 0;
        static bool     lost       = false;

        if (is_transport_connected()) {
            lost = false;
            // Resync periodically so the slave knows we are alive and gets module info back after a reset
            if (!synced || timer_elapsed32(last_sync) > HLC_SPLIT_HEARTBEAT_INTERVAL) {
                transaction_rpc_send(MODULE_SYNC, sizeof(module), &module); // Sync to slave
                last_sync = timer_read32();
                if (!synced) {
                    wait_ms(10);
                    // Good moment to make sure the backlight wakes up after boot for both halves
                    backlight_wakeup();
                    synced = true;
                }
            }
        } else if (synced) {
            // Split link was up but is gone (e.g. after host sleep/wake), reset like a replug would
            if (!lost) {
                lost_since = timer_read32();
                lost       = true;
            } else if (timer_elapsed32(lost_since) > HLC_SPLIT_RECOVERY_TIMEOUT) {
                mcu_reset();
            }
        }

        display_module_housekeeping_task_kb(false); // Is master so can never be the second display
    }

    if (!is_keyboard_master()) {
        display_module_housekeeping_task_kb(module_master == hlc_tft_display);

        // Master went quiet after having talked to us, reset to recover the split link
        if (heartbeat_seen && timer_elapsed32(last_heartbeat) > HLC_SPLIT_RECOVERY_TIMEOUT) {
            mcu_reset();
        }
    }

    // Backlight feature
    if (HLC_BACKLIGHT_TIMEOUT != 0) {
        if (last_input_activity_elapsed() <= HLC_BACKLIGHT_TIMEOUT) {
            if (backlight_off) {
                backlight_wakeup();
            }
        } else {
            if (!backlight_off) {
                backlight_suspend();
            }
        }
    }

    module_housekeeping_task_kb();

    housekeeping_task_user();
}

report_mouse_t pointing_device_task_combined_kb(report_mouse_t left_report, report_mouse_t right_report) {
    // Only runs on master
    // Fixes the following bug: If master is right and master is NOT a cirque trackpad, the inputs would be inverted.
    if (module != hlc_cirque_trackpad && !is_keyboard_left()) {
        mouse_xy_report_t x = left_report.x;
        mouse_xy_report_t y = left_report.y;
        left_report.x       = -x;
        left_report.y       = -y;
    }
    return pointing_device_task_combined_user(left_report, right_report);
}
