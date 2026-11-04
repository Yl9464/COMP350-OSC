.globl _syscall

;void syscall (int AX, int BX, int CX, int DX)
_syscall:
    push    bp
    mov     bp,sp
    mov     ax,[bp+4]   ; get AX parameter
    mov     bx,[bp+6]   ; get BX parameter
    mov     cx,[bp+8]   ; get CX parameter
    mov     dx,[bp+10]  ; get DX parameter
    int     #0x21
    pop     bp
    ret     
