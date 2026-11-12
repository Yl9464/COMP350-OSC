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
as86 userlib.asm -o userlib.o
bcc -ansi -c -o tstpr1.o tstpr1.c
ld86 -d -o tstpr1 tstpr1.o userlib.o
./loadFile tstpr1
bcc -ansi -c -o tstpr2.o tstpr2.c
ld86 -d -o tstpr2 tstpr2.o userlib.o
./loadFile tstpr2
bcc -ansi -c -o shell.o shell.c
ld86 -d -o shell shell.o userlib.o
./loadFile shell
