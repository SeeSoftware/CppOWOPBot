#include "Bucket.h"
#include <limits>

void Bucket::Update()
{
	float dt = mClock.restart().asSeconds();
	if (Per == 0.0f)
	{
		currrent = std::numeric_limits<int>::max();
		return;
	}

	accumulator += dt;

	float timeperone = (1.0f / ((float)Rate / (float)Per));
	int increment = (uint32_t)floor(accumulator / timeperone);
	currrent += increment;
	currrent = std::min(currrent, Rate);
	accumulator -= (float)increment * timeperone;
}

bool Bucket::CanSpend(uint32_t num) const
{
	return currrent >= num;
}

bool Bucket::Spend(uint32_t num)
{
	if (currrent >= num)
	{
		currrent -= num;
		return true;
	}
	return false;
}

void Bucket::Reset()
{
	accumulator = 0;
	currrent = 0;
	mClock.restart();
}
