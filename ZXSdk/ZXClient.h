#pragma once
#include <mutex>
#include <string>
#include "WebSocket.h"

class ZXEngine;
class ZXClient : public WSCall
{
public:
	ZXClient();
	~ZXClient();

public:
	// WSCall implementation
	void onMessage(const std::string& msg) override;
	void onOpen(const std::string& err) override;
	void onClose(const std::string& err) override;
	void onFail(const std::string& err) override;

	// 接收
	void OnDataRecv(const std::string message);

	// 建立ws连接
	bool Start(std::string url);

	// 关闭ws连接
	void Stop();

	// 返回连接状态
	bool GetConnect();

	/*
	  "request":true
	  "id":3764139
	  "method":"join"
	  "data":{
		"rid":"room"
	  }
	*/
	// 加入房间
	bool SendJoin();

	/*
	  "request":true
	  "id":3764139
	  "method":"leave"
	  "data":{
		"rid":"room"
	  }
	*/
	// 退出房间
	void SendLeave();

	/*
	 "request":true
	 "id":3764139
	 "method":"keepalive"
	 "data":{
	   "rid":"room"
	 }
	*/
	// 发送心跳
	void SendAlive();

	/*
	  "request":true
	  "id":3764139
	  "method":"publish"
	  "data":{
		"rid":"room",
		"jsep":{"type":"offer","sdp":"..."},
		"minfo":{
			"audio":true,
			"video":true,
			"videotype":0
		}
	  }
	*/
	// 发布流
	bool SendPublish(std::string sdp, bool bAudio, bool bVideo, int videoType);

	/*
	  "request":true
	  "id":3764139
	  "method":"unpublish"
	  "data":{
		"rid":"room",
		"mid":"64236c21-21e8-4a3d-9f80-c767d1e1d67f#ABCDEF",
		"sfuid":"shenzhen-sfu-1", (可选)
	  }
	*/
	// 取消发布流
	void SendUnPublish(std::string mid, std::string sfuid);

	/*
	  "request":true
	  "id":3764139
	  "method":"subscribe"
	  "data":{
		"rid":"room",
		"mid":"64236c21-21e8-4a3d-9f80-c767d1e1d67f#ABCDEF",
		"jsep":{"type":"offer","sdp":"..."},
		"sfuid":"shenzhen-sfu-1", (可选)
	  }
	*/
	// 订阅流
	bool SendSubscribe(std::string sdp, std::string mid, std::string sfuid);

	/*
	  "request":true
	  "id":3764139
	  "method":"unsubscribe"
	  "data":{
		"rid": "room",
		"mid": "64236c21-21e8-4a3d-9f80-c767d1e1d67f#ABCDEF"
		"sid": "64236c21-21e8-4a3d-9f80-c767d1e1d67f#ABCDEF"
		"sfuid":"shenzhen-sfu-1", (可选)
	  }
	*/
	// 取消订阅流
	void SendUnSubscribe(std::string mid, std::string sid, std::string sfuid);

public:
	// 上层对象
	ZXEngine *pZXEngine;

	// 临时变量
	std::string strSfu;
	std::string strMid;
	std::string strSdp;
	std::string strSid;

	bool close_;
	bool connect_;
	std::mutex mutex_;
	WebSocketClient websocket_;

	// 信令参数
	int nIndex;
	int nType;
	int nRespOK;
	bool bRespResult;
};

