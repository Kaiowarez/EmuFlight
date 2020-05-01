/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */
 #include <stdint.h>

 #include "platform.h"
 #include "drivers/io.h"

 #include "drivers/dma.h"
 #include "drivers/timer.h"
 #include "drivers/timer_def.h"

const timerHardware_t timerHardware[USABLE_TIMER_CHANNEL_COUNT] = {
    //{ TIM12, IO_TAG(PB14), TIM_Channel_1, TIM_USE_PPM,   0, GPIO_AF_TIM12, NULL,  0, 0, },    // PPM_IN
    //{ TIM5,  IO_TAG(PA2),  TIM_Channel_3, TIM_USE_MOTOR, 1, GPIO_AF_TIM5,  DMA1_Stream0, DMA_Channel_6, DMA1_ST0_HANDLER}, // S1_OUT
    //{ TIM5,  IO_TAG(PA3),  TIM_Channel_4, TIM_USE_MOTOR, 1, GPIO_AF_TIM5,  DMA1_Stream1, DMA_Channel_6, DMA1_ST1_HANDLER}, // S2_OUT
    //{ TIM1,  IO_TAG(PA10), TIM_Channel_3, TIM_USE_MOTOR, 1, GPIO_AF_TIM1,  DMA2_Stream6, DMA_Channel_0, DMA2_ST6_HANDLER}, // S3_OUT
    //{ TIM2,  IO_TAG(PA15), TIM_Channel_1, TIM_USE_MOTOR, 1, GPIO_AF_TIM2,  DMA1_Stream5, DMA_Channel_3, DMA1_ST5_HANDLER}, // S4_OUT
    //{ TIM8,  IO_TAG(PC8),  TIM_Channel_3, TIM_USE_MOTOR, 1, GPIO_AF_TIM8,  DMA2_Stream2, DMA_Channel_0, DMA2_ST2_HANDLER}, // S5_OUT
    //{ TIM3,  IO_TAG(PB0),  TIM_Channel_3, TIM_USE_MOTOR, 1, GPIO_AF_TIM3,  DMA1_Stream7, DMA_Channel_5, DMA1_ST7_HANDLER}, // S6_OUT
    //{ TIM11, IO_TAG(PB9),  TIM_Channel_1, TIM_USE_ANY, 1,   GPIO_AF_TIM11, NULL,         NULL,          NULL},             // Camera Control
    DEF_TIM(TIM12, CH1, PB14, TIM_USE_PPM,   0, 0), // PPM In
    DEF_TIM(TIM5,  CH3, PA2,  TIM_USE_MOTOR, 0, 0), // S1
    DEF_TIM(TIM5,  CH4, PA3,  TIM_USE_MOTOR, 0, 0), // S2
    DEF_TIM(TIM1,  CH3, PA10, TIM_USE_MOTOR, 0, 0), // S3
    DEF_TIM(TIM2,  CH1, PA15, TIM_USE_MOTOR, 0, 0), // S4
    DEF_TIM(TIM8,  CH3, PC8,  TIM_USE_MOTOR, 0, 0), // S5
    DEF_TIM(TIM3,  CH3, PB0,  TIM_USE_MOTOR, 0, 0), // S6
    DEF_TIM(TIM11, CH1, PB9,  TIM_USE_ANY,   0, 0),   // Camera Control
};
