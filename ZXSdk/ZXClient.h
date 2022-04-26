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

	// ����
	void OnDataRecv(const std::string message);

	// ����ws����
	bool Start(std::string url);

	// �ر�ws����
	void Stop();

	// ��������״̬
	bool GetConnect();

	/*
	  "request":true
	  "id":3764139
	  "method":"join"
	  "data":{
		"rid":"room"
	  }
	*/
	// ���뷿��
	bool SendJoin();

	/*
	  "request":true
	  "id":3764139
	  "method":"leave"
	  "data":{
		"rid":"room"
	  }
	*/
	// �˳�����
	void SendLeave();

	/*
	 "request":true
	 "id":3764139
	 "method":"keepalive"
	 "data":{
	   "rid":"room"
	 }
	*/
	// ��������
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
	// ������
	bool SendPublish(std::string sdp, bool bAudio, bool bVideo, int videoType);

	/*
	  "request":true
	  "id":3764139
	  "method":"unpublish"
	  "data":{
		"rid":"room",
		"mid":"64236c21-21e8-4a3d-9f80-c767d1e1d67f#ABCDEF",
		"sfuid":"shenzhen-sfu-1", (��ѡ)
	  }
	*/
	// ȡ��������
	void SendUnPublish(std::string mid, std::string sfuid);

	/*
	  "request":true
	  "id":3764139
	  "method":"subscribe"
	  "data":{
		"rid":"room",
		"mid":"64236c21-21e8-4a3d-9f80-c767d1e1d67f#ABCDEF",
		"jsep":{"type":"offer","sdp":"..."},
		"sfuid":"shenzhen-sfu-1", (��ѡ)
	  }
	*/
	// ������
	bool SendSubscribe(std::string sdp, std::string mid, std::string sfuid);

	/*
	  "request":true
	  "id":3764139
	  "method":"unsubscribe"
	  "data":{
		"rid": "room",
		"mid": "64236c21-21e8-4a3d-9f80-c767d1e1d67f#ABCDEF"
		"sid": "64236c21-21e8-4a3d-9f80-c767d1e1d67f#ABCDEF"
		"sfuid":"shenzhen-sfu-1", (��ѡ)
	  }
	*/
	// ȡ��������
	void SendUnSubscribe(std::string mid, std::string sid, std::string sfuid);

public:
	// �ϲ����
	ZXEngine *pZXEngine;

	// ��ʱ����
	std::string strSfu;
	std::string strMid;
	std::string strSdp;
	std::string strSid;

	bool close_;
	bool connect_;
	std::mutex mutex_;
	WebSocketClient websocket_;

	// �������
	int nIndex;
	int nType;
	int nRespOK;
	bool bRespResult;
};

