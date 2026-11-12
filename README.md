# COMP 350 - Project C: Loading Files and Executing Programs
Team: Lensi Gondaliya, Nina Aubourg, Ying Lu

## How to build and run
1. On the class server run `bash compileOS.sh`. It builds the bootloader, kernel, loadFile,
   tstpr1, tstpr2 and shell, and uses loadFile to put kernel, message.txt, tstpr1, tstpr2
   and shell onto diskc.img.
2. Open simulator.jar, tick Hard Drive C, choose diskc.img, set Cylinders 28, Heads 2,
   Sectors 18, and click Boot Disk C.

## Step 1 - readFile (interrupt 0x21, AX=3)
**What we did:** readFile(name, buffer, &sectorsRead) reads the directory (sector 2),
looks for a 32-byte entry whose 6-character name matches, then reads each listed sector
into the buffer (512 bytes apart) until it reaches sector number 0. If no entry matches,
sectorsRead is set to 0. readSector reads from hard disk C (DL = 0x80).
**How to verify:** Type `type messag` at the shell. The contents of message.txt print.

## Step 2 - executeProgram (interrupt 0x21, AX=4)
**What we did:** executeProgram(name) uses readFile to load the program, copies it into
memory at segment 0x2000 with putInMemory, then calls launchProgram(0x2000).
**How to verify:** Type `exec tstpr2` at the shell. "tstpr2 is working!" prints.
(We first tested it by putting `interrupt(0x21, 4, "tstpr1", 0, 0);` in main(), which
printed "tstpr1 is working!".)

## Step 3 - terminate (interrupt 0x21, AX=5)
**What we did:** terminate() first hung the computer with while(1). After Step 4 it builds
the name "shell" one character at a time in a char array and calls executeProgram on it,
so the shell reloads when a program ends.
**How to verify:** `exec tstpr2` prints "tstpr2 is working!" and then the SHELL> prompt returns.

## Step 4 - Shell
**What we did:** shell.c is a user program compiled with userlib.asm and loaded onto the
disk as "shell". It loops: prints `SHELL> `, reads a line with syscall(1, ...), and prints
"Bad Command!" for anything it doesn't recognize. All input/output uses syscall().
The kernel's main() now just sets up interrupt 0x21 and runs the shell.
**How to verify:** Boot the disk. `SHELL> ` appears. Type `hello`; it prints "Bad Command!".

## Step 5 - type command
**What we did:** `type filename` loads the file with syscall(3, ...) and prints it with
syscall(0, ...). If no sectors are read, it prints "File not found!".
**How to verify:** `type messag` prints the message; `type abc` prints "File not found!".

## Step 6 - exec command
**What we did:** `exec filename` first checks that the file exists with syscall(3, ...),
then runs it with syscall(4, ...), which loads it over the shell. If the file doesn't exist,
it prints "Program not found!". The check happens in the shell because strings stored in
the kernel can't be printed while a user program's data segment is active.
**How to verify:** `exec tstpr2` prints "tstpr2 is working!" and returns to SHELL>;
`exec abc` prints "Program not found!".

## Fixes we made along the way
- compileOS.sh uses loadFile for the kernel and message.txt instead of dd, as the handout says.
- kernel_asm.asm, userlib.asm, tstpr1.c and tstpr2.c were missing a final newline,
  which made as86 fail; userlib.asm also had a stray first line.
