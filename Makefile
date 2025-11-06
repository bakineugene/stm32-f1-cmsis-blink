include config.mk

SRCS = src/main.c  cmsis/Source/Templates/system_stm32f1xx.c
ASM_SRCS = cmsis/Source/Templates/gcc/startup_$(shell echo $(DEVICE_SUBFAMILY) | tr '[:upper:]' '[:lower:]').s

OBJS = $(SRCS:%.c=build/%.o) $(ASM_SRCS:%.s=build/%.o)
BUILD_DIRS = build/src build/cmsis/Source/Templates build/cmsis/Source/Templates/gcc

TARGET = firmware

$(BUILD_DIRS):
	@mkdir -p $@

all: $(BUILD_DIRS) $(TARGET).bin $(TARGET).hex

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

build/%.o: %.c | $(BUILD_DIRS)
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

build/%.o: %.s | $(BUILD_DIRS)
	$(CC) $(CFLAGS) -c -o $@ $<

-include $(DEPS)

clean:
	rm -rf build

flash: $(TARGET).bin
	openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg \
	-c "program $(TARGET).bin verify reset exit 0x08000000"

.PHONY: all clean flash
