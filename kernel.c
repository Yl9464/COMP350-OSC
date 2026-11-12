/* kernel.c
/*Project B -Lensi Gondaliya, Nina Aubourg, Ying Lu */
// void readString(char *chars);
// void printString(char *);
// void printChar(char);
// void handleInterrupt21(int, int, int, int);
// int matchFileName(char *, char *);
// void readFile(char *, char *, int *);

// /* External Assembly Functions */
// extern void makeInterrupt21();
// extern int interrupt(int, int, int, int, int);

void readString(char *chars);
void printString(char *chars);
void printChar(char c);
void readSector(char *buffer, int sector);
void readFile(char *fileName, char *buffer, int *sectorsRead);
int matchFileName(char *name1, char *name2);
void handleInterrupt21(int ax, int bx, int cx, int dx);

main()
{
	char buffer[13312]; /*this is the maximum size of a file*/
	int sectorsRead;
	makeInterrupt21();
	interrupt(0x21, 3, "messag", buffer, &sectorsRead); /*read the file into buffer*/
	if (sectorsRead > 0)
		interrupt(0x21, 0, buffer, 0, 0); /*print out the file*/
	else
		interrupt(0x21, 0, "messag not found\r\n", 0, 0); /*no sectors read? then print an error*/
	while (1)
		; /*hang up*/

	// Step 5
	// makeInterrupt21();
	// interrupt(0x21, 1, line, 0, 0);
	// interrupt(0x21, 0, line, 0, 0);
	// step 4

	// makeInterrupt21(); // call
	// interrupt(0x21,0,0,0,0);
	// Part3
	// readSector(buffer, 30);
	// printString(buffer);

	// Part 2
	//	printString("Enter a line: ");
	//	readString(line);
	//	printString(line);
	//  printString("Hello World");// Part 1
}
void printNum(int n)
{
	char buf[10];
	int i = 0, j;

	if (n == 0)
	{
		printChar('0');
		return;
	}
	while (n > 0)
	{
		buf[i++] = (n % 10) + '0';
		n /= 10;
	}
	for (j = i - 1; j >= 0; j--)
	{
		printChar(buf[j]);
	}
}
void readSector(char *buffer, int sector)
{
	int AH = 2;
	int AL = 1;		  // sectors to read
	int BX = *buffer; //(pass your char* array here)
	int CH = 0;		  // track #
	int CL = sector + 1;
	int DH = 0;	   // head number
	int DL = 0x80; // (for the hard disk, use 0x80)

	int AX = AH * 256 + AL;
	int CX = CH * 256 + CL;
	int DX = DH * 256 + DL;

	// call interrupt 0x13
	interrupt(0x13, AX, buffer, CX, DX);
}
void readString(char *chars) // empty char array that can hold 80 elements
{
	int i = 0;
	char c = 0;
	while (1)
	{ // repeat calls
		// BIOS read keypress
		// call interrupt 0x16 repeatedly and save resutls...
		c = (char)interrupt(0x16, 0, 0, 0, 0);
		//...until enter key is pressed (ASCII 0xd)
		if (c == 0x0D)
		{
			chars[i] = 0x0A;	 // line feed
			chars[i + 1] = 0x00; // indicat end of string

			// prinst chars to screen
			interrupt(0x10, 0x0E0D, 0, 0, 0); // move curosor back to start of line
			interrupt(0x10, 0x0E0A, 0, 0, 0); // move cursor down to next line
			break;
		}
		// handle back space (ASCII 0x8)
		else if (c == 0x08)
		{
			// print back space to screen, and decrease array index by 1
			if (i > 0)
			{
				i--; // decrease array index
					 // erase char on screen
				interrupt(0x10, 0x0E08, 0, 0, 0);
				interrupt(0x10, 0x0E20, 0, 0, 0);
				interrupt(0x10, 0x0E08, 0, 0, 0);
			}
		}
		// print char
		else
		{
			chars[i] = c;
			i++;

			// show char
			interrupt(0x10, 0x0E00 + c, 0, 0, 0);
		}
	}
}

void printString(char *chars)
{
	int i = 0;
	while (chars[i] != 0x0)
	{
		interrupt(0x10, 0xe * 256 + chars[i], 0, 0, 0);
		i++;
	}
}

void printChar(char c)
{
	interrupt(0x10, 0xe * 256 + c, 0, 0, 0);
}
void handleInterrupt21(int ax, int bx, int cx, int dx)
{
	// printString("Hello from interrupt 21!\r\n");
	if (ax == 0)
	{
		printString((char *)bx); // print str
	}
	else if (ax == 1)
	{
		readString((char *)bx); // read string into buffer
	}
	else if (ax == 2)
	{
		readSector((char *)bx, cx); // read sector at CX into buffe
	}
	else if (ax == 3) /* <-- ADD THIS BRANCH */
	{
		readFile((char *)bx, (char *)cx, (int *)dx);
	}
	else
	{
		printString("Error: function not found");
	}
}

int matchFileName(char *fileName, char *dirEntry)
{
	int i;
	for (i = 0; i < 6; i++)
	{
		/* If input string ends before 6 chars, pad comparison with null bytes */
		char c = fileName[i];
		if (c == '\r' || c == '\n' || c == '\0')
		{
			c = 0x00;
		}
		if (c != dirEntry[i])
		{
			return 0; /* No match */
		}
	}
	return 1; /* Match found */
}

/* Reads a file by name into a memory buffer */
void readFile(char *fileName, char *buffer, int *sectorsRead)
{
	char dir[512];
	int fileentry;
	int sectorIndex;
	int found = 0;

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
