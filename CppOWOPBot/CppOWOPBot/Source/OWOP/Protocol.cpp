#include "Protocol.h"
#include <assert.h>
#include "Util.h"


namespace Protocol
{

	

	void WorldUpdate::Deserialize(const std::vector<uint8_t>& data)
	{
		Util::ConstDataView view(data.data(), data.size());
		opcode = view.GetValue<PacketOpCode>(0);

		//cursors
		cursorUpdates.resize(view.GetValue<uint8_t>(1));
		for (int i = view.GetValue<uint8_t>(1); i--;)
		{
			OWOP::CursorData cData;
			memcpy(&cData, data.data() + 2 + i * 16, sizeof(OWOP::CursorData));
			cursorUpdates[i] = cData;
		}


		size_t off = 2 + view.GetValue<uint8_t>(1) * 16;

		//tiles
		tileUpdates.resize(view.GetValue<uint16_t>(off));
		for (uint16_t i = view.GetValue<uint16_t>(off), j = 0; j < i; j++)
		{
			OWOP::TileUpdate tData;
			memcpy(&tData, data.data() + 2 + off + j * 15, sizeof(OWOP::TileUpdate));
			tileUpdates[j] = tData;
		}


		//disconnects
		off += view.GetValue<uint16_t>(off) * 15 + 2;
		disconnects.resize(view.GetValue<uint8_t>(off));
		for (int i = view.GetValue<uint8_t>(off); i--;)
		{
			uint32_t id = view.GetValue<uint32_t>(1 + off + i * 4);
			disconnects[i] = id;
		}

	}

	std::vector<uint8_t> WorldUpdate::Serialize() const
	{
		assert(0 && "Not implemented");
		return std::vector<uint8_t>();
	}



	std::vector<OWOP::Color> ChunkData::Decompress() const
	{
		size_t len = compressedData[1] << 8 | compressedData[0];
		std::vector<OWOP::Color> decompressedDataVec(len/3);
		uint8_t* decompressedData = (uint8_t*)decompressedDataVec.data(); //might be a bit unsafe but operator[] is unsafe anyways :D (pretty much the same thing)

		int numRepeates = compressedData[3] << 8 | compressedData[2];
		int offset = numRepeates * 2 + 4;
		int uptr = 0;
		int cptr = offset;
		for (int i = 0; i < numRepeates; i++)
		{
			int currentRepeatLoc = (compressedData[4 + i * 2 + 1] << 8 | compressedData[4 + i * 2]) + offset;
			while (cptr < currentRepeatLoc)
				decompressedData[uptr++] = compressedData[cptr++];

			int repeatedNum = compressedData[cptr + 1] << 8 | compressedData[cptr];
			int repeatedColorR = compressedData[cptr + 2];
			int repeatedColorG = compressedData[cptr + 3];
			int repeatedColorB = compressedData[cptr + 4];
			cptr += 5;
			while (repeatedNum--)
			{
				decompressedData[uptr] = repeatedColorR;
				decompressedData[uptr + 1] = repeatedColorG;
				decompressedData[uptr + 2] = repeatedColorB;
				uptr += 3;
			}

		}
		while (cptr < compressedData.size())
		{
			decompressedData[uptr++] = compressedData[cptr++];
		}
		return decompressedDataVec;
	}

	void ChunkData::Deserialize(const std::vector<uint8_t>& data)
	{
		compressedData.resize(data.size() - 10);
		memcpy(&opcode, data.data(), 10); //copies opcode, xy, locked
		memcpy(compressedData.data(),data.data() + 10, data.size() - 10);
	}

	std::vector<uint8_t> ChunkData::Serialize() const
	{
		assert(0 && "Not implemented");
		return std::vector<uint8_t>();
	}



	std::vector<uint8_t> JoinWorld::Serialize() const
	{
		std::string truncatedName = worldName.substr(0, OWOP::MAX_WORLDNAME_LENGTH);
		std::vector<uint8_t> data(truncatedName.size() + sizeof(OWOP::WORLD_VERIFICATION));

		memcpy(data.data(), truncatedName.data(), truncatedName.size());

		Util::DataView view(data.data(), data.size());
		view.SetValue(truncatedName.size(), OWOP::WORLD_VERIFICATION);

		return data;
		
	}

	std::vector<uint8_t> SendChat::Serialize() const
	{
		std::string msg = message + OWOP::CHAT_VERIFICATION;
		return std::vector<uint8_t>(msg.begin(), msg.end());
	}

	std::shared_ptr<IS2CMessage> ParsePacket(const std::vector<uint8_t> &data, Ws::Opcode wsOpcode)
	{
		std::shared_ptr<IS2CMessage> msg;

		Util::ConstDataView view(data.data(), data.size());
		PacketOpCode opcode = view.GetValue<PacketOpCode>(0);

		if (wsOpcode == Ws::Opcode::text) //special case
		{
			msg = std::make_shared<ChatMessage>();
			msg->Deserialize(data);
			return msg;
		}

		switch (opcode)
		{
			case PacketOpCode::SetID:
				msg = std::make_shared<SetID>();
				break;
			case PacketOpCode::WorldUpdate:
				msg = std::make_shared<WorldUpdate>();
				break;
			case PacketOpCode::ChunkLoad:
				msg = std::make_shared<ChunkData>();
				break;
			case PacketOpCode::Teleport:
				msg = std::make_shared<Teleport>();
				break;
			case PacketOpCode::SetRank:
				msg = std::make_shared<SetRank>();
				break;
			case PacketOpCode::Captcha:
				msg = std::make_shared<CaptchaRequest>();
				break;
			case PacketOpCode::SetPQuota:
				msg = std::make_shared<SetPQuota>();
				break;
			case PacketOpCode::ChunkProtected:
				msg = std::make_shared<ChunkProtected>();
				break;
			default:
				msg = std::shared_ptr<IS2CMessage>();
				break;
		}

		if (!msg)
			return msg;

		msg->Deserialize(data);
		return msg;

	}

	std::shared_ptr<IS2CMessage> ParsePacket(const Ws::MessagePtr & message)
	{
		std::vector<uint8_t> data(message->get_payload().size()); //u win you get a copy of my data ffs
		memcpy(data.data(), message->get_payload().data(), data.size());

		return ParsePacket(data, message->get_opcode());
	}

}