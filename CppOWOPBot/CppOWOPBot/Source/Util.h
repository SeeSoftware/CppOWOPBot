#pragma once
#include <cstdint>
#include <sstream>

//Unsafe but a neat utility

namespace Util
{

	class DataView
	{
	public:
		DataView() = default;
		DataView(uint8_t * start, size_t size) :StartPtr(start), Size(size) {};
		~DataView() = default;

		template <class T>
		T GetValue(size_t position) const
		{
			if (position + sizeof(T) > Size)
				return T();
			return *(T*)(StartPtr + position);
		}

		template <class T>
		void SetValue(size_t position, T newData)
		{
			if (position + sizeof(T) > Size)
				return;
			*(T*)(StartPtr + position) = newData;
		}

		uint8_t *StartPtr;
		size_t Size;
	};

	class ConstDataView
	{
	public:
		ConstDataView() = default;
		ConstDataView(const uint8_t * start, size_t size) :StartPtr(start), Size(size) {};
		~ConstDataView() = default;

		template <class T>
		T GetValue(size_t position) const
		{
			if (position + sizeof(T) > Size)
				return T();
			return *(T*)(StartPtr + position);
		}
		const uint8_t *StartPtr;
		size_t Size;
	};


	extern std::stringstream HexDump(const uint8_t *data, size_t length);

}
