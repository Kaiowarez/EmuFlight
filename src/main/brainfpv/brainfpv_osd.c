/**
 ******************************************************************************
 * @addtogroup OnScreenDisplay OSD Module
 * @brief Process OSD information
 *
 *
 * @file       brainfpv_osd.c
 * @author     dRonin, http://dronin.org Copyright (C) 2015-2016
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2013-2014
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2014.
 * @brief      OSD gen module, handles OSD draw. Parts from CL-OSD and SUPEROSD projects
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Additional note on redistribution: The copyright and license notices above
 * must be maintained in each individual source file that is a derivative work
 * of this source file; otherwise redistribution is prohibited.
 */


#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "brainfpv_osd.h"
#include "ch.h"
#include "video.h"
#include "images.h"
#include "osd_utils.h"
#include "spectrograph.h"

#include "common/maths.h"
#include "common/axis.h"
#include "common/color.h"
#include "common/utils.h"
#include "common/printf.h"
#include "common/typeconversion.h"

#include "drivers/sensor.h"
#include "drivers/system.h"
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "drivers/time.h"
#include "drivers/light_led.h"
#include "drivers/light_ws2811strip.h"
#include "drivers/sound_beeper.h"
#include "drivers/max7456.h"
#include "drivers/max7456_symbols.h"

#include "sensors/sensors.h"
#include "sensors/boardalignment.h"
#include "sensors/compass.h"
#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/gyro.h"
#include "sensors/battery.h"

#include "io/flashfs.h"
#include "io/gimbal.h"
#include "io/gps.h"
#include "io/ledstrip.h"
#include "io/serial.h"
#include "io/beeper.h"
#include "io/osd.h"

#include "telemetry/telemetry.h"

#include "flight/mixer.h"
#include "flight/failsafe.h"
#include "flight/imu.h"

#include "config/feature.h"
#include "pg/pg.h"
#include "pg/pg_ids.h"
#include "pg/vcd.h"
#include "pg/max7456.h"

#include "fc/rc_controls.h"
#include "rx/rx.h"
#include "fc/runtime_config.h"
#include "fc/rc_modes.h"


#include "cms/cms.h"

#include "rx/crsf.h"
#include "rx/rx.h"
#include "pg/rx.h"


#if defined(USE_BRAINFPV_OSD) | 1

PG_REGISTER_WITH_RESET_TEMPLATE(bfOsdConfig_t, bfOsdConfig, PG_BRAINFPV_CONFIG, 0);

PG_RESET_TEMPLATE(bfOsdConfig_t, bfOsdConfig,
  .sync_threshold = BRAINFPV_OSD_SYNC_TH_DEFAULT,
  .white_level    = 110,
  .black_level    = 20,
  .x_offset       = 0,
  .x_scale        = 8,
  .sbs_3d_enabled = 0,
  .sbs_3d_right_eye_offset = 30,
  .font = 0,
  .ir_system = 0,
  .ir_trackmate_id = 0,
  .ir_ilap_id = 0,
  .ahi_steps = 2,
  .bmi160foc = 0,
  .bmi160foc_ret = 0,
  .altitude_scale = 1,
  .speed_scale = 1,
  .map = 1,
  .map_max_dist_m = 500,
  .sticks_display = 0,
  .spec_enabled = 0,
  .show_logo_on_arm = 1,
  .show_pilot_logo = 1,
  .invert = 0,
  .hd_frame = 0,
  .hd_frame_width = 100,
  .hd_frame_height = 55,
  .hd_frame_h_offset = 0,
  .hd_frame_v_offset = 0,
  .crsf_link_stats = 1,
  .crsf_link_stats_power = 1,
  .crsf_link_stats_rssi = CRSF_LQ_LOW,
  .crsf_link_stats_snr = CRSF_SNR_LOW,
  .crsf_link_stats_snr_threshold = -2
);

const char * const gitTag = __GIT_TAG__;

void video_qspi_enable(void);
extern binary_semaphore_t onScreenDisplaySemaphore;

extern uint8_t *draw_buffer;
extern uint8_t *disp_buffer;

extern bool cmsInMenu;
bool osd_arming_or_stats = false;
extern bool osdStatsVisible;
bool osdArming = false;

extern crsfLinkInfo_t crsf_link_info;
static bool show_crsf_link_info;

bool brainfpv_user_avatar_set = false;
bool brainfpv_hd_frame_menu = false;
extern bfOsdConfig_t bfOsdConfigCms;

static void simple_artificial_horizon(int16_t roll, int16_t pitch, int16_t x, int16_t y,
        int16_t width, int16_t height, int8_t max_pitch,
        uint8_t n_pitch_steps);
void draw_stick(int16_t x, int16_t y, int16_t horizontal, int16_t vertical);
void draw_map_uav_center();
void draw_hd_frame(const bfOsdConfig_t * config);


enum BrainFPVOSDMode {
    MODE_BETAFLIGHT,
    MODE_SPEC,
};

/*******************************************************************************/
// MAX7456 Emulation
#define MAX_X(x) (x * 12)
#define MAX_Y(y) (y * 18)

uint16_t maxScreenSize = VIDEO_BUFFER_CHARS_PAL;


static uint8_t bf_font(void)
{
    uint8_t font = bfOsdConfig()->font;

    if (font >= NUM_FONTS) {
        font = NUM_FONTS - 1;
    }

    return font;
}

bool max7456Init(const struct max7456Config_s *max7456Config, const struct vcdProfile_s *vcdProfile, bool cpuOverclock)
{
    (void)max7456Config;
    (void)vcdProfile;
    (void)cpuOverclock;

    return true;
}

void max7456Invert(bool invert)
{
    (void)invert;
}

void max7456Brightness(uint8_t black, uint8_t white)
{
    (void)black;
    (void)white;
}

bool max7456DmaInProgress(void)
{
    return false;
}

void max7456DrawScreen(void)
{}

void  max7456WriteNvm(uint8_t char_address, const uint8_t *font_data)
{
    (void)char_address; (void)font_data;
}

uint8_t max7456GetRowsCount(void)
{
    if (Video_GetType() == VIDEO_TYPE_NTSC)
        return VIDEO_LINES_NTSC;
    else
        return VIDEO_LINES_PAL;
}

void max7456Write(uint8_t x, uint8_t y, const char *buff)
{

    write_string(buff, MAX_X(x), MAX_Y(y), 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, bf_font());
}

void max7456WriteChar(uint8_t x, uint8_t y, uint8_t c)
{
    char buff[2] = {c, 0};

    write_string(buff, MAX_X(x), MAX_Y(y), 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, bf_font());
}

void max7456ClearScreen(void)
{
    clearGraphics();
}

void  max7456RefreshAll(void)
{
}

static uint8_t dummyBuffer[VIDEO_BUFFER_CHARS_PAL+40];
uint8_t* max7456GetScreenBuffer(void)
{
    return dummyBuffer;
}

bool max7456BuffersSynced(void)
{
    return true;
}

/*******************************************************************************/

void brainFpvOsdInit(void)
{
    for (uint16_t i=0; i<(image_userlogo.width * image_userlogo.height) / 4; i++) {
        if (image_userlogo.data[i] != 0) {
            brainfpv_user_avatar_set = true;
            break;
        }
    }
    if (bfOsdConfig()->crsf_link_stats && (rxConfig()->serialrx_provider == SERIALRX_CRSF)) {
        show_crsf_link_info = true;
    }
    else {
        show_crsf_link_info = false;
    }
}

void brainFpvOsdWelcome(void)
{
    char string_buffer[100];

#define GY (GRAPHICS_BOTTOM / 2 - 30)
    brainFpvOsdMainLogo(GRAPHICS_X_MIDDLE, GY);

    tfp_sprintf(string_buffer, "BF VERSION: %s", gitTag);
    write_string(string_buffer, GRAPHICS_X_MIDDLE, GRAPHICS_BOTTOM - 60, 0, 0, TEXT_VA_TOP, TEXT_HA_CENTER, FONT8X10);
    write_string("MENU: THRT MID YAW LEFT PITCH UP", GRAPHICS_X_MIDDLE, GRAPHICS_BOTTOM - 35, 0, 0, TEXT_VA_TOP, TEXT_HA_CENTER, FONT8X10);
#if defined(USE_BRAINFPV_SPECTROGRAPH)
    if (bfOsdConfig()->spec_enabled) {
        write_string("SPECT: THRT MID YAW RIGHT PITCH UP", GRAPHICS_X_MIDDLE, GRAPHICS_BOTTOM - 25, 0, 0, TEXT_VA_TOP, TEXT_HA_CENTER, FONT8X10);
    }
#endif
}

static int32_t getAltitude(void)
{
    int32_t alt = baro.BaroAlt;
    switch (osdConfig()->units) {
        case OSD_UNIT_IMPERIAL:
            return (alt * 328) / 100; // Convert to feet / 100
        default:
            return alt;               // Already in metre / 100
    }
}

static float getVelocity(void)
{
    float vel = gpsSol.groundSpeed;

    switch (osdConfig()->units) {
        case OSD_UNIT_IMPERIAL:
            return CM_S_TO_MPH(vel);
        default:
            return CM_S_TO_KM_H(vel);
        }
    // Unreachable
    return -1.0f;
}


void osdUpdateLocal()
{
    if (bfOsdConfig()->altitude_scale && sensors(SENSOR_BARO)) {
        float altitude = getAltitude() / 100.f;
        osd_draw_vertical_scale(altitude, 100, 1, GRAPHICS_RIGHT - 20, GRAPHICS_Y_MIDDLE, 120, 10, 20, 5, 8, 11, 0);
    }

    if (sensors(SENSOR_GPS)) {
        if (bfOsdConfig()->speed_scale) {
            float speed = getVelocity();
            osd_draw_vertical_scale(speed, 100, -1, GRAPHICS_LEFT + 5, GRAPHICS_Y_MIDDLE, 120, 10, 20, 5, 8, 11, 0);
        }
        if (bfOsdConfig()->map) {
            draw_map_uav_center();
        }
    }

    if (bfOsdConfig()->sticks_display == 1) {
        // Mode 2
        draw_stick(GRAPHICS_LEFT + 30, GRAPHICS_BOTTOM - 30, rcData[YAW], rcData[THROTTLE]);
        draw_stick(GRAPHICS_RIGHT - 30, GRAPHICS_BOTTOM - 30, rcData[ROLL], rcData[PITCH]);
    }
    else if (bfOsdConfig()->sticks_display == 2) {
        // Mode 1
        draw_stick(GRAPHICS_LEFT + 30, GRAPHICS_BOTTOM - 30, rcData[YAW], rcData[PITCH]);
        draw_stick(GRAPHICS_RIGHT - 30, GRAPHICS_BOTTOM - 30, rcData[ROLL], rcData[THROTTLE]);
    }

    if (bfOsdConfig()->hd_frame) {
        draw_hd_frame(bfOsdConfig());
    }
}



#define IS_HI(X)  (rcData[X] > 1750)
#define IS_LO(X)  (rcData[X] < 1250)
#define IS_MID(X) (rcData[X] > 1250 && rcData[X] < 1750)

void osdMain(void) {
    uint32_t key_time = 0;
    uint32_t currentTime;

    while (1) {
        if (chBSemWaitTimeout(&onScreenDisplaySemaphore, MS2ST(500)) == MSG_TIMEOUT) {
            // No trigger received within 500ms, re-enable the video
            video_qspi_enable();
        }

        currentTime = micros();
        enum SpecCommand spec_command = SPEC_COMMAND_NONE;
        static enum BrainFPVOSDMode mode = MODE_BETAFLIGHT;
        clearGraphics();

        /* Hide OSD when OSDSW mode is active */
        if (IS_RC_MODE_ACTIVE(BOXOSD))
          continue;

 #if defined(USE_BRAINFPV_SPECTROGRAPH)
        if (bfOsdConfig()->spec_enabled) {
            if (IS_MID(THROTTLE) && IS_HI(YAW) && IS_HI(PITCH) && !ARMING_FLAG(ARMED)) {
                mode = MODE_SPEC;
            }
            else {
                if ((mode == MODE_SPEC) && !ARMING_FLAG(ARMED)) {
                    if (IS_HI(ROLL) && ((millis() - key_time) > 250)) {
                        spec_command = SPEC_COMMAND_SWAXIS;
                        key_time = millis();
                    }
                    if (IS_LO(ROLL)) {
                        mode = MODE_BETAFLIGHT;
                    }
                }
            }
        }
 #endif /* defined(USE_BRAINFPV_SPECTROGRAPH) */

        if (millis() < 7000) {
            brainFpvOsdWelcome();
        }
        else {
            switch (mode) {
                case MODE_BETAFLIGHT:
                    osdUpdate(currentTime);
                    if (!cmsInMenu && !osd_arming_or_stats){
                        osdUpdateLocal();
                    }
                    if (cmsInMenu && brainfpv_hd_frame_menu) {
                        draw_hd_frame(&bfOsdConfigCms);
                    }
                    break;
                case MODE_SPEC:
 #if defined(USE_BRAINFPV_SPECTROGRAPH)
                    spectrographOSD(spec_command);
 #endif /* defined(USE_BRAINFPV_SPECTROGRAPH) */
                    break;
                default:
                    break;
            }
        }
    }
}

void brainFpvOsdArtificialHorizon(void)
{
    simple_artificial_horizon(attitude.values.roll, -1 * attitude.values.pitch,
                              GRAPHICS_X_MIDDLE, GRAPHICS_Y_MIDDLE,
                              GRAPHICS_BOTTOM * 0.8f, GRAPHICS_RIGHT * 0.8f, 30,
                              bfOsdConfig()->ahi_steps);
}

#define CENTER_BODY       3
#define CENTER_WING       7
#define CENTER_RUDDER     5
#define PITCH_STEP       10
void brainFpvOsdCenterMark(void)
{
    write_line_outlined(GRAPHICS_X_MIDDLE - CENTER_WING - CENTER_BODY, GRAPHICS_Y_MIDDLE ,
            GRAPHICS_X_MIDDLE - CENTER_BODY, GRAPHICS_Y_MIDDLE, 2, 0, 0, 1);
    write_line_outlined(GRAPHICS_X_MIDDLE + 1 + CENTER_BODY, GRAPHICS_Y_MIDDLE,
            GRAPHICS_X_MIDDLE + 1 + CENTER_BODY + CENTER_WING, GRAPHICS_Y_MIDDLE, 0, 2, 0, 1);
    write_line_outlined(GRAPHICS_X_MIDDLE, GRAPHICS_Y_MIDDLE - CENTER_RUDDER - CENTER_BODY, GRAPHICS_X_MIDDLE,
            GRAPHICS_Y_MIDDLE - CENTER_BODY, 2, 0, 0, 1);
}


static void simple_artificial_horizon(int16_t roll, int16_t pitch, int16_t x, int16_t y,
        int16_t width, int16_t height, int8_t max_pitch, uint8_t n_pitch_steps)
{
    width /= 2;
    height /= 2;

    float sin_roll = sinf(DECIDEGREES_TO_RADIANS(roll));
    float cos_roll = cosf(DECIDEGREES_TO_RADIANS(roll));

    int pitch_step_offset = pitch / (PITCH_STEP * 10);

    /* how many degrees the "lines" are offset from their ideal pos
         * since we need both, don't do fmodf.. */
    float modulo_pitch =DECIDEGREES_TO_DEGREES(pitch) - pitch_step_offset * 10.0f;

    // roll to pitch transformation
    int16_t pp_x = x + width * ((sin_roll * modulo_pitch) / (float)max_pitch);
    int16_t pp_y = y + height * ((cos_roll * modulo_pitch) / (float)max_pitch);

    int16_t d_x, d_x2; // delta x
    int16_t d_y, d_y2; // delta y

    d_x = cos_roll * width / 2;
    d_y = sin_roll * height / 2;

    d_x = 3 * d_x / 4;
    d_y = 3 * d_y / 4;
    d_x2 = 3 * d_x / 4;
    d_y2 = 3 * d_y / 4;

    int16_t d_x_10 = width * sin_roll * PITCH_STEP / (float)max_pitch;
    int16_t d_y_10 = height * cos_roll * PITCH_STEP / (float)max_pitch;

    int16_t d_x_2 = d_x_10 / 6;
    int16_t d_y_2 = d_y_10 / 6;

    for (int i = (-max_pitch / 10)-1; i<(max_pitch/10)+1; i++) {
        int angle = (pitch_step_offset + i);

        if (angle < -n_pitch_steps) continue;
        if (angle > n_pitch_steps) continue;

        angle *= PITCH_STEP;

        /* Wraparound */
        if (angle > 90) {
            angle = 180 - angle;
        } else if (angle < -90) {
            angle = -180 - angle;
        }

        int16_t pp_x2 = pp_x - i * d_x_10;
        int16_t pp_y2 = pp_y - i * d_y_10;

        char tmp_str[5];

        tfp_sprintf(tmp_str, "%d", angle);

        if (angle < 0) {
            write_line_outlined_dashed(pp_x2 - d_x2, pp_y2 + d_y2, pp_x2 + d_x2, pp_y2 - d_y2, 2, 2, 0, 1, 5);
            write_line_outlined(pp_x2 - d_x2, pp_y2 + d_y2, pp_x2 - d_x2 - d_x_2, pp_y2 + d_y2 - d_y_2, 2, 2, 0, 1);
            write_line_outlined(pp_x2 + d_x2, pp_y2 - d_y2, pp_x2 + d_x2 - d_x_2, pp_y2 - d_y2 - d_y_2, 2, 2, 0, 1);

            write_string(tmp_str, pp_x2 - d_x - 4, pp_y2 + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, FONT_OUTLINED8X8);
            write_string(tmp_str, pp_x2 + d_x + 4, pp_y2 - d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, FONT_OUTLINED8X8);
        } else if (angle > 0) {
            write_line_outlined(pp_x2 - d_x2, pp_y2 + d_y2, pp_x2 + d_x2, pp_y2 - d_y2, 2, 2, 0, 1);
            write_line_outlined(pp_x2 - d_x2, pp_y2 + d_y2, pp_x2 - d_x2 + d_x_2, pp_y2 + d_y2 + d_y_2, 2, 2, 0, 1);
            write_line_outlined(pp_x2 + d_x2, pp_y2 - d_y2, pp_x2 + d_x2 + d_x_2, pp_y2 - d_y2 + d_y_2, 2, 2, 0, 1);

            write_string(tmp_str, pp_x2 - d_x - 4, pp_y2 + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, FONT_OUTLINED8X8);
            write_string(tmp_str, pp_x2 + d_x + 4, pp_y2 - d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, FONT_OUTLINED8X8);
        } else {
            write_line_outlined(pp_x2 - d_x, pp_y2 + d_y, pp_x2 - d_x / 3, pp_y2 + d_y / 3, 2, 2, 0, 1);
            write_line_outlined(pp_x2 + d_x / 3, pp_y2 - d_y / 3, pp_x2 + d_x, pp_y2 - d_y, 2, 2, 0, 1);
        }
    }
}



#define FIX_RC_RANGE(x) (MIN(MAX(-500, x - 1500), 500))
#define STICK_WIDTH 2
#define STICK_LENGTH 20
#define STICK_BOX_SIZE 4
#define STICK_MOVEMENT_EXTENT (STICK_LENGTH - STICK_BOX_SIZE / 2 + 1)

void draw_stick(int16_t x, int16_t y, int16_t horizontal, int16_t vertical)
{

    write_filled_rectangle_lm(x - STICK_LENGTH, y - STICK_WIDTH / 2, 2 * STICK_LENGTH, STICK_WIDTH, 0, 1);
    write_filled_rectangle_lm(x - STICK_WIDTH / 2, y - STICK_LENGTH, STICK_WIDTH, 2 * STICK_LENGTH, 0, 1);

    write_hline_lm(x - STICK_LENGTH - 1, x - STICK_WIDTH / 2 -1, y - STICK_WIDTH / 2 - 1, 1, 1);
    write_hline_lm(x - STICK_LENGTH - 1, x - STICK_WIDTH / 2 -1, y + STICK_WIDTH / 2 + 1, 1, 1);

    write_hline_lm(x + STICK_WIDTH / 2 + 1, x + STICK_LENGTH + 1, y - STICK_WIDTH / 2 - 1, 1, 1);
    write_hline_lm(x + STICK_WIDTH / 2 + 1, x + STICK_LENGTH + 1, y + STICK_WIDTH / 2 + 1, 1, 1);

    write_hline_lm(x - STICK_WIDTH / 2 -1, x + STICK_WIDTH / 2 + 1 , y - STICK_LENGTH -1, 1, 1);
    write_hline_lm(x - STICK_WIDTH / 2 -1, x + STICK_WIDTH / 2 + 1 , y + STICK_LENGTH + 1, 1, 1);

    write_vline_lm(x - STICK_WIDTH / 2 - 1, y - STICK_WIDTH / 2 - 1, y - STICK_LENGTH -1, 1, 1);
    write_vline_lm(x + STICK_WIDTH / 2 + 1, y - STICK_WIDTH / 2 - 1, y - STICK_LENGTH -1, 1, 1);

    write_vline_lm(x - STICK_WIDTH / 2 - 1, y + STICK_LENGTH  + 1, y + STICK_WIDTH / 2 + 1, 1, 1);
    write_vline_lm(x + STICK_WIDTH / 2 + 1, y + STICK_LENGTH  + 1, y + STICK_WIDTH / 2 + 1, 1, 1);

    write_vline_lm(x - STICK_LENGTH - 1, y -STICK_WIDTH / 2 -1, y + STICK_WIDTH / 2 + 1, 1, 1);
    write_vline_lm(x + STICK_LENGTH + 1, y -STICK_WIDTH / 2 -1, y + STICK_WIDTH / 2 + 1, 1, 1);

    int16_t stick_x =  x + (STICK_MOVEMENT_EXTENT * FIX_RC_RANGE(horizontal)) / 500.f;
    int16_t stick_y =  y - (STICK_MOVEMENT_EXTENT * FIX_RC_RANGE(vertical)) / 500.f;

    write_filled_rectangle_lm(stick_x - (STICK_BOX_SIZE) / 2 - 1, stick_y - (STICK_BOX_SIZE) / 2 - 1, STICK_BOX_SIZE + 2, STICK_BOX_SIZE + 2, 0, 1);
    write_filled_rectangle_lm(stick_x - (STICK_BOX_SIZE) / 2, stick_y - (STICK_BOX_SIZE) / 2, STICK_BOX_SIZE, STICK_BOX_SIZE, 1, 1);
}


void brainFpvOsdUserLogo(uint16_t x, uint16_t y)
{
    draw_image(MAX_X(x) - image_userlogo.width / 2, MAX_Y(y) - image_userlogo.height / 2, &image_userlogo);
}

void brainFpvOsdMainLogo(uint16_t x, uint16_t y)
{
    draw_image(x - image_mainlogo.width / 2, y - image_mainlogo.height / 2, &image_mainlogo);
}


const point_t HOME_ARROW[] = {
    {
        .x = 0,
        .y = -10,
    },
    {
        .x = 9,
        .y = 1,
    },
    {
        .x = 3,
        .y = 1,
    },
    {
        .x = 3,
        .y = 8,
    },
    {
        .x = -3,
        .y = 8,
    },
    {
        .x = -3,
        .y = 1,
    },
    {
        .x = -9,
        .y = 1,
    }
};

void brainFfpvOsdHomeArrow(int home_dir, uint16_t x, uint16_t y)
{
    x = MAX_X(x);
    y = MAX_Y(y);
    draw_polygon(x, y, home_dir, HOME_ARROW, 7, 0, 1);
}

#define MAP_MAX_DIST_PX 70
void draw_map_uav_center()
{
    uint16_t x, y;

    uint16_t dist_to_home_m = GPS_distanceToHome;

    if (dist_to_home_m > bfOsdConfig()->map_max_dist_m) {
        dist_to_home_m = bfOsdConfig()->map_max_dist_m;
    }

    float dist_to_home_px = MAP_MAX_DIST_PX * (float)dist_to_home_m / (float)bfOsdConfig()->map_max_dist_m;

    // don't draw map if we are very close to home
    if (dist_to_home_px < 1.0f) {
        return;
    }

    // Get home direction relative to UAV
    int16_t home_dir = GPS_directionToHome - DECIDEGREES_TO_DEGREES(attitude.values.yaw);

    x = GRAPHICS_X_MIDDLE + roundf(dist_to_home_px * sinf(home_dir * (float)(M_PI / 180)));
    y = GRAPHICS_Y_MIDDLE - roundf(dist_to_home_px * cosf(home_dir * (float)(M_PI / 180)));

    // draw H to indicate home
    write_string("H", x + 1, y - 3, 0, 0, TEXT_VA_TOP, TEXT_HA_CENTER, FONT_OUTLINED8X8);
}

#define HD_FRAME_CORNER_LEN 10
void draw_hd_frame(const bfOsdConfig_t * config)
{
    uint16_t x1, x2;
    uint16_t y1, y2;

    if (config->hd_frame == 0) {
        return;
    }

    x1 = GRAPHICS_X_MIDDLE - config->hd_frame_width - config->hd_frame_h_offset;
    if (x1 > GRAPHICS_RIGHT) {
        x1 = 0;
    }

    y1 = GRAPHICS_Y_MIDDLE - config->hd_frame_height + config->hd_frame_v_offset;
    if (y1 > GRAPHICS_BOTTOM) {
        y1 = 0;
    }

    x2 = x1 + 2 * config->hd_frame_width;
    y2 = y1 + 2 * config->hd_frame_height;

    switch (config->hd_frame) {
        case 1:
            // FULL frame
            write_hline_lm(x1, x2, y1, 1, 1);
            write_hline_lm(x1, x2, y2, 1, 1);
            write_vline_lm(x1, y1, y2, 1, 1);
            write_vline_lm(x2, y1, y2, 1, 1);
            break;
        default:
            // Corners
            write_hline_lm(x1, x1 + HD_FRAME_CORNER_LEN, y1, 1, 1);
            write_hline_lm(x2 - HD_FRAME_CORNER_LEN, x2, y1, 1, 1);

            write_hline_lm(x1, x1 + HD_FRAME_CORNER_LEN, y2, 1, 1);
            write_hline_lm(x2 - HD_FRAME_CORNER_LEN, x2, y2, 1, 1);

            write_vline_lm(x1, y1, y1 + HD_FRAME_CORNER_LEN, 1, 1);
            write_vline_lm(x1, y2 - HD_FRAME_CORNER_LEN, y2, 1, 1);

            write_vline_lm(x2, y1, y1 + HD_FRAME_CORNER_LEN, 1, 1);
            write_vline_lm(x2, y2 - HD_FRAME_CORNER_LEN, y2, 1, 1);
            break;
    }
}

#define CRSF_LINE_SPACING 12

bool osdElementRssi_BrainFPV(uint16_t x_pos, uint16_t y_pos)
{
    if (!show_crsf_link_info) {
        return false;
    }
    else {
        char tmp_str[20];
        bool show;

        x_pos = MAX_X(x_pos);
        y_pos = MAX_Y(y_pos);

        tfp_sprintf(tmp_str, "%c%d", SYM_RSSI, crsf_link_info.lq);
        write_string(tmp_str, x_pos, y_pos, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, bf_font());
        y_pos += 16;

        if (bfOsdConfig()->crsf_link_stats_power) {
            tfp_sprintf(tmp_str, "%dmW", crsf_link_info.tx_power);
            write_string(tmp_str, x_pos, y_pos, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, FONT8X10);
            y_pos += CRSF_LINE_SPACING;
        }

        switch (bfOsdConfig()->crsf_link_stats_rssi) {
            case CRSF_OFF:
                show = false;
                break;
            case CRSF_LQ_LOW:
                show = (crsf_link_info.lq <= osdConfig()->rssi_alarm);
                break;
            case CRSF_SNR_LOW:
                show =  (crsf_link_info.snr <= bfOsdConfig()->crsf_link_stats_snr_threshold);
                break;
            case CRSF_ON:
                show = true;
                break;
        }

        if (show) {
            tfp_sprintf(tmp_str, "%ddBm", -1 * (int16_t)crsf_link_info.rssi);
            write_string(tmp_str, x_pos, y_pos, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, FONT8X10);
            y_pos += CRSF_LINE_SPACING;
        }

        switch (bfOsdConfig()->crsf_link_stats_snr) {
            case CRSF_OFF:
                show = false;
                break;
            case CRSF_LQ_LOW:
                show = (crsf_link_info.lq <= osdConfig()->rssi_alarm);
                break;
            case CRSF_SNR_LOW:
                show =  (crsf_link_info.snr <= bfOsdConfig()->crsf_link_stats_snr_threshold);
                break;
            case CRSF_ON:
                show = true;
                break;
        }

        if (show) {
            tfp_sprintf(tmp_str, "SN %ddB", crsf_link_info.snr);
            write_string(tmp_str, x_pos, y_pos, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, FONT8X10);
        }

        // set the RSSI for other things in betaflight
        setRssiCrsfLq(crsf_link_info.lq);
    }

    return true;
}

#endif /* USE_BRAINFPV_OSD */













