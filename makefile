
CFLAGS  ?=  -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
            -Wformat-truncation -fno-common -Wconversion \
            -g3 -Os -ffunction-sections -fdata-sections -I. \
            -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 $(EXTRA_CFLAGS)
LDFLAGS ?= -Tlink.ld -nostartfiles -nostdlib --specs nano.specs -lc -lgcc -Wl,--gc-sections -Wl,-Map=$@.map
SOURCES = -Iheaders src/*.c

ifeq ($(OS),Windows_NT)
  RM = cmd /C del /Q /F
else
  RM = rm -f
endif

build: firmware.bin
	rm firmware.elf.map

firmware.elf: 
	arm-none-eabi-gcc $(SOURCES) $(CFLAGS) $(LDFLAGS) -o bin/$@

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary bin/$< bin/$@

flash: firmware.bin
	st-flash --reset write bin/$< 0x8000000
	rm firmware.elf.map

clean:
	$(RM) bin/firmware.*
