#Project C - Lensi Gondaliya, Nina Aubourg, Ying Lu
nasm bootload.asm
dd if=/dev/zero of=diskc.img bs=512 count=1000
dd if=bootload of=diskc.img bs=512 count=1 conv=notrunc
bcc -ansi -c -o kernel_c.o kernel.c
as86 kernel_asm.asm -o kernel_asm.o
ld86 -o kernel -d kernel_c.o kernel_asm.o
gcc -o loadFile loadFile.c
./loadFile kernel
./loadFile message.txt
