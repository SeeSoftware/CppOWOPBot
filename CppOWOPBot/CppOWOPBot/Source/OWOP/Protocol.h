#pragma once
#include "WebsocketppDefs.h"
#include "Structs.h"
#include <cstdint>
#include <vector>
#include <string>
#include <memory>



namespace Protocol
{
	struct IS2CMessage;

	enum class PacketOpCode : uint8_t  //enum class might not be the best thing but eh it stops namespace pollution
	{
		SetID,
		WorldUpdate,
		ChunkLoad,
		Teleport,
		SetRank,
		Captcha,
		SetPQuota,
		ChunkProtected,

		ChatMessage //not an actual opcode
	};


	//just a simple static cast (just for having cleaner code)
	template<typename T>
	std::shared_ptr<T> DowncastMessage(const std::shared_ptr<IS2CMessage> &message)
	{
		return std::static_pointer_cast<T>(message);
	}

	std::shared_ptr<IS2CMessage> ParsePacket(const std::vector<uint8_t> &data, Ws::Opcode wsOpcode);
	std::shared_ptr<IS2CMessage> ParsePacket(const Ws::MessagePtr &message);

	#pragma pack(push,1)
	
	struct IC2SMessage
	{
		IC2SMessage() = default;
		virtual ~IC2SMessage() = default;

		uint8_t __base;

		virtual websocketpp::frame::opcode::value WSOpcode() const = 0;
		virtual std::vector<uint8_t> Serialize() const = 0;
	};


	template<typename T, bool TextOPCode = false>
	struct C2SMessage : public IC2SMessage
	{
		C2SMessage() = default;
		virtual ~C2SMessage() = default;

		virtual Ws::Opcode WSOpcode() const noexcept final { return TextOPCode ? Ws::Opcode::text : Ws::Opcode::binary; }

		virtual std::vector<uint8_t> C2SMessage::Serialize() const
		{
			constexpr uintptr_t VTablePtrSize = sizeof(IC2SMessage); //A bit hacky: offsetting memcpy by vtableptr size

			std::vector<uint8_t> vec(sizeof(T) - VTablePtrSize);
			memcpy(vec.data(), &__base+1, sizeof(T) - VTablePtrSize);

			return vec;
		}
	};


	struct IS2CMessage
	{
		IS2CMessage() = default;
		virtual ~IS2CMessage() = default;

		virtual void Deserialize(const std::vector<uint8_t>& data) = 0;
		virtual std::vector<uint8_t> Serialize() const = 0;

		PacketOpCode opcode;
	};


	template<typename T, bool TextOPCode = false>
	struct S2CMessage : public IS2CMessage
	{
		S2CMessage() = default;
		virtual ~S2CMessage() = default;

		constexpr Ws::Opcode WSOpcode() const noexcept{ return TextOPCode ? Ws::Opcode::text : Ws::Opcode::binary; }

		virtual void S2CMessage::Deserialize(const std::vector<uint8_t>& data)
		{
			constexpr uintptr_t VTablePtrSize = sizeof(IC2SMessage)-1; //(uintptr_t)&opcode - (uintptr_t)this; //A bit hacky: offsetting memcpy by vtableptr size

			memcpy(&opcode, data.data(), std::min(sizeof(T) - VTablePtrSize, data.size()));
		}

		virtual std::vector<uint8_t> S2CMessage::Serialize() const
		{
			constexpr uintptr_t VTablePtrSize = sizeof(IC2SMessage)-1;//(uintptr_t)&opcode - (uintptr_t)this; //A bit hacky: offsetting memcpy by vtableptr size

			std::vector<uint8_t> vec(sizeof(T) - VTablePtrSize);
			memcpy(vec.data(), &opcode, sizeof(T) - VTablePtrSize);

			return vec;
		}
	};



	//Server to client

	//Not really received by server but just as a convinience since chat messages are direct text packets
	struct ChatMessage : public S2CMessage<ChatMessage,true> 
	{
		std::string message;

		virtual void Deserialize(const std::vector<uint8_t>& data) { opcode = PacketOpCode::ChatMessage; message = std::string(data.begin(), data.end()); }
		virtual std::vector<uint8_t> Serialize() const { return std::vector<uint8_t>(); }
	};

	struct SetID : public S2CMessage<SetID>
	{
		OWOP::CursorID id;
	};

	struct WorldUpdate : public S2CMessage<WorldUpdate>
	{
		std::vector<OWOP::CursorData> cursorUpdates;
		std::vector<OWOP::TileUpdate> tileUpdates;
		std::vector<OWOP::CursorID> disconnects;

		virtual void Deserialize(const std::vector<uint8_t>& data);
		virtual std::vector<uint8_t> Serialize() const;
	};

	struct ChunkData : public S2CMessage<ChunkData>
	{
		sf::Vector2i chunkPos;
		bool locked;
		std::vector<uint8_t> compressedData;

		std::vector<OWOP::Color> Decompress() const;

		virtual void Deserialize(const std::vector<uint8_t>& data);
		virtual std::vector<uint8_t> Serialize() const;
	};

	struct Teleport : public S2CMessage<Teleport>
	{
		sf::Vector2i worldPos;
	};

	struct SetRank : public S2CMessage<SetRank>
	{
		OWOP::RankID rank;
	};

	struct CaptchaRequest : public S2CMessage<CaptchaRequest>
	{
		OWOP::CaptchaState state;
	};

	struct SetPQuota : public S2CMessage<SetPQuota>
	{
		uint16_t rate;
		uint16_t per;
	};

	struct ChunkProtected : public S2CMessage<ChunkProtected>
	{
		sf::Vector2i chunkPos;
		bool newState;
	};

	//Client to server
	struct JoinWorld : public C2SMessage<JoinWorld>
	{
		JoinWorld() = default;
		JoinWorld(const std::string worldName) : worldName(worldName){}

		std::string worldName = "";

		virtual std::vector<uint8_t> Serialize() const;
	};

	struct RequestChunk : public C2SMessage<RequestChunk>
	{
		RequestChunk() = default;
		RequestChunk(const sf::Vector2i chunkPos) : chunkPos(chunkPos) {}

		sf::Vector2i chunkPos = sf::Vector2i(0,0);
	};

	struct UpdatePixel : public C2SMessage<UpdatePixel>
	{
		UpdatePixel() = default;
		UpdatePixel(const sf::Vector2i worldPos, const OWOP::Color &pxCol) : worldPos(worldPos),pxColor(pxCol) {}

		sf::Vector2i worldPos = sf::Vector2i(0, 0);
		OWOP::Color pxColor = OWOP::Color(0,0,0);
	};

	struct UpdateCursor : public C2SMessage<UpdateCursor>
	{
		UpdateCursor() = default;
		UpdateCursor(const sf::Vector2i subPos, OWOP::ToolID tool = 0, const OWOP::Color &cursorCol = OWOP::Color(255,255,255)) : subPos(subPos), tool(tool), cursorColor(cursorCol) {}

		sf::Vector2i subPos = sf::Vector2i(0,0); //in pixel * 16 positions
		OWOP::Color cursorColor = OWOP::Color(0,0,0);
		OWOP::ToolID tool = 0;
	};

	struct SendChat : public C2SMessage<ChatMessage,true>
	{
		SendChat() = default;
		SendChat(const std::string message) : message(message) {}

		std::string message = "";

		virtual std::vector<uint8_t> Serialize() const;
	};

	struct ProtectChunk : public C2SMessage<ProtectChunk>
	{
		ProtectChunk() = default;
		ProtectChunk(const sf::Vector2i &chunkPos, bool state = true) : chunkPos(chunkPos),state(state) { }

		sf::Vector2i chunkPos = sf::Vector2i(0,0);
		bool state = true; //TODO: bool is not allways 1 byte probably better to use uint8_t

		uint8_t _unusedPadding = 0; //for some reason you need 1 byte at the end
	};

	struct ClearChunk : public C2SMessage<ClearChunk>
	{
		ClearChunk() = default;
		ClearChunk(const sf::Vector2i &chunkPos,const OWOP::Color &fillColor) : chunkPos(chunkPos),fillColor(fillColor) {}

		sf::Vector2i chunkPos = sf::Vector2i(0, 0);
		OWOP::Color fillColor = OWOP::Color(0, 0, 0);

		uint16_t _unusedPadding = 0; //for some reason you need 2 bytes at the end
	};

	//todo: protectChunk, setChunk, clearChunk

#pragma pack(pop)

}