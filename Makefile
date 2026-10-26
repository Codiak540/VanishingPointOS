# Builds boot.firm with plain arm-none-eabi-gcc (no devkitARM needed) + firmtool.
#   pip install git+https://github.com/TuxSH/firmtool.git
CROSS ?= arm-none-eabi-
CC    := $(CROSS)gcc

A9_ARCH  := -marm -march=armv5te -mtune=arm946e-s
A9_FLAGS := -g -std=gnu11 -Wall -Wextra -O2 -mword-relocations -fomit-frame-pointer \
            -ffunction-sections -fdata-sections -Wno-main -Wno-array-bounds \
            -Wno-stringop-overflow -Wno-stringop-overread $(A9_ARCH) -DARM9 -D__3DS__ \
            -Iarm9/source
A9_SRC   := $(wildcard arm9/source/*.c arm9/source/fatfs/*.c arm9/source/fatfs/sdmmc/*.c) \
            $(wildcard arm9/source/*.s arm9/source/fatfs/sdmmc/*.s)
# object names must be bare (chainloader.o, i2c.o ...): Luma's linker script matches on them
A9_OBJ   := $(addprefix build/arm9/,$(addsuffix .o,$(basename $(notdir $(A9_SRC)))))

A11_ARCH  := -marm -march=armv6k -mtune=mpcore -mfloat-abi=soft
A11_FLAGS := -g -std=gnu11 -Wall -Wextra -O2 -mword-relocations -fomit-frame-pointer \
             -ffunction-sections -fdata-sections -Wno-main -fno-builtin $(A11_ARCH) \
             -DARM11 -D__3DS__
A11_SRC   := $(wildcard arm11/source/*.c arm11/source/*.s)
A11_OBJ   := $(addprefix build/arm11/,$(addsuffix .o,$(basename $(notdir $(A11_SRC)))))

LINK = -nostartfiles -specs=nosys.specs -Wl,--nmagic,--gc-sections,--no-warn-rwx-segments

all: boot.firm

boot.firm: arm11.elf arm9.elf
	firmtool build $@ -D arm11.elf arm9.elf -C XDMA NDMA

arm9.elf: $(A9_OBJ) arm9/linker.ld
	$(CC) $(A9_ARCH) -g -T arm9/linker.ld $(LINK) $(A9_OBJ) -o $@

arm11.elf: $(A11_OBJ) arm11/linker.ld
	$(CC) $(A11_ARCH) -g -T arm11/linker.ld $(LINK) $(A11_OBJ) -o $@

# One explicit rule per source file (arm9 and arm11 share some file names).
define A9_RULE
build/arm9/$(basename $(notdir $(1))).o: $(1)
	@mkdir -p $$(dir $$@)
	$$(CC) $$(A9_$(2)) -c $$< -o $$@
endef
define A11_RULE
build/arm11/$(basename $(notdir $(1))).o: $(1)
	@mkdir -p $$(dir $$@)
	$$(CC) $$(A11_$(2)) -c $$< -o $$@
endef
A9_SFLAGS  := $(A9_ARCH) -g -x assembler-with-cpp
A11_SFLAGS := $(A11_ARCH) -g -x assembler-with-cpp
$(foreach f,$(filter %.c,$(A9_SRC)),$(eval $(call A9_RULE,$(f),FLAGS)))
$(foreach f,$(filter %.s,$(A9_SRC)),$(eval $(call A9_RULE,$(f),SFLAGS)))
$(foreach f,$(filter %.c,$(A11_SRC)),$(eval $(call A11_RULE,$(f),FLAGS)))
$(foreach f,$(filter %.s,$(A11_SRC)),$(eval $(call A11_RULE,$(f),SFLAGS)))

clean:
	rm -rf build *.elf boot.firm
.PHONY: all clean
