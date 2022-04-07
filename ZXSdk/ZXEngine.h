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

	// ���������������ַ
	void setServerIp(std::string ip, uint16_t port);

	// ����sdk�ص�
	void setSdkListen(msg_callback observer);

	// ��ʼ��sdk
	bool initSdk(std::string uid);

	// �ͷ�sdk
	void freeSdk();

	// ��������
	bool start();

	// ֹͣ����
	void stop();

	// ��������״̬
	bool getConnect();

	// ���뷿��
	bool joinRoom(std::string rid);

	// �뿪����
	void leaveRoom();

	// ������˷�
	void setMicrophoneMute(bool bMute);

	// ��ȡ��˷�״̬
	bool getMicrophoneMute();

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

	// �����ص�
	void OnPeerPublish(bool bSuc, std::string error);

	// �������ӶϿ��ص�
	void OnPeerPublishError();

	// �����ص�
	void OnPeerSubscribe(std::string mid, bool bSuc, std::string error);

	// �������ӶϿ��ص�
	void OnPeerSubscribeError(std::string mid);

private:
	// PeerConnectionFactory����
	bool initPeerConnectionFactory();
	void freePeerConnectionFactory();

	// ��������
	void startPublish();
	// ֹͣ����
	void stopPublish();
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

	// ״̬����
	bool room_close_;
	bool socket_close_;

	// ����״̬
	int mStatus;
	// ����״̬
	bool bHeatOk;
	// �̶߳���
	bool bWorkExit;
	bool bHeatExit;
	HANDLE hWorkThread;
	HANDLE hHeatThread;

	// �ص�����
	msg_callback mListen;

	// �������
	ZXClient mZXClient;
	Json::Value json_add_;
	Json::Value json_remove_;

	// ����������
	ZXPeerLocal mLocalPeer;
	std::map<std::string, ZXPeerRemote*> vtRemotePeers;
	std::mutex mutex;

	// RTC����
	std::unique_ptr<rtc::Thread> g_ws_thread_;
	std::unique_ptr<rtc::Thread> g_signaling_thread;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
};

