/* shell.c - Project C - Lensi Gondaliya, Nina Aubourg, Ying Lu */
main()
{
    char line[80];
    char buffer[13312];
    int sectorsRead;

    while (1) {
        syscall(0, "SHELL> ");
        syscall(1, line);

        /* Step 5: type filename - print a file */
        if (line[0] == 't' && line[1] == 'y' && line[2] == 'p' && line[3] == 'e' && line[4] == ' ') {
            syscall(3, line + 5, buffer, &sectorsRead);
            if (sectorsRead > 0) {
                syscall(0, buffer);
                syscall(0, "\r\n");
            } else {
                syscall(0, "File not found!\r\n");
            }
        }
        else {
            syscall(0, "Bad Command!\r\n");
        }
    }
}
