/* Function Declarations */
void printString(char *);
void printChar(char);
void readString(char *);
void readSector(char *, int);
void handleInterrupt21(int, int, int, int);
int matchFileName(char *, char *);
void readFile(char *, char *, int *);
void executeProgram(char *); /* step 2*/

/* External Assembly Functions */
extern void makeInterrupt21();
extern int interrupt(int, int, int, int, int);
extern void putInMemory(int, int, char); /* step 2 */
extern void launchProgram(int);			 /* step 2 */
int main()
{
	// step2
	makeInterrupt21();
	interrupt(0x21, 4, "tstpr1", 0, 0);
	while (1)
		;
	// Step 1
	// char buffer[13312]; /*this is the maximum size of a file*/
	// int sectorsRead;
	// makeInterrupt21();
	// interrupt(0x21, 3, "messag", buffer, &sectorsRead); /*read the file into buffer*/
	// if (sectorsRead > 0)
	// 	interrupt(0x21, 0, buffer, 0, 0); /*print out the file*/
	// else
	// 	interrupt(0x21, 0, "messag not found\r\n", 0, 0); /*no sectors read? then print an error*/
	// while (1)
	// 	; /*hang up*/
}

/* Load and Execute Program */
void executeProgram(char *name)
{
	char buffer[13312];
	int sectorsRead;
	int i;
	int segment = 0x2000;

	/* 1. Call readFile to load the file into a buffer */
	readFile(name, buffer, &sectorsRead);

	/* Check if the file was found and read successfully */
	if (sectorsRead > 0)
	{
		/* 2. Transfer the file from buffer into memory at segment 0x2000 */
		for (i = 0; i < 13312; i++)
		{
			putInMemory(segment, i, buffer[i]);
		}

		/* 3. Call launchProgram (does not return) */
		launchProgram(segment);
	}
	else
	{
		printString("Error: Program not found.\r\n");
	}
}

/*Read Sector from Disk using BIOS Interrupt 0x13 */
void readSector(char *buffer, int sector)
{
	int relativeSector = sector + 1;
	int head = 0;
	int track = 0;

	int ax = 2 * 256 + 1;				   /* AH = 2 (read), AL = 1 sector */
	int cx = track * 256 + relativeSector; /* CH = track (0), CL = sector + 1 */
	int dx = head * 256 + 0x80;			   /* DH = head (0), DL = hard drive (0x80) */

	interrupt(0x13, ax, buffer, cx, dx);
}
/* Helper function to compare filenames up to 6 characters */
int matchFileName(char *fileName, char *dirEntry)
{
	int i;
	for (i = 0; i < 6; i++)
	{
		char c = fileName[i]; // extracts char at i from fileName
		if (c == '\r' || c == '\n' || c == '\0')
		{			  // check if c is a return, newline, or null terminator character.
			c = 0x00; // if so replace is null byte
		}
		if (c != dirEntry[i]) // compare normalized fileName char against corresponding byte in dirEntry[i]
		{
			return 0; // if no match, returns 0 to signal mismatch and exit early
		}
	}
	return 1; // returns 1, signalling successful match
}

/* Reads a file by name into a memory buffer */
void readFile(char *fileName, char *buffer, int *sectorsRead)
{
	unsigned char dir[512]; /* Use unsigned char to avoid negative sector numbers */
	int fileentry;
	int sectorIndex;
	int found = 0;

	*sectorsRead = 0;

	/* 1. Read Sector 2 (Directory) into local buffer */
	readSector((char *)dir, 2);

	/* 2. Step through all 16 entries (each 32 bytes wide) */
	for (fileentry = 0; fileentry < 512; fileentry += 32)
	{
		/* Skip unused directory entries */
		if (dir[fileentry] == 0x00)
		{
			continue;
		}

		/* Check if the filename matches */
		if (matchFileName(fileName, (char *)&dir[fileentry]))
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
		int currentSector = dir[fileentry + sectorIndex];

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

void handleInterrupt21(int ax, int bx, int cx, int dx) // called when software interrupt 0x21 is triggered
{
	if (ax == 0) // 0 (printing a str)
	{
		printString((char *)bx);
		// cast bx from int to char*
		// call printString() to display it
	}
	else if (ax == 1) // 1 (reading keyboard Input)
	{
		readString((char *)bx);
		// cast bx to char* to specify memory buffer location
		// call readString() to populate buffer with user input
	}
	else if (ax == 2) // read raw sector reading
	{
		readSector((char *)bx, cx);
		// cast bx to char* as destination buffer address in memory
		// pass cs as target disk sector
	}
	else if (ax == 3) // check if ax =3 if so…
	{				  // read a file

		readFile((char *)bx, (char *)cx, (int *)dx);
		// bx = file to read
		// cx = dest memory address of file contents
		// dx = point to int where readFile() write total number of sectors read
	}

	else if (ax == 4)
	{ // check if ax =4 if so…
		// execute program (load and execute binary from disk
		executeProgram((char *)bx);
		// bx = program name to execute
	}
	else // error block if ax params service code doesn’t equal 0, 2, ,3 or 4
	{
		printString("Error: Invalid system call interrupt AX value.\r\n");
		// output error message over console with problem AX value
	}
}

/*Read String from Keyboard using BIOS Interrupt 0x16 */
void readString(char *line)
{
	int i = 0;
	char key = 0;

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
				printChar(' ');	 /* Overwrite character */
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

/*Print String using BIOS Interrupt 0x10 */
void printString(char *chars)

{
	int i = 0;
	while (chars[i] != 0x0)
	{
		int ax = 0x0E * 256 + chars[i];
		interrupt(0x10, ax, 0, 0, 0);
		i++;
	}
}
/* Helper function for printing a single character */

void printChar(char c)
{
	int ax = 0x0E * 256 + c;
	interrupt(0x10, ax, 0, 0, 0);
}