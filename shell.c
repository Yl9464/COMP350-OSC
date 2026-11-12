/* shell.c - Project C - Lensi Gondaliya, Nina Aubourg, Ying Lu */
main()
{
    char line[80];

    while (1) {
        syscall(0, "SHELL> ");
        syscall(1, line);
        syscall(0, "Bad Command!\r\n");
    }
}
