#include "Util.h"
#include <iomanip>

std::stringstream Util::HexDump(const uint8_t * data, size_t length)
{
	std::stringstream stream;

	stream.fill('0');
	stream << "Dumping " << std::dec << length << " bytes:\n";
	for (size_t stride = 0; stride < (length / 16) + 1; stride++)
	{
		int printedchars = 0;
		for (size_t i = 0; i < 16 && (i + (stride * 16)) < length; i++)
		{
			size_t index = (i + (stride * 16));
			stream << std::setw(2) << std::hex << (int)data[index] << " ";
			printedchars += 3;
		}

		for (size_t i = 0; i < ((3 * 16) + 1) - printedchars; i++)
			stream << " ";

		for (size_t i = 0; i < 16 && (i + (stride * 16)) < length; i++)
		{
			size_t index = (i + (stride * 16));
			stream << (char)((data[index] >= 32 && data[index] <= 126) ? data[index] : '.');
		}

		stream << "\n";
	}

	return stream;
}
