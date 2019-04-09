#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

namespace std
{
	template <class T>
	struct hash<sf::Vector2<T>>
	{
		std::size_t operator()(const sf::Vector2<T>& v) const
		{
			using std::hash;

			// Compute individual hash values for first
			// and second. Combine them using the Boost-func

			std::size_t tmp0 = hash<T>()(v.x);
			std::size_t tmp1 = hash<T>()(v.y);

			tmp0 ^= tmp1 + 0x9e3779b9 + (tmp0 << 6) + (tmp0 >> 2);

			return tmp0;
		}
	};
}

template<typename T, typename U>
inline sf::Vector2<T> operator* (const sf::Vector2<T> &a, const sf::Vector2<U> &b)
{
	return sf::Vector2<T>(a.x * static_cast<T>(b.x), a.y * static_cast<T>(b.y));
}

template<typename T, typename U>
inline sf::Vector2<T> operator/ (const sf::Vector2<T> &a, const sf::Vector2<U> &b)
{
	return sf::Vector2<T>(a.x / static_cast<U>(b.x), a.y / static_cast<U>(b.y));
}

template<typename T, typename U>
inline sf::Vector2<T> &operator*= (sf::Vector2<T> &a, const sf::Vector2<U> &b)
{
	a.x *= static_cast<T>(b.x);
	a.y *= static_cast<T>(b.y);
	return a;
}

template<typename T, typename U>
inline sf::Vector2<T> &operator/= (sf::Vector2<T> &a, const sf::Vector2<U> &b)
{
	a.x /= static_cast<T>(b.x);
	a.y /= static_cast<T>(b.y);
	return a;
}


namespace sf
{
	template<typename T, typename U>
	float VectorDistance(const sf::Vector2<T> &a, const sf::Vector2<U> &b)
	{
		return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
	}

	template<typename T>
	float VectorLength(const sf::Vector2<T> &a)
	{
		return sqrt(a.x * a.x + a.y * a.y);
	}

	template<typename T, typename U>
	T VectorDotProduct(const sf::Vector2<T> &a, const sf::Vector2<U> &b)
	{
		return a.x * b.x + a.y * b.y;
	}

	template<typename T>
	sf::Vector2<T> VectorNormalize(sf::Vector2<T> a)
	{
		float length = VectorLength(a);
		a.x /= (T)length;
		a.y /= (T)length;
		return a;
	}
}