#Project B - Lensi Gondaliya, Nina Aubourg, Ying Lu 
nasm bootload.asm
dd if=/dev/zero of=diskc.img bs=512 count=1000
dd if=bootload of=diskc.img bs=512 count=1 conv=notrunc
as86 kernel_asm.asm -o kernel_asm.o
bcc -ansi -c -o kernel_c.o kernel.c
ld86 -o kernel -d kernel_c.o kernel_asm.o
#dd if=kernel of=diskc.img bs=512 conv=notrunc seek=3
dd if=message.txt of=diskc.img bs=512 count=1 seek=30 conv=notrunc

bcc -ansi -c -o tstpr1.o tstpr1.c
as86 userlib.asm -o userlib.o
ld86 -d -o tstpr1 tstpr1.o userlib.o
./loadFile tstpr1