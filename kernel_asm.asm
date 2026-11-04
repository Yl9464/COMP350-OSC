.global _putInMemory
.global _interrupt
.global _makeInterrupt21
.global _launchProgram
.extern _handleInterrupt21

;void putInMemory (int segment, int address, char character)
_putInMemory:
        push    bp
        mov     bp,sp
        push    ds
        push    si ;save call added
        mov     ax,[bp+4];segment
        mov     si,[bp+6];address
        mov     cl,[bp+8];character
        mov     ds,ax
        mov     [si],cl
        pop     ds
        pop     si
        pop     bp
        ret     

;int interrupt (int number, int AX, int BX, int CX, int DX)
_interrupt:
        push    bp
        mov     bp,sp
        push    si             ; Preserve caller's SI added

        mov     ax,[bp+4] ;get the interrupt number in AL
        push    ds ;use self-modifying code to call the right interrupt
        mov     bx,cs
        mov     ds,bx
        mov     si,#intr
        mov     [si+1],al ;change the 00 below to the contents of AL
        pop     ds

        mov     ax,[bp+6] ;get the other parameters AX, BX, CX, and DX
        mov     bx,[bp+8]
        mov     cx,[bp+10]
        mov     dx,[bp+12]
intr:
        int     #0x00 ;call the interrupt (00 will be changed above)
        mov     ah,#0 ;we only want AL returned
        pop     bp
        ret     
;void makeInterrupt21()
_makeInterrupt21:
        push    bp ;added
        mov     bp,sp;added
        push    ds ;added
        push    si ;save call added
        mov     dx,#_interrupt21ServiceRoutine
        ;push    ds
        mov     ax, #0
        mov     ds,ax
        mov     si,#0x84 ;vector 0x21 address (0x21 *4 = 0x84)
        mov     ax,cs
        cli     ;disable interrupts during vector update added

        mov     [si+2],ax ;set segment
        mov     [si],dx ;set offset 
        sti     ;re-enable interrupts added
        pop     si ;added
        pop     ds
        pop     bp ;added
        ret     

_interrupt21ServiceRoutine:
        push    ds ;preserver user added
        push    dx
        push    cx
        push    bx
        push    ax

        mov     ax,cs ;switch ds to kernel segment for C execution added
        mov     ds,ax
        call    _handleInterrupt21
        pop     ax
        pop     bx
        pop     cx
        pop     dx
        pop     ds ;restore user DS added
        iret    

;this is called to start a program that is loaded into memory
;void launchProgram(int segment)
_launchProgram:
        push    bp ;added
        mov     bp,sp
        mov     bx,[bp+4]; get semgnet paramets 

        ;mov     bx,[bp+2] ;get the segment into bx
        mov     ax,cs ;modify the jmp below to jump to our segment
        mov     ds,ax ;this is self-modifying code
        mov     si,#jump
        mov     [si+3],bx ;change the first 0000 to the segment

        mov     ds,bx ;set up the segment registers
        mov     es,bx
        mov     ss,bx
        mov     sp,#0xfff0 ;set up the stack pointer
        mov     bp,#0xfff0
jump:
        jmp     #0x0000:0x0000 ;and start running (the first 0000 is changed above)
