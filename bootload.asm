    bits  16

KSEG    equ 0x1000  ;kernel goes into memory at 0x10000
KSIZE   equ 10  ;kernel is at most 10 sectors
KSTART  equ 3   ;kernel lives at sector 3

    org   0h

    ;set up the segment registers
    mov   ax,KSEG
    mov   ds,ax
    mov   ss,ax
    mov   es,ax

    ;stack starts at KSEG:fff0
    mov   ax,0xfff0
    mov   sp,ax
    mov   bp,ax

    ;read in the kernel from the disk
    mov   cl,KSTART+1     ;cl holds sector number
    mov   dh,0        ;head number
    mov   ch,0        ;track number
    mov   ah,2        ;absolute disk read
    mov   al,KSIZE        ;read KSIZE sectors
    mov   dl,80h      ;read from hard disk C
    mov   bx,0        ;read into 0 (in the segment)
    int   13h         ;BIOS disk read

    ;call the kernel
    jmp   KSEG:0

    times 510-($-$$) db 0

    ;AA55 tells BIOS that this is a valid bootloader
    dw    0xAA55