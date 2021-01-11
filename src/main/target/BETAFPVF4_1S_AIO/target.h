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

#define TARGET_BOARD_IDENTIFIER "BEFH"
#define USBD_PRODUCT_STRING     "BETAFPVF411RX"

#define USE_TARGET_CONFIG

#define LED0_PIN                PC14

#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN              PA3

#define USE_BEEPER
//#define BEEPER_INVERTED

//spi
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15
#define USE_SPI_DEVICE_3
#define SPI3_NSS_PIN            PA15
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5

//GYRO
#define MPU6000_CS_PIN          PA4
#define MPU6000_SPI_INSTANCE    SPI1

#define USE_EXTI
#define MPU_INT_EXTI            PB6
#define USE_MPU_DATA_READY_SIGNAL

#define USE_GYRO
#define USE_GYRO_SPI_MPU6000
#define GYRO_MPU6000_ALIGN      CW0_DEG_FLIP
#define USE_ACC
#define USE_ACC_SPI_MPU6000
#define ACC_MPU6000_ALIGN       CW0_DEG_FLIP

// *************** UART *****************************
#define USE_VCP
#define USE_USB_DETECT

#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_SOFTSERIAL1
#define SOFTSERIAL1_TX_PIN      PA8
#define SOFTSERIAL1_RX_PIN      PA8

#define SERIAL_PORT_COUNT       3

#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART1

// *************** OSD *****************************
#define USE_MAX7456
#define MAX7456_SPI_INSTANCE    SPI2
#define MAX7456_SPI_CS_PIN      PB12

//RX
#define RX_SCK_PIN                    SPI3_SCK_PIN
#define RX_MISO_PIN                   SPI3_MISO_PIN
#define RX_MOSI_PIN                   SPI3_MOSI_PIN
#define RX_NSS_PIN                    SPI3_NSS_PIN
#define RX_NSS_GPIO_CLK_PERIPHERAL    RCC_APB2Periph_GPIOA

#define USE_RX_SPI
#define RX_SPI_INSTANCE               SPI3
#define RX_CC2500_SPI_DISABLE_CHIP_DETECTION
#define RX_CC2500_SPI_GDO_0_PIN       PC13
#define RX_CC2500_SPI_LED_PIN         PC15
#define RX_CC2500_SPI_LED_PIN_INVERTED
#define RX_FRSKY_SPI_LED_PIN_INVERTED
#define USE_RX_CC2500_SPI_PA_LNA
#define RX_CC2500_SPI_TX_EN_PIN       PB9
#define RX_CC2500_SPI_LNA_EN_PIN      PA13
#define USE_RX_CC2500_SPI_DIVERSITY
#define RX_CC2500_SPI_ANT_SEL_PIN     PA14
#define BINDPLUG_PIN                  PB2
#define USE_RX_FRSKY_SPI_D
#define USE_RX_FRSKY_SPI_X
#define USE_RX_SFHSS_SPI
#define DEFAULT_RX_FEATURE            FEATURE_RX_SPI
#define RX_SPI_DEFAULT_PROTOCOL       RX_SPI_FRSKY_X
#define USE_RX_FRSKY_SPI_TELEMETRY

// *************** ADC *****************************
#define USE_ADC
#define ADC1_DMA_STREAM         DMA2_Stream0
#define VBAT_ADC_PIN            PA1
#define CURRENT_METER_ADC_PIN   PB0

#define DEFAULT_FEATURES                (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_SOFTSERIAL)
#define DEFAULT_VOLTAGE_METER_SOURCE    VOLTAGE_METER_ADC
#define DEFAULT_CURRENT_METER_SOURCE    CURRENT_METER_ADC

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define USABLE_TIMER_CHANNEL_COUNT 8
#define USED_TIMERS             ( TIM_N(1)|TIM_N(2)|TIM_N(3)|TIM_N(4)|TIM_N(5) )
