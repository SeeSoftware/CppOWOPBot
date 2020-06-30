#include "ConsoleHelper.h"

#include <vector>

namespace ConsoleHelper
{

	DWORD GetNumCharsInConsoleBuffer()
	{
		CONSOLE_SCREEN_BUFFER_INFO buffer_info;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buffer_info);
		return buffer_info.dwSize.X * buffer_info.dwSize.Y;

	}

	DWORD ReadConsoleBuffer(char* buffer, DWORD bufsize, COORD firstChar)
	{
		DWORD num_character_read = 0;
		ReadConsoleOutputCharacterA(GetStdHandle(STD_OUTPUT_HANDLE), buffer, bufsize, firstChar, &num_character_read);
		//buffer[bufsize - 1] = '\n';

		return num_character_read;
	}

	DWORD ReadConsoleAttributes(uint16_t* buffer, DWORD bufsize, COORD firstChar)
	{
		DWORD num_character_read = 0;
		ReadConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE), buffer, bufsize, firstChar, &num_character_read);
		//buffer[bufsize - 1] = '\n';

		return num_character_read;
	}

	void GetConsoleRect(int &left, int &top, int & width, int & height)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		left = csbi.srWindow.Left;
		top = csbi.srWindow.Top;
	}



	std::string GetTerminalSmapshot()
	{
		std::string output;
		int left, top, width, height;

		GetConsoleRect(left, top, width, height);

		std::vector<std::string> lines;

		DWORD size = width * height;
		char *buff = new char[size];
		ReadConsoleBuffer(buff, size, { 0,(short)top });


		for (int count = 0; count < (int)size; count += width)
		{
			lines.emplace_back(buff + count, min(width, (int)size - (int)count) - 1);
		}

		delete[] buff;

		for (unsigned int y = 0; y < lines.size(); y++)
		{
			output.append(lines[y]);
			output.append("\n");
		}

		return output;
	}


}