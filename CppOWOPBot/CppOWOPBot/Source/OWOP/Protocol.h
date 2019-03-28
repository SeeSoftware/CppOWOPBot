#pragma once
#include "Structs.h"
#include <cstdint>
#include <vector>

//Unsafe but a neat utility
class DataView
{
public:
	DataView() = default;
	DataView(uint8_t * start, size_t size) :StartPtr(start),Size(size) {};
	virtual ~DataView() = default;

	template <class T>
	T GetValue(size_t position) const
	{
		if (position + sizeof(T) > Size())
			return T();
		return *(T*)(StartPtr + position);
	}

	template <class T>
	void SetValue(size_t position, T newData)
	{
		if (position + sizeof(T) > Size())
			return;
		*(T*)(StartPtr + position) = newData;
	}

	uint8_t *StartPtr;
	size_t Size;
};

namespace Messages
{
	#pragma pack(push,1)

	template<typename T>
	struct BaseMessage
	{
		PacketOpCode opcode;

		virtual void BaseMessage::Deserialize(const std::vector<uint8_t>& data)
		{
			constexpr uintptr_t VTablePtrSize = &opcode - this; //A bit hacky: offsetting memcpy by vtableptr size

			memcpy(data.data(), &opcode, sizeof(T) - VTablePtrSize);
		}

		virtual std::vector<uint8_t> BaseMessage::Serialize()
		{
			constexpr uintptr_t VTablePtrSize = &opcode - this; //A bit hacky: offsetting memcpy by vtableptr size

			std::vector<uint8_t> vec(sizeof(T) - VTablePtrSize);
			memcpy(vec.data(), &opcode, sizeof(T) - VTablePtrSize);

		}
	};
	

	//Receive
	struct SetID : public BaseMessage<SetID>
	{
		uint32_t id;
	};

	struct WorldUpdate : public BaseMessage<WorldUpdate>
	{

	};

	struct Teleport : public BaseMessage<Teleport>
	{
		sf::Vector2i pos;
	};

	struct SetRank : public BaseMessage<SetRank>
	{
		uint8_t rank;
	};

	struct CaptchaRequest : public BaseMessage<CaptchaRequest>
	{
		CaptchaState state;
	};

	struct SetPQuota : public BaseMessage<SetPQuota>
	{
		uint16_t rate;
		uint16_t per;
	};

	struct ChunkProtected : public BaseMessage<ChunkProtected>
	{
		sf::Vector2i chunkPos;
		bool newState;
	};

	//Send


	
#pragma pack(pop)
}