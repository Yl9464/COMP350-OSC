/* kernel.c
/*Project B -Lensi Gondaliya, Nina Aubourg, Ying Lu */
void readString(char *chars);
void printString(char *);
void printChar(char);
void handleInterrupt21(int, int, int, int);

main()
{
	char line[80];
	char buffer[512];

	// Step 5
	 makeInterrupt21();
	 interrupt(0x21, 1, line, 0, 0);
	 interrupt(0x21, 0, line, 0, 0);
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
	while (1)
		;
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
	else
	{
		printString("Error: function not found");
	}
}
