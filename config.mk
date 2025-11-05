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
CMSIS_DEVICE = $(CMSIS_ROOT)/STM32F1xx
CMSIS_CORE = cmsis_core/CMSIS/Core

# Compilation flags
CFLAGS = -mcpu=$(DEVICE_CORE) -mthumb -Wall -O0 -g
CFLAGS += -I$(CMSIS_CORE)/Include -I$(CMSIS_DEVICE)/Include -Iinclude
CFLAGS += -D$(DEVICE_SUBFAMILY)  -ffreestanding
LDFLAGS = -T ld/linker.ld -nostartfiles -nostdlib -Wl,-Map=output.map


