F411_TARGETS += $(TARGET)
FEATURES     += VCP
TARGET_SRC  =                                       \
             drivers/accgyro/accgyro_spi_bmi160.c \
             drivers/nbd7456.c                      \
             drivers/rx/rx_cc2500.c                 \
             rx/cc2500_common.c                     \
             rx/cc2500_sfhss.c                      \
             rx/cc2500_frsky_shared.c               \
             rx/cc2500_frsky_d.c                    \
             rx/cc2500_frsky_x.c
