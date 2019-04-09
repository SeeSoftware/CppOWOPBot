#pragma once
#include <memory>
#include <websocketpp/config/asio_client.hpp>
//simple defenitions for websocketpp library

//forward decalre
namespace websocketpp
{
	template<typename config>
	class client;
}

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

namespace Ws
{
	using Opcode = websocketpp::frame::opcode::value;
	using TLSClient = websocketpp::client<websocketpp::config::asio_tls_client>;
	using Client = websocketpp::client<websocketpp::config::asio_client>;
	using MessagePtr = websocketpp::config::asio_tls_client::message_type::ptr;
	using ContextPtr = std::shared_ptr<boost::asio::ssl::context>;
	using ConnectionHdl = websocketpp::connection_hdl;
}