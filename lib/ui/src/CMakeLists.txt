SET(SOURCES screens/ui_Screen1.c
    screens/ui_Screen2.c
    screens/ui_Screen3.c
    screens/ui_Screen4.c
    ui.c
    ui_helpers.c
    images/ui_img_1862871016.c
    images/ui_img_lamp_png.c
    images/ui_img_801059491.c
    images/ui_img_2047347433.c
    images/ui_img_30409_treecanopy_v1_920485_png.c)

add_library(ui ${SOURCES})
