TARGET_CHIP := NRF51822_QFAA_CA
BOARD := BOARD_PCA10001

C_SOURCE_FILES += main_gui.c
C_SOURCE_FILES += radio_config.c
C_SOURCE_FILES += nrf_delay.c
C_SOURCE_FILES += spi.c
C_SOURCE_FILES += data_fifo.c
C_SOURCE_FILES += command_fifo.c

OUTPUT_FILENAME := gui_radio
<<<<<<< HEAD
SDK_PATH = ../../../../../
=======
SDK_PATH = ../../../../
>>>>>>> a259999c7a4f9ab26c34ea22f445b5d858ce36ef


#Uncomment correct line if you have s110 programmed on the chip.
DEVICE_VARIANT := xxaa
#DEVICE_VARIANT := xxab

#USE_SOFTDEVICE := s110
#USE_SOFTDEVICE := s210

include $(SDK_PATH)Source/templates/gcc/Makefile.common
