include config.mk

SRCS = src/main.c src/system_stm32f1xx.c
ASM_SRCS = src/startup/startup_$(shell echo $(DEVICE_SUBFAMILY) | tr '[:upper:]' '[:lower:]').s

OBJS = $(SRCS:.c=.o) $(ASM_SRCS:.s=.o)

TARGET = firmware

all: $(TARGET).bin $(TARGET).hex

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).hex output.map

flash: $(TARGET).bin
	openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg \
	-c "program $(TARGET).bin verify reset exit 0x08000000"

.PHONY: all clean flash
