#!/usr/bin/make
# The name of your project (used to name the compiled .hex file)
#TARGET = $(notdir $(CURDIR))
#
TARGET = regbot

# The teensy version to use, 30 or 31 or 35 or 36 (teensy 3.2 is the same as 3.1)
#TEENSY = 41
TEENSY = 31
#TEENSY = 36

# Some libraries will require this to be defined
# If you define this, you will break the default main.cpp
ARDUINO = 105

# configurable options
OPTIONS = -DUSB_SERIAL -DLAYOUT_US_ENGLISH -DTEENSY$(TEENSY) -DTEENSYDUINO

# directory to build in
BUILDDIR = $(CURDIR)/build

#************************************************************************
# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.
# To use this makefile without Arduino, copy the resources from these
# locations and edit the pathnames.  The rest of Arduino is not needed.
#************************************************************************

# path location for Teensy Loader, teensy_post_compile and teensy_reboot
#TOOLSPATH = $(CURDIR)/tools
BASEPATH = .
TOOLSPATH = $(BASEPATH)/tools

# path location for Teensy 3 core
ifeq ($(TEENSY), 41)
	COREPATH = $(BASEPATH)/teensy4
else
	COREPATH = $(BASEPATH)/teensy3
endif

# path location for Arduino libraries
#LIBRARYPATH = libraries
LIBRARYPATH = $(BASEPATH)/libraries

# path location for the arm-none-eabi compiler
#COMPILERPATH = $(TOOLSPATH)/arm-none-eabi/bin
COMPILERPATH = $(TOOLSPATH)/arm/bin
#COMPILERPATH = /usr/bin

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPPFLAGS = compiler Pre-Process options for C and C++
CPPFLAGS = -Wall -g0 -O3 -mcpu=cortex-m4 -mthumb -nostdlib -MMD $(OPTIONS) \
      -DF_CPU=$(TEENSY_CORE_SPEED) -I$(BASEPATH)/src -I$(COREPATH) -I Snooze/ \
      -I$(LIBRARYPATH)/SD/src -I$(LIBRARYPATH)/SdFat/src 

# compiler options for C++ only
#CXXFLAGS = -std=gnu++0x -g0 -O3 -felide-constructors -fno-exceptions -fno-rtti -Wno-c++14-compat
CXXFLAGS = -std=gnu++17 -g0 -O3 -felide-constructors -fno-exceptions -fno-rtti


# compiler options for C only
CFLAGS =

# compiler options specific to teensy version
# ldscript must be one of mk20dx128.ld  mk20dx256.ld  mk64fx512.ld  mk66fx1m0.ld  mkl26z64.ld
ifeq ($(TEENSY), 30)
    CPPFLAGS += -D__MK20DX128__
    LDSCRIPT = $(COREPATH)/mk20dx128.ld
    TEENSY_CORE_SPEED = 48000000
else
    ifeq ($(TEENSY), 31)
        CPPFLAGS += -D__MK20DX256__ 
        LDSCRIPT = $(COREPATH)/mk20dx256.ld
        TEENSY_CORE_SPEED = 96000000
    else
      ifeq ($(TEENSY), 35)
        CPPFLAGS += -D__MK64FX512__ -DREGBOT_HW4
        LDSCRIPT = $(COREPATH)/mk64fx512.ld
        TEENSY_CORE_SPEED = 120000000
      else
	ifeq ($(TEENSY), 36)
		CPPFLAGS += -D__MK66FX1M0__
		LDSCRIPT = $(COREPATH)/mk66fx1m0.ld
		TEENSY_CORE_SPEED = 180000000
	else
  	    ifeq ($(TEENSY), 40)
		CPPFLAGS += -D__IMXRT1062__ -DARDUINO_TEENSY40
		LDSCRIPT = $(COREPATH)/imxrt1062.ld
		TEENSY_CORE_SPEED = 600000000
	    else
		ifeq ($(TEENSY), 41)
			CPPFLAGS += -D__IMXRT1062__ -DARDUINO_TEENSY41 -DREGBOT_HW41
			LDSCRIPT = $(COREPATH)/imxrt1062_t41.ld
			TEENSY_CORE_SPEED = 200000000
		else
			$(error Invalid setting for TEENSY)
		endif
	    endif
	endif
      endif
    endif
endif

# set arduino define if given
ifdef ARDUINO
	CPPFLAGS += -DARDUINO=$(ARDUINO)
endif

# linker options
LDFLAGS = -Os -Wl,--gc-sections -mcpu=cortex-m4 -mthumb -T$(LDSCRIPT)

# additional libraries to link
LIBS = -lm
# names for the compiler programs
# CC = $(COMPILERPATH)/arm-none-eabi-gcc.exe
# CXX = $(COMPILERPATH)/arm-none-eabi-g++.exe
# OBJCOPY = $(COMPILERPATH)/arm-none-eabi-objcopy.exe
# SIZE = $(COMPILERPATH)/arm-none-eabi-size.exe
CC = $(COMPILERPATH)/arm-none-eabi-gcc
CXX = $(COMPILERPATH)/arm-none-eabi-g++
# CC = arm-none-eabi-gcc
# CXX = arm-none-eabi-g++
OBJCOPY = $(COMPILERPATH)/arm-none-eabi-objcopy
SIZE = $(COMPILERPATH)/arm-none-eabi-size
# OBJCOPY = arm-none-eabi-objcopy
# SIZE = arm-none-eabi-size

# automatically create lists of the sources and objects
# LC_FILES := $(wildcard ./Snooze/utility/*.c)
#LCPP_FILES := $(wildcard $(LIBRARYPATH)/*/*.cpp)
# LCPP2_FILES := $(wildcard $(LIBRARYPATH)/i2c_t3/*.cpp)
ifeq ($(TEENSY), 41)
	LCPP2_FILES := $(wildcard $(LIBRARYPATH)/Wire/*.cpp) \
               $(wildcard $(LIBRARYPATH)/ADC/*.cpp) \
	       $(wildcard ./libraries/SPI/*.cpp) \
               $(LIBRARYPATH)/Servo/Servo.cpp \
               $(wildcard $(LIBRARYPATH)/NXPMotionSense/*.cpp) \
               $(wildcard $(LIBRARYPATH)/Adafruit-GFX-Library/*.cpp) \
               $(wildcard $(LIBRARYPATH)/Adafruit_SSD1306/*.cpp)
else
	LCPP2_FILES := $(wildcard $(LIBRARYPATH)/i2c_t3/*.cpp) \
               $(wildcard $(LIBRARYPATH)/ADC/*.cpp) \
	       $(wildcard ./libraries/SPI/*.cpp) \
               $(LIBRARYPATH)/Servo/Servo.cpp  \
               $(wildcard $(LIBRARYPATH)/NXPMotionSense/*.cpp)
endif
TC_FILES := $(wildcard $(COREPATH)/*.c) $(wildcard $(LIBRARYPATH)/SD/utility/*.c)
TCPP_FILES := $(wildcard $(COREPATH)/*.cpp)
#C_FILES := $(wildcard src/*.c)
CPP_FILES := $(wildcard $(BASEPATH)/src/*.cpp) $(wildcard $(BASEPATH)/*.cpp)
#INO_FILES := $(wildcard src/*.ino)

# include paths for libraries
L_INC := $(foreach lib,$(filter %/, $(wildcard $(LIBRARYPATH)/*/)), -I$(lib))
#L_INC += -I$(BASEPATH)/src/adafruit/Adafruit-GFX-Library -I$(BASEPATH)/src/adafruit/Adafruit_SSD1306 -I$(BASEPATH)/src/adafruit/Adafruit_BusIO


#SOURCES := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o) $(INO_FILES:.ino=.o) $(TC_FILES:.c=.o) $(TCPP_FILES:.cpp=.o) $(LC_FILES:.c=.o) $(LCPP_FILES:.cpp=.o)
SOURCES := $(CPP_FILES:.cpp=.o) $(TC_FILES:.c=.o) $(TCPP_FILES:.cpp=.o) $(LC_FILES:.c=.o) $(LCPP_FILES:.cpp=.o) $(LCPP2_FILES:.cpp=.o)

OBJS := $(foreach src,$(SOURCES), $(BUILDDIR)/$(src))

all: $(TARGET).hex

build: $(TARGET).elf

hex: $(TARGET).hex

post_compile: $(TARGET).hex
	@$ $(TOOLSPATH)/teensy_post_compile -file=$(basename $<) -path=$(CURDIR) -tools=$(TOOLSPATH)

reboot:
	@-$ $(TOOLSPATH)/teensy_reboot

upload: post_compile reboot

$(BUILDDIR)/%.o: %.c
	@echo "[CC]\t$<"
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(L_INC) -o $@ -c $<

$(BUILDDIR)/%.o: %.cpp
	@echo "[CXX]\t$<"
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o $@ -c $<

$(BUILDDIR)/%.o: %.ino
	@echo "[CXX]\t$<"
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o $@ -x c++ -include Arduino.h -c $<

$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@echo "[LD]\t$@"
	@$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.hex: %.elf
	@echo "[HEX]\t$@"
	@$(SIZE) $<
	@$(OBJCOPY) -O ihex -R .eeprom $< $@

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	@echo Cleaning...
	@rm -rf $(BUILDDIR)
	@rm -f $(TARGET).elf $(TARGET).hex
