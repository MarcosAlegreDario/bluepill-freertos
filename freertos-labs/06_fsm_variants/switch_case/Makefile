# Proyecto bare-metal STM32F103C8T6 con libopencm3
PROJECT = main
TARGET = $(PROJECT).elf
BUILD_DIR = bin

# Rutas base
COMMON          = ../../../common
LIBOPENCM3_DIR  = ../../../libopencm3
FREERTOS_DIR    = ../../../freertos

# Includes
INCLUDES  = -I$(LIBOPENCM3_DIR)/include
INCLUDES += -I$(COMMON)
INCLUDES += -I$(FREERTOS_DIR)
INCLUDES += -I$(FREERTOS_DIR)/include
INCLUDES += -I$(FREERTOS_DIR)/portable/GCC/ARM_CM3
INCLUDES += -I../include
INCLUDES += -I..

# Micro objetivo
MCU     = cortex-m3
CDEFS   = -DSTM32F1

# Compilador
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
CFLAGS  = -mcpu=$(MCU) -mthumb -Wall -O0 -g $(INCLUDES) $(CDEFS) -nostdlib

# Linker
LDSCRIPT = $(COMMON)/linker.ld
LDFLAGS  = -T$(LDSCRIPT) -nostartfiles -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map
LDLIBS   = -L$(LIBOPENCM3_DIR)/lib -lopencm3_stm32f1 -lc -lgcc -lnosys

# Fuentes
FREERTOS_SRC = \
    $(FREERTOS_DIR)/list.c \
    $(FREERTOS_DIR)/queue.c \
    $(FREERTOS_DIR)/tasks.c \
    $(FREERTOS_DIR)/heap_4.c \
    $(FREERTOS_DIR)/portable/GCC/ARM_CM3/port.c

SRC = main.c fsm_switch_case.c $(FREERTOS_SRC)

OBJ = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(SRC)))

vpath %.c $(dir $(SRC))

.PHONY: all clean flash openocd gdb size help

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJ) $(LIBOPENCM3_DIR)/lib/libopencm3_stm32f1.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	$(OBJCOPY) -O ihex   $@ $(BUILD_DIR)/$(PROJECT).hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/$(PROJECT).bin

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBOPENCM3_DIR)/lib/libopencm3_stm32f1.a:
	$(MAKE) -C $(LIBOPENCM3_DIR)

flash: $(BUILD_DIR)/$(TARGET)
	openocd -f interface/stlink.cfg -c "set CPUTAPID 0" -f target/stm32f1x.cfg -c "program $< verify reset exit"

openocd:
	openocd -f interface/stlink.cfg -c "set CPUTAPID 0" -f target/stm32f1x.cfg
	
gdb: $(BUILD_DIR)/$(TARGET)
	gdb-multiarch $< -ex "target remote localhost:3333"

size: $(BUILD_DIR)/$(TARGET)
	arm-none-eabi-size $<

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Comandos disponibles:"
	@echo "  make           → Compila el proyecto"
	@echo "  make flash     → Flashea el binario vía OpenOCD"
	@echo "  make gdb       → Conecta GDB al target"
	@echo "  make openocd   → Lanza el servidor OpenOCD"
	@echo "  make size      → Muestra el uso de memoria"
	@echo "  make clean     → Limpia la carpeta bin/"
	@echo "  make help      → Muestra esta ayuda"
	@echo "  make all       → Compila el proyecto (alias de make)"
