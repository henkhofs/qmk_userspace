// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "qp.h"
#include "qp_surface.h"

// All values (including hue) are scaled to 0-255
#define HSV_SPLITKB 145, 235, 155
#define HSV_CAPS_OFF 17, 104, 77
#define HSV_CAPS_ON 17, 191, 245
#define HSV_SCROLL_OFF 202, 104, 77
#define HSV_SCROLL_ON 202, 191, 245
#define HSV_NUM_OFF 142, 104, 77
#define HSV_NUM_ON 142, 191, 245
#define HSV_CTRL_OFF 30, 104, 77
#define HSV_CTRL_ON 30, 191, 245
#define HSV_SHIFT_OFF 240, 104, 77
#define HSV_SHIFT_ON 240, 191, 245
#define HSV_ALT_OFF 180, 104, 77
#define HSV_ALT_ON 180, 191, 245
#define HSV_GUI_OFF 120, 104, 77
#define HSV_GUI_ON 120, 191, 245
#define HSV_WPM_SLOW 0, 104, 77
#define HSV_WPM_MEDIUM 180, 191, 245
#define HSV_WPM_FAST 60, 191, 245

#define HSV_LAYER_0 0, 0, 120
#define HSV_LAYER_1 30, 255, 200
#define HSV_LAYER_2 180, 255, 200
#define HSV_LAYER_3 0, 255, 200
#define HSV_LAYER_4 60, 255, 200
#define HSV_LAYER_5 220, 255, 200
#define HSV_LAYER_6 120, 255, 200
#define HSV_LAYER_7 210, 255, 200
#define HSV_LAYER_UNDEF 0, 255, 255

extern painter_device_t lcd;
extern painter_device_t lcd_surface;

void draw_grid(void);
void update_grid(void);
void init_grid(void);
void add_cell_cluster(void);
uint8_t get_random_color_index(void);
void update_display(void);
void backlight_wakeup(void);
void backlight_suspend(void);
