## General Flags
PROJECT = jack
MCU = atmega8
BURNMCU = atmega8
BURNPROGRAMMER = usbasp
TARGET = output/$(PROJECT).elf
CC = avr-gcc
CCXX = avr-g++

## Flags common to C, ASM, and Linker
COMMON = -mmcu=$(MCU)

## Flags common to C only
CFLAGS = $(COMMON)
CONLYFLAGS = -std=gnu99
CFLAGS += -Wall -gdwarf-2 -DF_CPU=12000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections
CFLAGS += -MD -MP -MT $(*F).o

## Flags common to ASM only
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2
ASMFLAGS += 

## Flags common to CPP/CXX only
CXXFLAGS = $(COMMON)
CXXFLAGS += $(CFLAGS)
CXXFLAGS += -std=c99

## Flags common to Linker only
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map=output/$(PROJECT).map
LDFLAGS += -Wl,--gc-sections

## Flags for Intel HEX file production
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Include Directories
INCLUDES = -I"."

## Libraries
LIBS = -lm -lc

## Link these object files to be made
OBJECTS = main.o usbdrv.o usbdrvasm.o

## Link objects specified by users
LINKONLYOBJECTS = 

## Compile

all: $(TARGET)

main.o: ./main.c
	 $(CC) $(INCLUDES) $(CFLAGS) $(CONLYFLAGS) -c  $<

usbdrv.o: ./usbdrv/usbdrv.c
	 $(CC) $(INCLUDES) $(CFLAGS) $(CONLYFLAGS) -c  $<

usbdrvasm.o: ./usbdrv/usbdrvasm.S
	 $(CC) $(INCLUDES) $(ASMFLAGS) -c  $<



## Link
$(TARGET): $(OBJECTS)
	-rm -rf $(TARGET) output/$(PROJECT).map
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)
	-rm -rf $(OBJECTS) main.d usbdrv.d usbdrvasm.d 
	-rm -rf output/$(PROJECT).hex output/$(PROJECT).eep output/$(PROJECT).lss
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS) $(TARGET) output/$(PROJECT).hex
	avr-objcopy $(HEX_FLASH_FLAGS) -O ihex $(TARGET) output/$(PROJECT).eep || exit 0
	avr-objdump -h -S $(TARGET) >> output/$(PROJECT).lss
	@avr-size -C --mcu=${MCU} ${TARGET}

## Program
burn:
	avrdude -p $(BURNMCU) -c $(BURNPROGRAMMER)  -U flash:w:output/$(PROJECT).hex:a 

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) main.d usbdrv.d usbdrvasm.d  output/$(PROJECT).elf output/$(PROJECT).map output/$(PROJECT).lss output/$(PROJECT).hex output/$(PROJECT).eep
