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

# Set the MCU accordingly to your device (e.g. at90usb1286 for a Teensy 2.0++, or atmega16u2 for an Arduino UNO R3)
MCU          = atmega32u4
ARCH         = AVR8
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s

CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -IConfig/
LD_FLAGS     =

# Default target
all:

# Set necessary variables for generic makefile
# Name of target (binary and derivatives)
TARGET       = Joystick
# Where to search for source files (.cpp)
SRC          = ./src/$(TARGET).c ./src/Descriptors.c $(LUFA_SRC_USB)
SOURCE_ROOT:=./src/
# Where FastArduino project is located (used to find library and includes)
FASTARDUINO_ROOT=../fast-arduino-lib
LUFA_PATH    = ../LUFA/LUFA
# Additional paths containing includes (usually empty)
ADDITIONAL_INCLUDES:=
# Additional paths containing libraries other than fastarduino (usually empty)
ADDITIONAL_LIBS:=


# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
# include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
#include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk

# include generic makefile for apps
include $(FASTARDUINO_ROOT)/Makefile-app.mk
