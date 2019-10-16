#pragma once
#include <Windows.h>
#include <string>

namespace ConsoleHelper
{
	DWORD GetNumCharsInConsoleBuffer();
	DWORD ReadConsoleBuffer(char* buffer, DWORD bufsize, COORD firstChar = { 0 });
	void GetConsoleRect(int &left, int &top, int & width, int & height);
	std::string GetTerminalSmapshot();
};

