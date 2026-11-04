# Build_An_OS_B
Documenting our process for Building OS B as a group

## Step 1 – printString (Lensi Gondaliya)

**What we did:** Wrote `printString(char*)`, which prints each character of a string using BIOS interrupt 0x10 (AH = 0xE, AL = character) until it reaches the 0x0 end-of-string. Also added a `printChar(char)` helper for debugging. Replaced the Project A `kernel_asm.asm` with the Project B version, which includes the `interrupt()` function.

**How to verify:** Call `printString("Hello World");` in `main()`, compile and run. "Hello World" appears on the screen.

## Step 4 – Interrupt 0x21 (Lensi Gondaliya)

**What we did:** Uncommented `.extern _handleInterrupt21` and the `_interrupt21ServiceRoutine` lines in `kernel_asm.asm`. Wrote `handleInterrupt21(int ax, int bx, int cx, int dx)` in `kernel.c`. In `main()`, `makeInterrupt21()` puts our handler's address in the interrupt vector table (0x84), then `interrupt(0x21, 0, 0, 0, 0)` triggers it.

**How to verify:** Compile and run. "Hello from interrupt 21!" prints on the screen at boot.

## How to compile and run

1. On the class server, run `bash compileOS.sh` to build `diskc.img`
2. Run `diskc.img` in the simulator
