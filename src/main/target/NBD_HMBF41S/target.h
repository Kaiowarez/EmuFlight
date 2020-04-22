/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define TARGET_BOARD_IDENTIFIER "HMBF4"
#define USBD_PRODUCT_STRING     "HMBF41S_FRSKY_US"

#define USE_LED_STRIP
#define LED0_PIN                PC13
#define LED1_PIN                PC14

#define USE_BEEPER
#define BEEPER_PIN              PB10
#define BEEPER_INVERTED

// *************** SPI1 Gyro & ACC **********************

#define USE_SPI

#define USE_SPI_DEVICE_2
#define SPI2_NSS_PIN            PB12
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5
#define SPI3_NSS_PIN            PA15

#define MPU6000_CS_PIN          PA4
#define MPU6000_SPI_INSTANCE    SPI3

#define USE_GYRO
#define USE_GYRO_SPI_MPU6000
#define USE_ACC
#define USE_ACC_SPI_MPU6000
#define ACC_MPU6000_ALIGN       CW90_DEG

// *************** OSD *****************************

#define USE_MAX7456
#define MAX7456_SPI_INSTANCE    SPI3
#define MAX7456_SPI_CS_PIN      PA15

// *************** CC2500 ***************************

#define RX_SCK_PIN              SPI2_SCK_PIN
#define RX_MISO_PIN             SPI2_MISO_PIN
#define RX_MOSI_PIN             SPI2_MOSI_PIN
#define RX_NSS_PIN              SPI2_NSS_PIN

// #define RX_NSS_GPIO_CLK_PERIPHERAL   RCC_APB2Periph_GPIOA

#define USE_RX_SPI
#define RX_SPI_INSTANCE           SPI2

#define RX_CC2500_SPI_DISABLE_CHIP_DETECTION
#define RX_CC2500_SPI_GDO_0_PIN   PB2
#define RX_CC2500_SPI_LED_PIN     PA13
#define RX_CC2500_SPI_LED_PIN_INVERTED
#define RX_FRSKY_SPI_LED_PIN_INVERTED
#define RX_CC2500_SPI_TX_EN_PIN   PB10

#define USE_RX_CC2500_SPI_DIVERSITY
#define RX_CC2500_SPI_ANT_SEL_PIN PA7

#define BINDPLUG_PIN              PC15

#define USE_RX_FRSKY_SPI_D
#define USE_RX_FRSKY_SPI_X
#define DEFAULT_RX_FEATURE      FEATURE_RX_SPI
#define RX_SPI_DEFAULT_PROTOCOL RX_SPI_FRSKY_X
#define USE_RX_FRSKY_SPI_TELEMETRY

// *************** UART *****************************
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN              PA9
#define UART1_RX_PIN              PA10

#define USE_UART2
#define UART2_TX_PIN              PA2
#define UART2_RX_PIN              PA3

#define SERIAL_PORT_COUNT         3

// *************** ADC *****************************
#define USE_ADC
#define ADC1_DMA_STREAM           DMA2_Stream0
#define VBAT_ADC_PIN              PB1
#define CURRENT_METER_ADC_PIN     PA5


#define DEFAULT_FEATURES                (FEATURE_OSD | FEATURE_TELEMETRY )
#define DEFAULT_VOLTAGE_METER_SOURCE    VOLTAGE_METER_ADC
#define DEFAULT_CURRENT_METER_SOURCE    CURRENT_METER_ADC
#define CURRENT_METER_SCALE_DEFAULT 179

#define TARGET_IO_PORTA           0xffff
#define TARGET_IO_PORTB           0xffff
#define TARGET_IO_PORTC           0xffff
#define TARGET_IO_PORTD           (BIT(2))

//#define  USE_DSHOT_DMA

#define USABLE_TIMER_CHANNEL_COUNT 5
#define USED_TIMERS             ( TIM_N(4) | TIM_N(5) )
