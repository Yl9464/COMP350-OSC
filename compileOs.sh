#!/bin/bash

# 1. Assemble bootloader
nasm bootload.asm -o bootload

# 2. Create blank 512KB disk image filled with zeros
dd if=/dev/zero of=diskc.img bs=512 count=1000

# 3. Copy bootloader to Sector 0
dd if=bootload of=diskc.img bs=512 count=1 conv=notrunc

# 4. Compile C kernel
bcc -ansi -c -o kernel_c.o kernel.c

# 5. Assemble assembly helper
as86 kernel.asm -o kernel_asm.o

# 6. Link kernel objects
ld86 -o kernel -d kernel_c.o kernel_asm.o

# 7. Copy kernel to Sector 3
./loadFile kernel
dd if=message.txt of=diskc.img bs=512 count=1 seek=30 conv=notrunc
./loadFile message.txt

