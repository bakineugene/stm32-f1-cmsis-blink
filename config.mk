# Device configuration
DEVICE_FAMILY = STM32F1
DEVICE_SUBFAMILY = STM32F103x6
DEVICE_CORE = cortex-m3

# Toolchain
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size

# CMSIS paths
CMSIS_ROOT = cmsis
CMSIS_CORE = cmsis_core/CMSIS/Core
CMSIS_LINKER = $(CMSIS_ROOT)/Source/Templates/gcc/linker/STM32F103X6_FLASH.ld

# Compilation flags
CFLAGS = -mcpu=$(DEVICE_CORE) -mthumb -Wall -O0 -g
CFLAGS += -I$(CMSIS_CORE)/Include -I$(CMSIS_ROOT)/Include -Iinclude
CFLAGS += -D$(DEVICE_SUBFAMILY)  -ffreestanding
LDFLAGS = -T $(CMSIS_LINKER) -nostartfiles -nostdlib -Wl,-Map=output.map


