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

	void SetCurrent(uint32_t newCurrent) { mCurrent = newCurrent; }
	uint32_t GetCurrent() { return mCurrent; }

public:
	uint32_t Per = 0;
	uint32_t Rate = 0;

private:

	sf::Clock mClock;
	uint32_t mCurrent = 0;
	float mAccumulator = 0.0f;
};