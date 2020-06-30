#include "Bucket.h"
#include <limits>

void Bucket::Update()
{
	float dt = mClock.restart().asSeconds();
	if (Per == 0)
	{
		mCurrent = std::numeric_limits<int>::max();
		return;
	}

	mAccumulator += dt;

	float timeperone = (1.0f / ((float)Rate / (float)Per));
	int increment = (uint32_t)floor(mAccumulator / timeperone);
	mCurrent += increment;
	mCurrent = std::min(mCurrent, Rate);
	mAccumulator -= (float)increment * timeperone;
}

bool Bucket::CanSpend(uint32_t num) const
{
	return mCurrent >= num;
}

bool Bucket::Spend(uint32_t num)
{
	if (mCurrent >= num)
	{
		mCurrent -= num;
		return true;
	}
	return false;
}

void Bucket::Reset()
{
	mAccumulator = 0;
	mCurrent = 0;
	mClock.restart();
}
