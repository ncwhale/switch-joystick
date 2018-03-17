#
#             LUFA Library
#     Copyright (C) Dean Camera, 2014.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

# Run "make help" for target help.

# Library path.
LUFA_PATH    = ../LUFA/LUFA
# BOOST_ROOT = ../boost_1_66_0
FASTARDUINO_ROOT=../fast-arduino-lib

# Set the MCU accordingly to your device (e.g. at90usb1286 for a Teensy 2.0++, or atmega16u2 for an Arduino UNO R3)
MCU          = atmega32u4
ARCH         = AVR8
F_CPU        = 16000000
F_USB        = $(F_CPU)
VARIANT:=ARDUINO_LEONARDO
PROGRAMMER:=LEONARDO
ADDITIONAL_INCLUDES:= -I../LUFA/

# Default target
all:

SOURCE_ROOT:=./src/
# include $(FASTARDUINO_ROOT)/Makefile-config.mk
# include $(FASTARDUINO_ROOT)/Makefile-common.mk
# include $(FASTARDUINO_ROOT)/Makefile-app.mk

# Build flags
OPTIMIZATION = s
SRC          = $(shell find $(SOURCE_ROOT) -name "*.c*") $(LUFA_SRC_USB)
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -IConfig/
#-I$(BOOST_ROOT)/ $(includes)
LD_FLAGS     = 
#-L$(abspath $(FASTARDUINO_ROOT)/dist/ARDUINO_LEONARDO-16MHz/) -lfastarduino 
#$(basename $(fastarduinolib)) -L$()

TARGET       = Joystick


# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
# include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk

