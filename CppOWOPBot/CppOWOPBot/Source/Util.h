#pragma once
#include <cstdint>
#include <sstream>
#include<type_traits>
#include<utility>

//Unsafe but a neat utility

namespace Util
{


	template<typename Callable>
	union storage
	{
		storage() {}
		std::decay_t<Callable> callable;
	};

	template<int, typename Callable, typename Ret, typename... Args>
	auto fnptr_(Callable&& c, Ret(*)(Args...))
	{
		static bool used = false;
		static storage<Callable> s;
		using type = decltype(s.callable);

		if (used)
			s.callable.~type();
		new (&s.callable) type(std::forward<Callable>(c));
		used = true;

		return [](Args... args) -> Ret
		{
			return Ret(s.callable(std::forward<Args>(args)...));
		};
	}

	template<typename Fn, int N = 0, typename Callable>
	Fn* fnptr(Callable&& c)
	{
		return fnptr_<N>(std::forward<Callable>(c), (Fn*)nullptr);
	}
	
	
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


	inline int div_floor(int a, int b)
	{
		int res = a / b;
		int rem = a % b;
		// Correct division result downwards if up-rounding happened,
		// (for non-zero remainder of sign different than the divisor).
		int corr = (rem != 0 && ((rem < 0) != (b < 0)));
		return res - corr;
	}

}


