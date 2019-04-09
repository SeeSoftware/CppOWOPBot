#pragma once
#include <stdint.h>
#include <SFML/System.hpp>

class Bucket
{
public:
	Bucket() = default;
	Bucket(uint32_t per, uint32_t rate) :Per(per), Rate(rate){}

	void Update();
	bool CanSpend(uint32_t num) const;
	bool Spend(uint32_t num);
	void Reset();

	uint32_t GetCurrent() { return currrent; }

public:
	uint32_t Per = 0;
	uint32_t Rate = 0;

private:

	sf::Clock mClock;
	uint32_t currrent = 0;
	float accumulator = 0.0f;
};