CROSS_COMPILE     := /opt/gcc-arm-none-eabi-5_4-2016q3/bin/arm-none-eabi-
TARGET            ?= bsp

CC                := $(CROSS_COMPILE)gcc
LD                := $(CROSS_COMPILE)ld
OBJCOPY           := $(CROSS_COMPILE)objcopy
OBJDUMP           := $(CROSS_COMPILE)objdump

INCLDIRS          := imx6ul \
                    bsp/clk \
                    bsp/delay \
                    bsp/gpio \
					bsp/int \
					bsp/epit \
					bsp/uart \
					bsp/gpt
                    
SRCDIRS           := project \
                    bsp/clk \
                    bsp/delay \
                    bsp/gpio \
					bsp/int \
					bsp/epit \
					bsp/uart \
					bsp/gpt
                    
INCLUDE            := $(patsubst %, -I %, $(INCLDIRS))

SFILES             := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.S))
CFILES             := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

SFILENDIR          := $(notdir $(SFILES))
CFILENDIR          := $(notdir $(CFILES))

SOBJS              := $(patsubst %, obj/%, $(SFILENDIR:.S=.o))
COBJS              := $(patsubst %, obj/%, $(CFILENDIR:.c=.o))
OBJS               := $(SOBJS) $(COBJS)

VPATH              := $(SRCDIRS)

.PHONY: clean

$(TARGET).bin: $(OBJS)
	$(LD) -Timx6ul.lds -o $(TARGET).elf $^ -lgcc -L/opt/gcc-arm-none-eabi-5_4-2016q3/lib/gcc/arm-none-eabi/5.4.1
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis
    
$(SOBJS): obj/%.o : %.S
	$(CC) -Wall -nostdlib -c -O2 $(INCLUDE) -o $@ $<
    
$(COBJS): obj/%.o : %.c
	$(CC) -Wall -nostdlib -c -O2 $(INCLUDE) -o $@ $<

clean:
	rm -rf $(TARGET).elf $(TARGET).dis $(TARGET).bin $(SOBJS) $(COBJS)