#pragma once
#include <mutex>
#include "ZXListen.h"
#include "ZXClient.h"
#include "ZXPeerLocal.h"
#include "ZXPeerScreen.h"
#include "ZXPeerRemote.h"
#include "rtc_base/strings/json.h"
#include "api/peerconnectioninterface.h"

class ZXEngine : public rtc::MessageHandler
{
public:
	ZXEngine();
	virtual ~ZXEngine();

	// 设置日志输出
	void setLogDebug(log_callback callback);

	// 设置服务器地址
	void setServerIp(std::string ip, uint16_t port);

	// 设置本地视频流回调
	void setLocalVideo(video_frame_callback callback);

	// 设置远端视频流回调
	void setRemoteVideo(video_frame_callback callback);

	// 加载sdk
	bool initSdk(std::string uid);

	// 释放sdk
	void freeSdk();

	// 加入房间
	bool joinRoom(std::string rid);

	// 离开房间
	void leaveRoom();

	// 设置是否推流
	void setPublish(bool bPub);

	// 设置是否屏幕共享
	void setScreen(bool bPub);
	void setScreenPub(bool bPub);
	void setFrameRate(int nFrameRate);

	// 设置麦克风
	void setMicrophoneMute(bool bMute);

	// 获取麦克风状态
	bool getMicrophoneMute();

	/*
		处理消息回调
	*/

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

	// 写日志
	static void writeLog(std::string msg);

private:
	// MessageHandler implementation
	void OnMessage(rtc::Message* msg) override;

	// PeerConnectionFactory对象
	bool initPeerConnectionFactory();
	void freePeerConnectionFactory();

	// 停止推流
	void stopPublish();
	// 停止屏幕推流
	void stopScreen();
	// 启动拉流
	void startSubscribe(std::string uid, std::string mid, std::string sfu);
	// 停止拉流
	void stopSubscribe(std::string mid);
	// 停止所有拉流
	void freeAllRemotePeer();

	// 线程
	void StartWorkThread();
	void StopWorkThread();
	void StartHeatThread();
	void StopHeatThread();
	static DWORD WINAPI WorkThreadFunc(LPVOID data);
	static DWORD WINAPI HeatThreadFunc(LPVOID data);

public:
	// 基本参数
	std::string strUid;
	std::string strRid;
	std::string strUrl;

	// 推流标记
	bool bPublish;
	bool bScreen;
	bool bScreenPub;

	// 连接状态
	int mStatus;
	// 关闭标记
	bool bRoomClose;

	// 线程对象
	bool bWorkExit;
	bool bHeatExit;
	HANDLE hWorkThread;
	HANDLE hHeatThread;

	// 帧回调对象
	video_frame_callback local_callback_;
	video_frame_callback remote_callback_;

	// 信令对象
	ZXClient mZXClient;

	// 推拉流对象
	ZXPeerLocal mLocalPeer;
	ZXPeerScreen mScreenPeer;
	std::map<std::string, ZXPeerRemote*> vtRemotePeers;
	std::mutex mutex;

	// RTC对象
	std::unique_ptr<rtc::Thread> g_ws_thread_;
	std::unique_ptr<rtc::Thread> g_signaling_thread;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
};

