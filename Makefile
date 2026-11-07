NAME := OffOS-0.1TEST
BUILD := build
ISO_ROOT := $(BUILD)/iso
ISO_OUT ?= ../generated-artifacts/$(NAME).iso

CC ?= gcc
AS := nasm
LD ?= ld
GRUB_MKRESCUE ?= grub-mkrescue

CFLAGS := -m32 -std=c11 -O2 -Wall -Wextra -Werror \
	-ffreestanding -fno-pie -fno-stack-protector -fno-builtin \
	-fno-asynchronous-unwind-tables -fno-unwind-tables \
	-march=i386 -mno-sse -mno-sse2 -mno-mmx -msoft-float

.PHONY: all iso run clean check

all: iso

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: boot.asm | $(BUILD)
	$(AS) -f elf32 $< -o $@

$(BUILD)/interrupts.o: interrupts.asm | $(BUILD)
	$(AS) -f elf32 $< -o $@

$(BUILD)/kernel.o: src/kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/kernel.elf: $(BUILD)/boot.o $(BUILD)/interrupts.o $(BUILD)/kernel.o linker.ld
	$(LD) -m elf_i386 -T linker.ld -nostdlib \
		$(BUILD)/boot.o $(BUILD)/interrupts.o $(BUILD)/kernel.o -o $@
	grub-file --is-x86-multiboot $@

$(ISO_ROOT)/boot/grub/grub.cfg: grub/grub.cfg
	mkdir -p $(ISO_ROOT)/boot/grub
	cp $< $@

$(ISO_ROOT)/boot/kernel.elf: $(BUILD)/kernel.elf
	mkdir -p $(ISO_ROOT)/boot
	cp $< $@

iso: $(ISO_ROOT)/boot/grub/grub.cfg $(ISO_ROOT)/boot/kernel.elf
	mkdir -p $(dir $(ISO_OUT))
	$(GRUB_MKRESCUE) -o $(ISO_OUT) $(ISO_ROOT)
	@echo "ISO criada: $(ISO_OUT)"

check: $(BUILD)/kernel.elf
	grub-file --is-x86-multiboot $<
	@echo "Kernel Multiboot válido."

run: iso
	qemu-system-i386 -m 128M -cdrom $(ISO_OUT) -boot d

clean:
	rm -rf $(BUILD)
	rm -f $(ISO_OUT)