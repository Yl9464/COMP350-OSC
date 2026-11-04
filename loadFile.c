// loadFile.c
// Loads a file from the host operating system into diskc.img
// Compile using GCC: gcc loadFile.c -o loadFile

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int i;

    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // 1. Open the source file in binary read mode
    FILE *loadFil = fopen(argv[1], "rb");
    if (loadFil == NULL)
    {
        printf("Error: File '%s' not found.\n", argv[1]);
        return 1;
    }

    // 2. Open the floppy disk image in binary read/write mode
    FILE *floppy = fopen("diskc.img", "r+b");
    if (floppy == NULL)
    {
        printf("Error: diskc.img not found.\n");
        fclose(loadFil);
        return 1;
    }

    // 3. Load the disk map from Sector 1 (offset 512)
    char map[512];
    fseek(floppy, 512, SEEK_SET);
    for (i = 0; i < 512; i++)
    {
        map[i] = fgetc(floppy);
    }

    // 4. Load the directory from Sector 2 (offset 1024)
    char dir[512];
    fseek(floppy, 512 * 2, SEEK_SET);
    for (i = 0; i < 512; i++)
    {
        dir[i] = fgetc(floppy);
    }

    // 5. Find a free 32-byte entry in the directory
    int dirIndex = -1;
    for (i = 0; i < 512; i += 32)
    {
        if (dir[i] == 0)
        {
            dirIndex = i;
            break;
        }
    }

    if (dirIndex == -1)
    {
        printf("Error: Directory is full (max 16 files).\n");
        fclose(loadFil);
        fclose(floppy);
        return 1;
    }

    // 6. Clear directory name field and write filename (up to 6 characters)
    for (i = 0; i < 6; i++)
    {
        dir[dirIndex + i] = 0x00;
    }

    for (i = 0; i < 6; i++)
    {
        if (argv[1][i] == '\0')
            break;
        dir[dirIndex + i] = argv[1][i];
    }

    int dirSectorPtr = dirIndex + 6; // Directory offset for storing sector numbers
    int sectCount = 0;

    // 7. Read host file and write sector-by-sector to floppy image
    while (!feof(loadFil))
    {
        if (sectCount == 26)
        {
            printf("Error: File exceeds max directory capacity (26 sectors / 13 KB).\n");
            fclose(loadFil);
            fclose(floppy);
            return 1;
        }

        // Buffer sector content and read up to 512 bytes
        char sectorBuffer[512];
        int bytesRead = 0;

        for (i = 0; i < 512; i++)
        {
            int c = fgetc(loadFil);
            if (c == EOF)
            {
                // Zero-pad remainder of the sector
                for (; i < 512; i++)
                {
                    sectorBuffer[i] = 0x00;
                }
                break;
            }
            sectorBuffer[i] = (char)c;
            bytesRead++;
        }

        // Stop if EOF reached right at sector boundary
        if (bytesRead == 0)
        {
            break;
        }

        // Find a free map entry (Sectors 0-2 are system reserved)
        int freeSector = -1;
        for (i = 3; i < 256; i++)
        {
            if (map[i] == 0)
            {
                freeSector = i;
                break;
            }
        }

        if (freeSector == -1)
        {
            printf("Error: Disk is full.\n");
            fclose(loadFil);
            fclose(floppy);
            return 1;
        }

        // Mark sector as used in map and record sector in directory entry
        map[freeSector] = 0xFF;
        dir[dirSectorPtr] = (char)freeSector;
        dirSectorPtr++;
        sectCount++;

        // Write entire 512-byte sector to disk image
        fseek(floppy, freeSector * 512, SEEK_SET);
        for (i = 0; i < 512; i++)
        {
            fputc(sectorBuffer[i], floppy);
        }
    }

    // 8. Write updated disk map back to Sector 1 (offset 512)
    fseek(floppy, 512, SEEK_SET);
    for (i = 0; i < 512; i++)
    {
        fputc(map[i], floppy);
    }

    // 9. Write updated directory back to Sector 2 (offset 1024)
    fseek(floppy, 512 * 2, SEEK_SET);
    for (i = 0; i < 512; i++)
    {
        fputc(dir[i], floppy);
    }

    // 10. Flush and close handles
    fclose(loadFil);
    fclose(floppy);

    printf("File '%s' successfully written to diskc.img (%d sectors used).\n", argv[1], sectCount);
    return 0;
}