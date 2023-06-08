// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.0
// LVGL version: 8.3.6
// Project name: test

#ifndef _TEST_UI_H
#define _TEST_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined __has_include
#if __has_include("lvgl.h")
#include "lvgl.h"
#elif __has_include("lvgl/lvgl.h")
#include "lvgl/lvgl.h"
#else
#include "lvgl.h"
#endif
#else
#include "lvgl.h"
#endif

#include "ui_helpers.h"
#include "ui_events.h"
// SCREEN: ui_Screen1
void ui_Screen1_screen_init(void);
void ui_event_Screen1(lv_event_t * e);
extern lv_obj_t * ui_Screen1;
extern lv_obj_t * ui_sc1MainPanel;
extern lv_obj_t * ui_secondButton;
extern lv_obj_t * ui_Label1;
extern lv_obj_t * ui_Image1;
extern lv_obj_t * ui_firstButton;
extern lv_obj_t * ui_Label2;
extern lv_obj_t * ui_Image2;
extern lv_obj_t * ui_thirdButton;
extern lv_obj_t * ui_Image3;
extern lv_obj_t * ui_Label4;
extern lv_obj_t * ui_forthButton;
extern lv_obj_t * ui_Image5;
extern lv_obj_t * ui_Label5;
// SCREEN: ui_Screen2
void ui_Screen2_screen_init(void);
void ui_event_Screen2(lv_event_t * e);
extern lv_obj_t * ui_Screen2;
extern lv_obj_t * ui_sc2BrightnessPanel;
void ui_event_BrightnessSlider(lv_event_t * e);
extern lv_obj_t * ui_BrightnessSlider;
extern lv_obj_t * ui_BrightnessTi;
extern lv_obj_t * ui_Label6;
extern lv_obj_t * ui_sc2VolPanel;
void ui_event_muteButton(lv_event_t * e);
extern lv_obj_t * ui_muteButton;
extern lv_obj_t * ui_muteT;
void ui_event_lowButton(lv_event_t * e);
extern lv_obj_t * ui_lowButton;
extern lv_obj_t * ui_lowT;
void ui_event_highButton(lv_event_t * e);
extern lv_obj_t * ui_highButton;
extern lv_obj_t * ui_hightT;
extern lv_obj_t * ui_volTi;
extern lv_obj_t * ui_sc2SystemPanel;
void ui_event_SysInfoBut(lv_event_t * e);
extern lv_obj_t * ui_SysInfoBut;
extern lv_obj_t * ui_sysInfoT;
void ui_event_Button2(lv_event_t * e);
extern lv_obj_t * ui_Button2;
extern lv_obj_t * ui_Label7;
// SCREEN: ui_Screen3
void ui_Screen3_screen_init(void);
extern lv_obj_t * ui_Screen3;
extern lv_obj_t * ui_sc3MainPanel;
extern lv_obj_t * ui_sysSpec;
void ui_event_backButton(lv_event_t * e);
extern lv_obj_t * ui_backButton;
extern lv_obj_t * ui_backT;
// SCREEN: ui_Screen4
void ui_Screen4_screen_init(void);
extern lv_obj_t * ui_Screen4;
extern lv_obj_t * ui_Colorwheel1;
void ui_event_Button1(lv_event_t * e);
extern lv_obj_t * ui_Button1;
extern lv_obj_t * ui_Label8;
extern lv_obj_t * ui____initial_actions0;

LV_IMG_DECLARE(ui_img_1862871016);    // assets\wallpapersden.com_beautiful-scenery-mountains-lake_480x320.png
LV_IMG_DECLARE(ui_img_lamp_png);    // assets\lamp.png
LV_IMG_DECLARE(ui_img_801059491);    // assets\curtains (1).png
LV_IMG_DECLARE(ui_img_2047347433);    // assets\air-conditioner.png
LV_IMG_DECLARE(ui_img_30409_treecanopy_v1_920485_png);    // assets\30409_treecanopy_v1_920485.png

void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
