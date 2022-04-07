#include "pch.h"
#include "WebSocket.h"
#include <thread>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "rtc_base/logging.h"

WebSocketClient::WebSocketClient()
{
	m_call = nullptr;
}

WebSocketClient::~WebSocketClient()
{

}

void WebSocketClient::set_callback(WSCall * call)
{
	m_call = call;
}

bool WebSocketClient::open(const std::string uri)
{
	close();

	using websocketpp::lib::placeholders::_1;
	using websocketpp::lib::placeholders::_2;
	using websocketpp::lib::bind;

	m_endpoint = std::unique_ptr<client>(new client);
	m_endpoint->set_access_channels(websocketpp::log::alevel::none);
	//m_endpoint->set_error_channels(websocketpp::log::elevel::all);

	// Initialize ASIO
	m_endpoint->init_asio();

	// Register our handlers
	m_endpoint->set_socket_init_handler(bind(&WebSocketClient::onSocketInit, this, _1));
	m_endpoint->set_message_handler(bind(&WebSocketClient::onMessage, this, _1, _2));
	m_endpoint->set_open_handler(bind(&WebSocketClient::onOpen, this, _1));
	m_endpoint->set_close_handler(bind(&WebSocketClient::onClose, this, _1));
	m_endpoint->set_fail_handler(bind(&WebSocketClient::onFail, this, _1));

	//m_endpoint->start_perpetual();

	websocketpp::lib::error_code ec;
	client::connection_ptr con = m_endpoint->get_connection(uri, ec);
	if (ec)
	{
		RTC_LOG(LS_ERROR) << "could not create connection because:" << ec.message();
		return false;
	}

	m_hdl = con->get_handle();
	m_endpoint->connect(con);

	std::thread thread(WebSocketClient::open_thread_, this);
	thread.detach();
	return true;
}

bool WebSocketClient::send(const std::string& msg)
{
	if (msg.empty())
	{
		return false;
	}
	if (m_hdl.expired())
	{
		RTC_LOG(LS_ERROR) << "client ws expired";
		return false;
	}
	if (m_endpoint.get() != nullptr)
	{
		m_endpoint->send(m_hdl, msg, websocketpp::frame::opcode::text);
		return true;
	}
	return false;
}

void WebSocketClient::close()
{
	if (m_endpoint.get() != nullptr)
	{
		try
		{
			websocketpp::lib::error_code ec;
			m_endpoint->close(m_hdl, websocketpp::close::status::normal, "", ec);
			if (ec)
			{
				RTC_LOG(LS_ERROR) << "ws close error = " << ec.message();
			}
		}
		catch (websocketpp::exception &e)
		{
			RTC_LOG(LS_ERROR) << e.what();
		}
		catch (std::exception &e)
		{
			RTC_LOG(LS_ERROR) << e.what();
		}
		//m_endpoint->stop_perpetual();
		m_endpoint.reset();
	}
}

void WebSocketClient::open_thread_(WebSocketClient *pWebsocket)
{
	try
	{
		pWebsocket->m_endpoint->run();
	}
	catch (websocketpp::exception &e)
	{
		RTC_LOG(LS_ERROR) << e.what();
	}
	catch (std::exception &e)
	{
		RTC_LOG(LS_ERROR) << e.what();
	}
}

void WebSocketClient::onSocketInit(websocketpp::connection_hdl hdl)
{
	RTC_LOG(LS_ERROR) << "client socket init";
}

void WebSocketClient::onMessage(websocketpp::connection_hdl hdl, message_ptr msg)
{
	if (m_call != nullptr)
	{
		m_call->onMessage(msg->get_payload());
	}
}

void WebSocketClient::onOpen(websocketpp::connection_hdl hdl)
{
	RTC_LOG(LS_ERROR) << "client socket open";
	if (m_call != nullptr)
	{
		m_call->onOpen("");
	}
}

void WebSocketClient::onClose(websocketpp::connection_hdl hdl)
{
	RTC_LOG(LS_ERROR) << "client socket close";
	if (m_call != nullptr)
	{
		m_call->onClose(getError(hdl));
	}
}

void WebSocketClient::onFail(websocketpp::connection_hdl hdl)
{
	RTC_LOG(LS_ERROR) << "client socket fail";
	if (m_call != nullptr)
	{
		m_call->onFail(getError(hdl));
	}
}

std::string WebSocketClient::getError(websocketpp::connection_hdl hdl)
{
	client::connection_ptr con = m_endpoint->get_con_from_hdl(hdl);

	boost::property_tree::ptree ptConnect;
	ptConnect.put<websocketpp::session::state::value>("state", con->get_state());
	ptConnect.put<websocketpp::close::status::value>("local_close_code", con->get_local_close_code());
	ptConnect.put<std::string>("local_close_reason", con->get_local_close_reason());
	ptConnect.put<websocketpp::close::status::value>("remote_close_code", con->get_remote_close_code());
	ptConnect.put<std::string>("remote_close_reason", con->get_remote_close_reason());
	ptConnect.put<std::error_code>("error_code", con->get_ec());
	ptConnect.put<std::string>("message", con->get_ec().message());
	ptConnect.put<std::string>("host", con->get_host());

	boost::property_tree::ptree pt;
	pt.add_child("connect", ptConnect);

	std::ostringstream oss;
	boost::property_tree::write_json(oss, pt, false);
	return oss.str();
}