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

	// ������־���
	void setLogDebug(log_callback callback);

	// ���÷�������ַ
	void setServerIp(std::string ip, uint16_t port);

	// ���ñ�����Ƶ���ص�
	void setLocalVideo(video_frame_callback callback);

	// ����Զ����Ƶ���ص�
	void setRemoteVideo(video_frame_callback callback);

	// ����sdk
	bool initSdk(std::string uid);

	// �ͷ�sdk
	void freeSdk();

	// ���뷿��
	bool joinRoom(std::string rid);

	// �뿪����
	void leaveRoom();

	// �����Ƿ�����
	void setPublish(bool bPub);

	// �����Ƿ���Ļ����
	void setScreen(bool bPub);
	void setScreenPub(bool bPub);
	void setFrameRate(int nFrameRate);

	// ������˷�
	void setMicrophoneMute(bool bMute);

	// ��ȡ��˷�״̬
	bool getMicrophoneMute();

	/*
		������Ϣ�ص�
	*/

	// ����socket�Ͽ���Ϣ
	void respSocketEvent();
	
	// �������˼����֪ͨ
	// json (rid, uid, biz)
	void respPeerJoin(Json::Value jsonObject);

	// ���������뿪��֪ͨ
	// json (rid, uid)
	void respPeerLeave(Json::Value jsonObject);

	// �������������֪ͨ
	// json (rid, uid, mid, sfuid, minfo)
	void respStreamAdd(Json::Value jsonObject);

	// ���������Ƴ���֪ͨ
	// json (rid, uid, mid)
	void respStreamRemove(Json::Value jsonObject);

	// ����������
	// json (rid, uid)
	void respPeerKick(Json::Value jsonObject);

	// д��־
	static void writeLog(std::string msg);

private:
	// MessageHandler implementation
	void OnMessage(rtc::Message* msg) override;

	// PeerConnectionFactory����
	bool initPeerConnectionFactory();
	void freePeerConnectionFactory();

	// ֹͣ����
	void stopPublish();
	// ֹͣ��Ļ����
	void stopScreen();
	// ��������
	void startSubscribe(std::string uid, std::string mid, std::string sfu);
	// ֹͣ����
	void stopSubscribe(std::string mid);
	// ֹͣ��������
	void freeAllRemotePeer();

	// �߳�
	void StartWorkThread();
	void StopWorkThread();
	void StartHeatThread();
	void StopHeatThread();
	static DWORD WINAPI WorkThreadFunc(LPVOID data);
	static DWORD WINAPI HeatThreadFunc(LPVOID data);

public:
	// ��������
	std::string strUid;
	std::string strRid;
	std::string strUrl;

	// �������
	bool bPublish;
	bool bScreen;
	bool bScreenPub;

	// ����״̬
	int mStatus;
	// �رձ��
	bool bRoomClose;

	// �̶߳���
	bool bWorkExit;
	bool bHeatExit;
	HANDLE hWorkThread;
	HANDLE hHeatThread;

	// ֡�ص�����
	video_frame_callback local_callback_;
	video_frame_callback remote_callback_;

	// �������
	ZXClient mZXClient;

	// ����������
	ZXPeerLocal mLocalPeer;
	ZXPeerScreen mScreenPeer;
	std::map<std::string, ZXPeerRemote*> vtRemotePeers;
	std::mutex mutex;

	// RTC����
	std::unique_ptr<rtc::Thread> g_ws_thread_;
	std::unique_ptr<rtc::Thread> g_signaling_thread;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
};

