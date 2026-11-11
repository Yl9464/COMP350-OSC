/* kernel.c */
/* Nina A */

/* Function Declarations */
void printString();
void printChar();
void readString();
void readSector();
void handleInterrupt21();
int matchFileName();
void readFile();

/* External Assembly Functions */
void makeInterrupt21();
int interrupt();

int main()
{
    char buffer[13312];
    int sectorsRead;
    makeInterrupt21();

    interrupt(0x21, 3, "messag", buffer, &sectorsRead);

    if (sectorsRead > 0)
    {
        interrupt(0x21, 0, buffer, 0, 0);
    }
    else
    {
        interrupt(0x21, 0, "message not found\r\n", 0, 0);
    }

    while (1)
        ;
    return 0;
}

/* Step 1: Print String using BIOS Interrupt 0x10 */
void printString(chars) char *chars;
{
    int i;
    int ax;

    i = 0;
    while (chars[i] != 0x0)
    {
        ax = 0x0E * 256 + chars[i];
        interrupt(0x10, ax, 0, 0, 0);
        i++;
    }
}

/* Helper function for printing a single character */
void printChar(c) char c;
{
    int ax;

    ax = 0x0E * 256 + c;
    interrupt(0x10, ax, 0, 0, 0);
}

/* Step 2: Read String from Keyboard using BIOS Interrupt 0x16 */
void readString(line) char *line;
{
    int i;
    char key;

    i = 0;
    key = 0;

    while (1)
    {
        key = (char)interrupt(0x16, 0, 0, 0, 0);

        /* Handle Enter Key */
        if (key == 0x0D)
        {
            line[i] = 0x0D;
            line[i + 1] = 0x0A; /* Line Feed */
            line[i + 2] = 0x00; /* End of String */
            printChar(0x0D);
            printChar(0x0A);
            return;
        }
        /* Handle Backspace Key */
        else if (key == 0x08)
        {
            if (i > 0)
            {
                i--;
                printChar(0x08); /* Move cursor back */
                printChar(' ');  /* Overwrite character */
                printChar(0x08); /* Move cursor back */
            }
        }
        /* Printable Characters */
        else
        {
            line[i] = key;
            printChar(key); /* Echo character to screen */
            i++;
        }
    }
}

/* Step 3: Read Sector from Disk using BIOS Interrupt 0x13 */
void readSector(buffer, sector) char *buffer;
int sector;
{
    int relativeSector;
    int tempHead;
    int head;
    int track;
    int ax;
    int cx;
    int dx;

    relativeSector = (sector - (sector / 18) * 18) + 1;
    tempHead = sector / 18;
    head = tempHead - (tempHead / 2) * 2;
    track = sector / 36;

    ax = 2 * 256 + 1;                  /* AH = 2 (read), AL = 1 sector */
    cx = track * 256 + relativeSector; /* CH = track (0), CL = sector + 1 */
    dx = head * 256 + 0x80;            /* DH = head (0), DL = hard drive (0x80) */

    interrupt(0x13, ax, buffer, cx, dx);
}

/* Helper function to compare filenames up to 6 characters */
int matchFileName(fileName, dirEntry)
char *fileName;
char *dirEntry;
{
    int i;
    int endReached;
    char c;

    endReached = 0;

    for (i = 0; i < 6; i++)
    {
        /* If input string ends before 6 chars, pad comparison with null bytes */
        if (endReached)
        {
            c = 0x00;
        }
        else if (fileName[i] == '\0' || fileName[i] == '\r' || fileName[i] == '\n')
        {
            endReached = 1;
            c = 0x00;
        }
        else
        {
            c = fileName[i];
        }
        if (c != dirEntry[i])
        {
            return 0;
        }
    }
    return 1; /* Match found */
}

/* Reads a file by name into a memory buffer */
void readFile(fileName, buffer, sectorsRead) char *fileName;
char *buffer;
int *sectorsRead;
{
    char dir[512];
    int fileentry;
    int sectorIndex;
    int found;
    unsigned char currentSector;

    found = 0;
    *sectorsRead = 0;

    /* 1. Read Sector 2 (Directory) into local buffer */
    readSector(dir, 2);

    /* 2. Step through all 16 entries (each 32 bytes wide) */
    for (fileentry = 0; fileentry < 512; fileentry += 32)
    {
        /* Skip unused directory entries */
        if (dir[fileentry] == 0x00)
        {
            continue;
        }

        /* Check if the filename matches */
        if (matchFileName(fileName, &dir[fileentry]))
        {
            found = 1;
            break;
        }
    }

    /* If not found, exit with sectorsRead = 0 */
    if (!found)
    {
        return;
    }

    /* 3. Load sectors sequentially into buffer */
    for (sectorIndex = 6; sectorIndex < 32; sectorIndex++)
    {
        currentSector = (unsigned char)dir[fileentry + sectorIndex];

        /* Sector 0 indicates end of file */
        if (currentSector == 0)
        {
            break;
        }

        /* Read sector into buffer offset by 512 bytes each iteration */
        readSector(buffer + (*sectorsRead * 512), currentSector);
        (*sectorsRead)++;
    }
}

/* Step 4 & 5: Service Handler for Interrupt 0x21 */
void handleInterrupt21(ax, bx, cx, dx) int ax, bx, cx, dx;
{
    if (ax == 0)
    {
        printString((char *)bx);
    }
    else if (ax == 1)
    {
        readString((char *)bx);
    }
    else if (ax == 2)
    {
        readSector((char *)bx, cx);
    }
    else if (ax == 3)
    {
        /* AX = 3: Read File, BX = fileName, CX = buffer, DX = sectorRead */
        readFile((char *)bx, (char *)cx, (int *)dx);
    }
    else if (ax == 4)
    {
        /* AX = 4: Load and Execute Program, BX = address of program name */
        executeProgram((char *)bx);
    }
    else
    {
        printString("Error: Invalid Interrupt call\r\n");
    }
}