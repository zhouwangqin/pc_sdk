#pragma once
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

class WSCall
{
public:
	virtual ~WSCall() {};
	virtual void onMessage(const std::string& msg) = 0;
	virtual void onClose(const std::string& err) {};
	virtual void onFail(const std::string& err) {};
	virtual void onOpen(const std::string& err) {};
};

class WebSocketClient
{
	typedef websocketpp::client<websocketpp::config::asio_client> client;
	typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

public:
	WebSocketClient();
	virtual ~WebSocketClient();

public:
	void set_callback(WSCall* call);
	bool open(const std::string uri);
	bool send(const std::string& msg);
	void close();

	virtual void onSocketInit(websocketpp::connection_hdl hdl);
	virtual void onMessage(websocketpp::connection_hdl hdl, message_ptr msg);
	virtual void onOpen(websocketpp::connection_hdl hdl);
	virtual void onClose(websocketpp::connection_hdl hdl);
	virtual void onFail(websocketpp::connection_hdl hdl);

protected:
	static void open_thread_(WebSocketClient *pWebsocket);
	std::string getError(websocketpp::connection_hdl hdl);
	
protected:
	WSCall* m_call;
	std::unique_ptr<client> m_endpoint;
	websocketpp::connection_hdl m_hdl;
};

