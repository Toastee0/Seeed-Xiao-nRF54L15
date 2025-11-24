// ============================================================================
// Watchface Demo — v1.0.2 POSTED  (nRF54L15 + Zephyr/NCS + LVGL 9)
// - BIG (green) & LITTLE (blue) faces from BIG.c / LITTLE.c
// - Ornate hands; white date, per-face placement (BIG:+81,0 / LITTLE:+84,0)
// - Tap center to toggle; auto-rotate every 30s
// - Time/day seeded from __TIME__/__DATE__
// - FIX: Zephyr-safe boot (void main), staged timers after first refresh
// - FINAL PJG Tessted 10/4/25 8:32 PM
// ============================================================================

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>   // MIN()
#include <lvgl.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* LVGL image symbols (must match BIG.c / LITTLE.c) */
LV_IMG_DECLARE(BIG);
LV_IMG_DECLARE(LITTLE);

/* -------- Layout -------- */
static uint16_t scr_w, scr_h, cx, cy;
static uint16_t R_outer, R_hour, R_min, R_sec;

/* Per-face date centers (offsets from screen center) */
#define DATE_BIG_DX   81
#define DATE_BIG_DY    0
#define DATE_LIT_DX   87   /* I'm changing this one in the video demo */
#define DATE_LIT_DY    0

/* Date text color: WHITE for both faces */
#define DATE_COLOR_HEX 0xFFFFFF

/* -------- Face & objects -------- */
static lv_obj_t *face_img;
static bool      face_is_big = true;

static lv_obj_t *hour_outline, *hour_inlay;
static lv_obj_t *min_outline,  *min_inlay;
static lv_obj_t *sec_line, *sec_counter;
static lv_obj_t *cap_obj;

static lv_point_precise_t hour_pts[2], min_pts[2], sec_pts[2], sec_cw_pts[2];

static lv_obj_t *lbl_date = NULL;

/* -------- Time base -------- */
static uint32_t g_secs = 0;   /* seconds since midnight */
static uint8_t  g_day  = 1;   /* 1..31 */

/* -------- Helpers -------- */
static inline void polar_to_xy(float deg, float r, lv_point_precise_t *out)
{
    float rad = (deg * (float)M_PI) / 180.0f;
    out->x = (lv_coord_t)(cx + r * cosf(rad));
    out->y = (lv_coord_t)(cy - r * sinf(rad)); /* +y is down */
}

/* Place date centered over the per-face window (force layout first) */
static void date_reposition_for_face(void)
{
    if (!lbl_date) return;
    lv_obj_update_layout(lbl_date);  /* ensure size is fresh */

    const int x = cx + (face_is_big ? DATE_BIG_DX : DATE_LIT_DX);
    const int y = cy + (face_is_big ? DATE_BIG_DY : DATE_LIT_DY);

    lv_obj_set_pos(lbl_date,
                   x - (lv_obj_get_width(lbl_date)  / 2),
                   y - (lv_obj_get_height(lbl_date) / 2));
}

/* Show numeric day; keeps WHITE for both faces and re-centers */
static void date_show(uint8_t day)
{
    if (!lbl_date) return;

    lv_obj_set_style_text_color(lbl_date, lv_color_hex(DATE_COLOR_HEX), 0);

    char buf[3];
    buf[0] = '0' + (day / 10);
    buf[1] = '0' + (day % 10);
    buf[2] = '\0';
    lv_label_set_text(lbl_date, buf);

    date_reposition_for_face();
}

/* Swap face and re-center date (used at boot, timer, and tap) */
static void set_face(bool big)
{
    face_is_big = big;
    if (face_img) {
        lv_img_set_src(face_img, face_is_big ? &BIG : &LITTLE);
        lv_obj_center(face_img);
    }
    date_reposition_for_face();
}

/* Ornate hands update */
static void hands_update_from_secs(uint32_t secs)
{
    uint32_t s = secs % 60;
    uint32_t m = (secs / 60) % 60;
    uint32_t h = (secs / 3600) % 12;

    float sec_deg  = 90.0f - (s * 6.0f);
    float min_deg  = 90.0f - (m * 6.0f + s * 0.1f);
    float hour_deg = 90.0f - (h * 30.0f + m * 0.5f);

    /* Hour */
    hour_pts[0].x = cx; hour_pts[0].y = cy;
    polar_to_xy(hour_deg, (float)R_hour, &hour_pts[1]);
    if (hour_outline) lv_line_set_points(hour_outline, hour_pts, 2);
    if (hour_inlay)   lv_line_set_points(hour_inlay,   hour_pts, 2);

    /* Minute */
    min_pts[0].x = cx; min_pts[0].y = cy;
    polar_to_xy(min_deg, (float)R_min, &min_pts[1]);
    if (min_outline) lv_line_set_points(min_outline, min_pts, 2);
    if (min_inlay)   lv_line_set_points(min_inlay,   min_pts, 2);

    /* Second + short counterweight */
    sec_pts[0].x = cx; sec_pts[0].y = cy;
    polar_to_xy(sec_deg, (float)R_sec, &sec_pts[1]);
    if (sec_line) lv_line_set_points(sec_line, sec_pts, 2);

    float cw_deg = sec_deg + 180.0f;
    sec_cw_pts[0].x = cx; sec_cw_pts[0].y = cy;
    polar_to_xy(cw_deg, (float)(R_sec * 0.18f), &sec_cw_pts[1]);
    if (sec_counter) lv_line_set_points(sec_counter, sec_cw_pts, 2);

    if (cap_obj) lv_obj_set_pos(cap_obj, cx - 5, cy - 5);  /* keep cap centered */
}

/* Seed from compile time/date (demo-grade; no RTC) */
static void seed_from_build(uint32_t *out_secs, uint8_t *out_day)
{
    int hh = 12, mm = 0, ss = 0;
    (void)sscanf(__TIME__, "%d:%d:%d", &hh, &mm, &ss);
    *out_secs = (uint32_t)(hh * 3600 + mm * 60 + ss);

    const char *date = __DATE__;        /* e.g., "Oct  4 2025" */
    char day_str[3] = { date[4], date[5], '\0' };
    int d = atoi(day_str);
    if (d < 1 || d > 31) d = 1;
    *out_day = (uint8_t)d;
}

/* -------- Timers & events -------- */

/* 1 Hz: advance time, update hands, roll date at midnight */
static void tick_cb(lv_timer_t *t)
{
    ARG_UNUSED(t);
    g_secs = (g_secs + 1) % (24 * 3600);
    hands_update_from_secs(g_secs);

    if (g_secs == 0) {
        g_day++;
        if (g_day > 31) g_day = 1;   /* demo-grade month roll */
        date_show(g_day);
    }
}

/* 30 s: flip faces */
static void flip_cb(lv_timer_t *t)
{
    ARG_UNUSED(t);
    set_face(!face_is_big);
    lv_refr_now(NULL);
}

/* tap center: toggle face */
static void center_tap_cb(lv_event_t *e)
{
    ARG_UNUSED(e);
    set_face(!face_is_big);
    lv_refr_now(NULL);
}

/* -------- Entry (Zephyr-style) -------- */
void main(void)
{
    printk("Watch demo v1.0.2 POSTED start\n");

    /* --- Display bring-up (white wipe to reset GRAM) --- */
    const struct device *disp = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(disp)) {
        printk("Display not ready\n");
        return;
    }
    (void)display_blanking_off(disp);

    /* Quick white wipe (line-by-line; no large malloc) */
    {
        uint16_t line[240];
        for (int x = 0; x < 240; x++) line[x] = 0xFFFF; /* RGB565 white */
        struct display_buffer_descriptor desc = {
            .width = 240, .height = 1, .pitch = 240, .buf_size = sizeof(line),
        };
        for (int y = 0; y < 240; y++) {
            (void)display_write(disp, 0, y, &desc, (const void *)line);
        }
    }

    /* --- LVGL stage --- */
    lv_display_t *ld = lv_display_get_default();
    scr_w = lv_display_get_horizontal_resolution(ld);
    scr_h = lv_display_get_vertical_resolution(ld);
    cx = scr_w / 2;  cy = scr_h / 2;

    R_outer = MIN(scr_w, scr_h) / 2 - 6;
    R_hour  = (uint16_t)(R_outer * 0.58f);
    R_min   = (uint16_t)(R_outer * 0.80f);
    R_sec   = (uint16_t)(R_outer * 0.86f);

    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Face image object */
    face_img = lv_img_create(scr);

    /* Date label — WHITE for both faces */
    lbl_date = lv_label_create(scr);
    lv_obj_set_style_text_color(lbl_date, lv_color_hex(DATE_COLOR_HEX), 0);
    // If you enable a larger font in prj.conf, you could set it here:
    // lv_obj_set_style_text_font(lbl_date, &lv_font_montserrat_20, 0);

    /* Hands (outline + inlay) */
    hour_outline = lv_line_create(scr);
    lv_obj_set_style_line_width(hour_outline, 10, 0);
    lv_obj_set_style_line_color(hour_outline, lv_color_hex(0x202020), 0);
    lv_obj_set_style_line_rounded(hour_outline, true, 0);

    hour_inlay = lv_line_create(scr);
    lv_obj_set_style_line_width(hour_inlay, 6, 0);
    lv_obj_set_style_line_color(hour_inlay, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_line_rounded(hour_inlay, true, 0);

    min_outline = lv_line_create(scr);
    lv_obj_set_style_line_width(min_outline, 8, 0);
    lv_obj_set_style_line_color(min_outline, lv_color_hex(0x202020), 0);
    lv_obj_set_style_line_rounded(min_outline, true, 0);

    min_inlay = lv_line_create(scr);
    lv_obj_set_style_line_width(min_inlay, 4, 0);
    lv_obj_set_style_line_color(min_inlay, lv_color_hex(0xE6E6E6), 0);
    lv_obj_set_style_line_rounded(min_inlay, true, 0);

    sec_line = lv_line_create(scr);
    lv_obj_set_style_line_width(sec_line, 2, 0);
    lv_obj_set_style_line_color(sec_line, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_line_rounded(sec_line, true, 0);

    sec_counter = lv_line_create(scr);
    lv_obj_set_style_line_width(sec_counter, 3, 0);
    lv_obj_set_style_line_color(sec_counter, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_line_rounded(sec_counter, true, 0);

    /* Center cap */
    cap_obj = lv_obj_create(scr);
    lv_obj_remove_style_all(cap_obj);
    lv_obj_set_style_bg_color(cap_obj, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(cap_obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(cap_obj, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_size(cap_obj, 10, 10);
    lv_obj_set_pos(cap_obj, cx - 5, cy - 5);

    /* Invisible center button to catch taps (no KSCAN needed) */
    lv_obj_t *tap = lv_btn_create(scr);
    lv_obj_remove_style_all(tap);         /* invisible */
    lv_obj_set_size(tap, 120, 120);       /* hit area */
    lv_obj_center(tap);
    lv_obj_add_flag(tap, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(tap, center_tap_cb, LV_EVENT_SHORT_CLICKED, NULL);

    /* --- ORDER MATTERS: set face FIRST, then apply date text, then hands --- */
    set_face(true);                 /* start on BIG; centers date for BIG */
    seed_from_build(&g_secs, &g_day);
    date_show(g_day);               /* sets text and re-centers again (safe) */
    hands_update_from_secs(g_secs);

    /* One immediate refresh to ensure all sizes/layouts are finalized */
    lv_refr_now(NULL);
    k_msleep(20);                   /* small settle delay before timers */

    /* Timers: 1Hz hands + 30s flip (created after first render) */
    (void)lv_timer_create(tick_cb, 1000, NULL);
    (void)lv_timer_create(flip_cb, 30000, NULL);

    /* Pump LVGL in case LVGL threads are configured differently */
    while (1) {
        lv_timer_handler();
        k_msleep(5);
    }
}
