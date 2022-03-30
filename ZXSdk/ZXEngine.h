#pragma once
#include <mutex>
#include "ZXListen.h"
#include "ZXClient.h"
#include "ZXPeerLocal.h"
#include "ZXPeerRemote.h"
#include "rtc_base/strings/json.h"
#include "api/peerconnectioninterface.h"

class ZXEngine : public rtc::MessageHandler
{
public:
	ZXEngine();
	virtual ~ZXEngine();

	// MessageHandler implementation
	void OnMessage(rtc::Message* msg) override;

	// 设置信令服务器地址
	void setServerIp(std::string ip, uint16_t port);

	// 设置sdk回调
	void setSdkListen(msg_callback observer);

	// 初始化sdk
	bool initSdk(std::string uid);

	// 释放sdk
	void freeSdk();

	// 启动连接
	bool start();

	// 停止连接
	void stop();

	// 返回连接状态
	bool getConnect();

	// 加入房间
	bool joinRoom(std::string rid);

	// 离开房间
	void leaveRoom();

	// 启动推流
	void startPublish();

	// 停止推流
	void stopPublish();

	// 启动拉流
	void startSubscribe(std::string uid, std::string mid, std::string sfu);

	// 停止拉流
	void stopSubscribe(std::string mid);

	// 设置麦克风
	void setMicrophoneMute(bool bMute);

	// 获取麦克风状态
	bool getMicrophoneMute();

	// 处理socket断开消息
	void respSocketEvent();
	
	// 处理有人加入的通知
	// json (rid, uid, biz)
	void respPeerJoin(Json::Value jsonObject);

	// 处理有人离开的通知
	// json (rid, uid)
	void respPeerLeave(Json::Value jsonObject);

	// 处理有流加入的通知
   // json (rid, uid, mid, sfuid, minfo)
	void respStreamAdd(Json::Value jsonObject);

	// 处理有流移除的通知
	// json (rid, uid, mid)
	void respStreamRemove(Json::Value jsonObject);

	// 处理被踢下线
	// json (rid, uid)
	void respPeerKick(Json::Value jsonObject);

	// 推流回调
	void OnPeerPublish(bool bSuc, std::string error);

	// 推流连接断开回调
	void OnPeerPublishError();

	// 拉流回调
	void OnPeerSubscribe(std::string mid, bool bSuc, std::string error);

	// 拉流连接断开回调
	void OnPeerSubscribeError(std::string mid);

private:
	// PeerConnectionFactory对象
	bool initPeerConnectionFactory();
	void freePeerConnectionFactory();

	// 释放对象
	void freeLocalPeer();
	void freeAllRemotePeer();

public:
	// 基本参数
	std::string strUid;
	std::string strRid;
	std::string strUrl;

	// 回调对象
	msg_callback mListen;

	// 信令对象
	ZXClient mZXClient;

	// 推拉流对象
	ZXPeerLocal mLocalPeer;
	std::map<std::string, ZXPeerRemote*> vtRemotePeers;
	std::mutex mutex;

	// RTC对象
	std::unique_ptr<rtc::Thread> g_ws_thread_;
	std::unique_ptr<rtc::Thread> g_signaling_thread;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
};

