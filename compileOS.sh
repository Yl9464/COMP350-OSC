#Project B - Lensi Gondaliya, Nina Aubourg, Ying Lu 
nasm bootload.asm
#image file of a hard disk that is filled with zeros
dd if=/dev/zero of=diskc.img bs=512 count=1000
# copy bootload to the beginning of diskc.img
dd if=bootload of=diskc.img bs=512 count=1 conv=notrunc
#compile kernel
bcc -ansi -c -o kernel_c.o kernel.c
#assemble kernel.asm
as86 -o kernel_asm.o kernel_asm.asm
#link together to produce kernel
ld86 -o kernel -d kernel_c.o kernel_asm.o
#Part B ~ dd if=kernel of=diskc.img bs=512 conv=notrunc seek=3 || copy to diskc.img at sector
./loadFile kernel
#put message.txt at sector 30
./loadFile message.txt #part C 
#run simulator
java -jar simulator.jar