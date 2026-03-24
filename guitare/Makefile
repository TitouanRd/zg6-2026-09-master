##############################################################################################
# Usage:
# make [dbg]		// -O0, -g (not optimzed, debug symbols)
# make opt			// -O2 -g (optimzed, debug symbols)
# make release		// -O2 optimized, no more debug symbols
##############################################################################################
# Start of user section
#
PROJ = main

# Define linker script file here
LDSCRIPT = config/stm32f411re_flash.lds

# Use newlib-nano libc (1) / standard newlib libc (0)
#   and add floating point support to printf/scanf
USE_NEWLIB_NANO = 0
USE_NEWLIB_NANO_FLOAT_IO = 1

# Use libc semihosting
USE_SEMIHOST=0

# Use Segger Real Time Transfer (RTT)
USE_RTT=0

# List all user C define here
UDEFS = -DSTM32F411xE

# Define ASM defines here
UADEFS = 

# List C source files here
SRC  = startup/stm32f411_periph.c startup/sys_handlers.c \
       startup/rcc.c startup/system_stm32f4xx.c \
       src/main.c src/hardware.c

# List ASM source files here
ASRC = startup/startup_stm32f411xe.s


# List all user directories here
UINCDIR = . libext

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = lib/libstm32.a -lm

# include external libraries and board drivers
include lib/lib.mk

#
# End of user defines
#############################################################################


#############################################################################
# Start of default section
#
ifeq ($(OS),Windows_NT)
    SHELL=C:/Windows/System32/cmd.exe
else
    SHELL=sh
endif

TARGET  = arm-none-eabi-
CC      = $(TARGET)gcc
OBJCOPY = $(TARGET)objcopy
AS      = $(TARGET)gcc -x assembler-with-cpp -c
SIZE    = $(TARGET)size
OBJDUMP = $(TARGET)objdump

ARCH      = armv7e-m
FLOAT-ABI = hard
CPU       = cortex-m4
CPUFLAGS  = -mthumb
FPU       = fpv4-sp-d16

# List all default C defines here, like -D_DEBUG=1
DDEFS = -march=$(ARCH) -mfloat-abi=$(FLOAT-ABI) -mcpu=$(CPU) -mfpu=$(FPU) $(CPUFLAGS)

# List all default ASM defines here, like -D_DEBUG=1
DADEFS = -D__ASSEMBLY__

# List all default directories to look for include files here
DINCDIR = 

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = lib/libstm32.a

MODE ?= dbg

ifeq (${MODE},release)
	OPT= -O2 -fomit-frame-pointer
	USE_SEMIHOST = 0
	USE_RTT = 0
else ifeq (${MODE},dbg)
	OPT= -g -O0 -fno-omit-frame-pointer -D__DEBUG__
	UADEFS += -D__DEBUG__
	ifeq (${USE_SEMIHOST},0)
		SRC += startup/SEGGER_RTT.c
		ASRC += startup/SEGGER_RTT_ASM_ARMv7M.s
	endif
else ifeq (${MODE},opt)
	OPT= -g -O2 -fno-omit-frame-pointer -D__DEBUG__
	UADEFS += -D__DEBUG__
	ifeq (${USE_SEMIHOST},0)
		SRC += startup/SEGGER_RTT.c
		ASRC += startup/SEGGER_RTT_ASM_ARMv7M.s
	endif
endif


ifeq (${USE_SEMIHOST},0)
	SRC += startup/libc_syscalls.c
else
	UADEFS += -DUSE_SEMIHOSTING
endif

ifeq (${USE_RTT},1)
    UADEFS += -DUSE_RTT
    UDEFS += -DUSE_RTT
endif

#
# End of default section
##############################################################################################

INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))
DEFS    = $(DDEFS) $(UDEFS)
ADEFS   = $(DADEFS) $(UADEFS)
OBJS    = $(SRC:.c=.o) $(ASRC:.s=.o)
LIBS    = $(DLIBS) $(ULIBS)

ASFLAGS = $(INCDIR) $(DEFS) -Wa,--gdwarf2 $(ADEFS)
CFLAGS = -std=c99 $(INCDIR) $(DEBUG) $(OPT) -Wall $(DEFS) -Wextra
ifeq (${MODE},dbg)
  CFLAGS +=  -Wwrite-strings -Wold-style-definition -Warray-bounds -Wconversion -Wno-sign-conversion -Wno-unused -Wno-unused-parameter -Werror -Wfatal-errors
endif
LDFLAGS = $(DEFS) -T$(LDSCRIPT) -Wl,-Map=$@.map,--gc-sections $(LIBDIR)
ifeq (${USE_NEWLIB_NANO},1)
  LDFLAGS += --specs=nano.specs
  ifeq (${USE_NEWLIB_NANO_FLOAT_IO},1)
    LDFLAGS += -u _scanf_float -u _printf_float
  endif
endif
ifeq (${USE_SEMIHOST},1)
  LDFLAGS += --specs=rdimon.specs
endif

#-fomit-frame-pointer -Wall -mlong-calls -ffunction-sections $(DEFS)
#LDFLAGS = $(MCFLAGS) -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(PROJ).map,--gc-sections $(LIBDIR)

# Generate dependency information
CFLAGS += -MD -MP -MF .dep/$(@F).d
ASFLAGS += -MD -MP -MF .dep/$(@F).d

#
# makefile rules
#
all: main.elf
	@echo "MODE: ${MODE}"

debug:
	@make MODE=dbg

release:
	@make MODE=release

opt:
	@make MODE=opt

%o: %c
	$(CC) -c $(CFLAGS) $< -o $@

%o: %s
	$(AS) $(ASFLAGS) $< -o $@

main.elf: $(OBJS) $(LDSCRIPT)
	$(CC) -o $@ $(filter-out %.lds, $^) $(LDFLAGS) $(LIBS)
	$(OBJDUMP) -h $@
	$(SIZE) $@
	
#	$(CC) -o $@ $(filter-out %.lds, $^) $(LDFLAGS) $(LIBS) -lc -lgcc -lgcov -lm

%hex: %elf
	$(OBJCOPY) -O ihex $< $@

%bin: %elf
	$(OBJCOPY) -O binary $< $@
	
clean:
	-rm -f $(OBJS)
	-rm -f *.elf
	-rm -f *.map
	-rm -f *.bin
	-rm -f *.hex
	-rm -fR .dep/*

# 
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

.PHONY: clean sim all debug release opt

