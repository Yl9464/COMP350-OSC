/* kernel.c
/*Project B -Lensi Gondaliya, Nina Aubourg, Ying Lu */
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

main()
{
	makeInterrupt21();
	interrupt(0x21, 0x400, (int)"tstpr1", 0, 0);
	while (1)
		;
	// char buffer[13312]; // Buffer sized for max file size (26 sectors * 512 bytes)
	// int sectorsRead = 0;

	// // Execute INT 0x21 service 3
	// interrupt(0x21, 3, 'messag', buffer, &sectorsRead);

	// if (sectorsRead > 0)
	// {
	// 	// File successfully loaded into buffer
	// 	printString(buffer);
	// }
	// else
	// {
	// 	printString("File not found.\r\n\0");
	// }

	// while (1)
	// 	;
}
void executeProgram(char *name)
{
	char buffer[13312]; // Buffer for maximum file size (26 sectors * 512 bytes)
	int sectorsRead = 0;
	int totalBytes;
	int i;

	// 1. Load the executable file into local buffer using readFile
	readFile(name, buffer, &sectorsRead);

	// If the file was not found (sectorsRead == 0), abort execution
	if (sectorsRead == 0)
	{
		return;
	}

	// 2. Transfer file contents byte-by-byte into memory segment 0x2000 offset i
	totalBytes = sectorsRead * 512;
	for (i = 0; i < totalBytes; i++)
	{
		putInMemory(0x2000, i, buffer[i]);
	}

	// 3. Launch program at segment 0x2000 (control never returns from launchProgram)
	launchProgram(0x2000);
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
void readFile(char *filename, char *buffer, int *sectorsRead)
{
	char dir[512];
	int entry, i, s;
	int fileFound = 0;
	int fileEntry = -1;
	int entryOffset;
	int match;
	int entrySectorOffset;
	unsigned char sectorNum;

	// 1. Load directory sector (Sector 2)
	readSector(dir, 2);

	// 2. Search 16 directory entries (32 bytes each)
	for (entry = 0; entry < 16; entry++)
	{
		entryOffset = entry * 32;

		// Skip empty directory entries
		if (dir[entryOffset] == 0)
			continue;

		match = 1;
		for (i = 0; i < 6; i++)
		{
			if (filename[i] != dir[entryOffset + i])
			{
				match = 0;
				break;
			}
			if (filename[i] == '\0')
			{
				break;
			}
		}

		if (match)
		{
			fileFound = 1;
			fileEntry = entry;
			break;
		}
	}

	// File not found: set sectors read to 0 and return
	if (!fileFound)
	{
		*sectorsRead = 0;
		return;
	}

	// 3. Read the file sector-by-sector into the buffer
	*sectorsRead = 0;
	entrySectorOffset = fileEntry * 32 + 6;

	for (s = 0; s < 26; s++)
	{
		sectorNum = (unsigned char)dir[entrySectorOffset + s];

		// Sector 0 marks end of file allocation
		if (sectorNum == 0)
			break;

		readSector(buffer, sectorNum);
		buffer += 512;
		(*sectorsRead)++;
	}
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
	else if (ax == 3)
	{
		readFile((char *)bx, (char *)cx, (int *)dx);
	}
	else if (ax == 4)
	{
		// AX = 4: Load program and execute it
		// BX = address of program name string
		executeProgram((char *)bx);
	}
	else
	{
		printString("Error: function not found");
	}
}
