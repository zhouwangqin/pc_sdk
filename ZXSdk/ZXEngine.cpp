#include "pch.h"
#include "ZXEngine.h"
#include "ZXBase.h"

#include "rtc_base/checks.h"
#include "rtc_base/ssladapter.h"
#include "rtc_base/strings/json.h"
#include "rtc_base/win32socketinit.h"
#include "rtc_base/win32socketserver.h"
#include "system_wrappers/include/field_trial.h"

#include "api/create_peerconnection_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"

ZXEngine::ZXEngine()
{
	strUid = "";
	strRid = "";
	strUrl = "";

	room_close_ = false;
	socket_close_ = false;

	mStatus = 0;
	bHeatOk = false;
	bWorkExit = false;
	bHeatExit = false;
	hWorkThread = NULL;
	hHeatThread = NULL;

	mListen = nullptr;
	vtRemotePeers.clear();
	peer_connection_factory_ = nullptr;

	mZXClient.pZXEngine = this;
	mLocalPeer.pZXEngine = this;

	// 打开日志
	rtc::LogMessage::LogToDebug(rtc::LS_ERROR);
	// 启动线程
	g_ws_thread_ = rtc::Thread::CreateWithSocketServer();
	g_ws_thread_->SetName("websocket_thread", nullptr);
	g_ws_thread_->Start();
}

ZXEngine::~ZXEngine()
{
	if (g_ws_thread_.get() != nullptr)
	{
		g_ws_thread_->Stop();
		g_ws_thread_.reset();
	}
}

void ZXEngine::OnMessage(rtc::Message * msg)
{
	if (msg->message_id == socket_disconnet_)
	{
		mStatus = 0;
		mZXClient.Stop();
	}
	if (msg->message_id == stream_add_)
	{
		std::string uid;
		if (!rtc::GetStringFromJsonObject(json_add_, "uid", &uid))
		{
			RTC_LOG(LS_ERROR) << "recv data mid error";
			OutputDebugStringA("recv data mid error");
			return;
		}

		std::string mid;
		if (!rtc::GetStringFromJsonObject(json_add_, "mid", &mid))
		{
			RTC_LOG(LS_ERROR) << "recv data mid error";
			OutputDebugStringA("recv data mid error");
			return;
		}

		std::string sfu;
		if (!rtc::GetStringFromJsonObject(json_add_, "sfuid", &sfu))
		{
			RTC_LOG(LS_ERROR) << "recv data sfu error";
			OutputDebugStringA("recv data mid error");
			return;
		}

		startSubscribe(uid, mid, sfu);
	}
	if (msg->message_id == stream_remove_)
	{
		std::string mid;
		if (!rtc::GetStringFromJsonObject(json_remove_, "mid", &mid))
		{
			RTC_LOG(LS_ERROR) << "recv data mid error";
			OutputDebugStringA("recv data mid error");
			return;
		}

		stopSubscribe(mid);
	}
}

void ZXEngine::setServerIp(std::string ip, uint16_t port)
{
	g_server_ip = ip;
	g_server_port = port;
}

void ZXEngine::setSdkListen(msg_callback observer)
{
	mListen = observer;
}

// 初始化sdk
bool ZXEngine::initSdk(std::string uid)
{
	if (uid == "")
	{
		return false;
	}

	strUid = uid;
	mLocalPeer.strUid = uid;
	return initPeerConnectionFactory();
}

// 释放sdk
void ZXEngine::freeSdk()
{
	freePeerConnectionFactory();
	strUid = "";
}

// 启动连接
bool ZXEngine::start()
{
	if (strUid == "" || peer_connection_factory_ == nullptr)
	{
		return false;
	}

	char cport[16];
	memset(cport, 0, sizeof(char) * 16);
	_itoa(g_server_port, cport, 10);
	std::string port_ = cport;
	std::string url = "ws://" + g_server_ip + ":" + port_ + "/ws?peer=" + strUid;
	strUrl = url;

	bool suc = mZXClient.Start(url);
	if (suc)
	{
		mStatus = 1;
		socket_close_ = false;
	}
	return suc;
}

// 停止连接
void ZXEngine::stop()
{
	mStatus = 0;
	socket_close_ = true;
	mZXClient.Stop();
}

// 返回连接状态
bool ZXEngine::getConnect()
{
	return mZXClient.GetConnect();
}

// 加入房间
bool ZXEngine::joinRoom(std::string rid)
{
	if (strUid == "" || peer_connection_factory_ == nullptr)
	{
		return false;
	}

	if (rid == "")
	{
		return false;
	}

	strRid = rid;
	room_close_ = false;
	if (mZXClient.SendJoin())
	{
		mStatus = 2;
		//room_close_ = false;
		// 启动线程
		StartWorkThread();
		StartHeatThread();
		return true;
	}
	return false;
}

// 离开房间
void ZXEngine::leaveRoom()
{
	mStatus = 1;
	room_close_ = true;
	StopHeatThread();
	StopWorkThread();
	stopPublish();
	freeAllRemotePeer();
	mZXClient.SendLeave();
	strRid = "";
}

// 设置麦克风
void ZXEngine::setMicrophoneMute(bool bMute)
{
	//mLocalPeer.SetAudioEnable(!bMute);
}

// 获取麦克风状态
bool ZXEngine::getMicrophoneMute()
{
	return false;
}

void ZXEngine::respSocketEvent()
{
	if (g_ws_thread_.get() != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		g_ws_thread_->Post(loc, this, socket_disconnet_);
	}

	/*
	Json::Value jRoot;
	jRoot["to"] = "audio";
	jRoot["type"] = "200";
	jRoot["data"] = "";

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	if (mListen != nullptr)
	{
		mListen((char*)(jsonStr.c_str()));
	}*/
}

void ZXEngine::respPeerJoin(Json::Value jsonObject)
{
	/*
	Json::Value jRoot;
	jRoot["to"] = "audio";
	jRoot["type"] = "201";
	jRoot["data"] = jsonObject;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	if (mListen != nullptr)
	{
		mListen((char*)(jsonStr.c_str()));
	}*/
}

void ZXEngine::respPeerLeave(Json::Value jsonObject)
{
	/*
	Json::Value jRoot;
	jRoot["to"] = "audio";
	jRoot["type"] = "202";
	jRoot["data"] = jsonObject;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	if (mListen != nullptr)
	{
		mListen((char*)(jsonStr.c_str()));
	}*/
}

void ZXEngine::respStreamAdd(Json::Value jsonObject)
{
	if (socket_close_ || room_close_)
	{
		return;
	}

	json_add_ = jsonObject;
	if (g_ws_thread_.get() != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		g_ws_thread_->Post(loc, this, stream_add_);
	}

	/*
	Json::Value jRoot;
	jRoot["to"] = "audio";
	jRoot["type"] = "203";
	jRoot["data"] = jsonObject;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	if (mListen != nullptr)
	{
		mListen((char*)(jsonStr.c_str()));
	}*/
}

void ZXEngine::respStreamRemove(Json::Value jsonObject)
{
	json_remove_ = jsonObject;
	if (g_ws_thread_.get() != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		g_ws_thread_->Post(loc, this, stream_remove_);
	}
	/*
	Json::Value jRoot;
	jRoot["to"] = "audio";
	jRoot["type"] = "204";
	jRoot["data"] = jsonObject;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	if (mListen != nullptr)
	{
		mListen((char*)(jsonStr.c_str()));
	}*/
}

void ZXEngine::respPeerKick(Json::Value jsonObject)
{
	/*
	Json::Value jRoot;
	jRoot["to"] = "audio";
	jRoot["type"] = "210";
	jRoot["data"] = jsonObject;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	if (mListen != nullptr)
	{
		mListen((char*)(jsonStr.c_str()));
	}*/
}

void ZXEngine::OnPeerPublish(bool bSuc, std::string error)
{
	Json::Value jsonObject;
	jsonObject["result"] = bSuc;
	jsonObject["msg"] = error;

	Json::Value jRoot;
	jRoot["to"] = "audio";
	jRoot["type"] = "220";
	jRoot["data"] = jsonObject;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	if (mListen != nullptr)
	{
		mListen((char*)(jsonStr.c_str()));
	}
}

void ZXEngine::OnPeerPublishError()
{
	Json::Value jRoot;
	jRoot["to"] = "audio";
	jRoot["type"] = "221";
	jRoot["data"] = "";

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	if (mListen != nullptr)
	{
		mListen((char*)(jsonStr.c_str()));
	}
}

void ZXEngine::OnPeerSubscribe(std::string mid, bool bSuc, std::string error)
{
	Json::Value jsonObject;
	jsonObject["mid"] = mid;
	jsonObject["result"] = bSuc;
	jsonObject["msg"] = error;

	Json::Value jRoot;
	jRoot["to"] = "audio";
	jRoot["type"] = "222";
	jRoot["data"] = jsonObject;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	if (mListen != nullptr)
	{
		mListen((char*)(jsonStr.c_str()));
	}
}

void ZXEngine::OnPeerSubscribeError(std::string mid)
{
	Json::Value jsonObject;
	jsonObject["mid"] = mid;

	Json::Value jRoot;
	jRoot["to"] = "audio";
	jRoot["type"] = "223";
	jRoot["data"] = jsonObject;

	std::string jsonStr;
	Json::StyledWriter writer;
	jsonStr = writer.write(jRoot);

	if (mListen != nullptr)
	{
		mListen((char*)(jsonStr.c_str()));
	}
}

bool ZXEngine::initPeerConnectionFactory()
{
	freePeerConnectionFactory();

	// rtc线程
	g_signaling_thread = rtc::Thread::Create();
	g_signaling_thread->SetName("signaling_thread", nullptr);
	g_signaling_thread->Start();

	RTC_LOG(LS_ERROR) << "peer_connection_factory_ create start";
	OutputDebugStringA("peer_connection_factory_ create start");
	peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
		nullptr /* network_thread */,
		nullptr /* worker_thread */,
		g_signaling_thread.get() /* signaling_thread */,
		nullptr /* default_adm */,
		webrtc::CreateBuiltinAudioEncoderFactory(),
		webrtc::CreateBuiltinAudioDecoderFactory(),
		webrtc::CreateBuiltinVideoEncoderFactory(),
		webrtc::CreateBuiltinVideoDecoderFactory(),
		nullptr /* audio_mixer */,
		nullptr /* audio_processing */);
	if (peer_connection_factory_ == nullptr)
	{
		RTC_LOG(LS_ERROR) << "peer_connection_factory_ = null";
		OutputDebugStringA("peer_connection_factory_ = null");
		freePeerConnectionFactory();
		return false;
	}
	RTC_LOG(LS_ERROR) << "peer_connection_factory_ create stop";
	OutputDebugStringA("peer_connection_factory_ create stop");
	return true;
}

void ZXEngine::freePeerConnectionFactory()
{
	if (peer_connection_factory_ != nullptr)
	{
		peer_connection_factory_ = nullptr;
	}
	if (g_signaling_thread.get() != nullptr)
	{
		g_signaling_thread->Stop();
		g_signaling_thread.reset();
	}
}

void ZXEngine::startPublish()
{
	mLocalPeer.StartPublish();
}

void ZXEngine::stopPublish()
{
	mLocalPeer.StopPublish();
}

void ZXEngine::startSubscribe(std::string uid, std::string mid, std::string sfu)
{
	mutex.lock();
	ZXPeerRemote* pRemote = vtRemotePeers[mid];
	if (pRemote == nullptr)
	{
		pRemote = new ZXPeerRemote();
		pRemote->strUid = uid;
		pRemote->strMid = mid;
		pRemote->strSfu = sfu;
		pRemote->pZXEngine = this;
		vtRemotePeers[mid] = pRemote;
	}
	pRemote->StartSubscribe();
	mutex.unlock();
}

// 停止拉流
void ZXEngine::stopSubscribe(std::string mid)
{
	mutex.lock();
	ZXPeerRemote* pRemote = vtRemotePeers[mid];
	if (pRemote != nullptr)
	{
		pRemote->StopSubscribe();
		delete pRemote;
		vtRemotePeers.erase(mid);
	}
	mutex.unlock();
}

void ZXEngine::freeAllRemotePeer()
{
	mutex.lock();
	std::map<std::string, ZXPeerRemote*>::iterator it;
	for (it = vtRemotePeers.begin(); it != vtRemotePeers.end(); it++)
	{
		ZXPeerRemote* pRemote = it->second;
		if (pRemote != nullptr)
		{
			pRemote->StopSubscribe();
			delete pRemote;
		}
		vtRemotePeers.erase(it);
	}
	mutex.unlock();
}

void ZXEngine::StartWorkThread()
{
	StopWorkThread();
	// 启动工作线程
	bWorkExit = false;
	hWorkThread = CreateThread(NULL, 0, WorkThreadFunc, this, 0, NULL);
}

void ZXEngine::StopWorkThread()
{
	if (hWorkThread != NULL)
	{
		bWorkExit = true;
		DWORD dwStart = GetTickCount();
		while (1)
		{
			if (GetTickCount() - dwStart > 5000)
			{
				TerminateThread(hWorkThread, 0);
				break;
			}
			if (WaitForSingleObject(hWorkThread, 1000) == WAIT_OBJECT_0)
			{
				break;
			}
		}
		CloseHandle(hWorkThread);
		hWorkThread = NULL;
	}
}

void ZXEngine::StartHeatThread()
{
	StopHeatThread();
	// 启动心跳线程
	bHeatExit = false;
	hHeatThread = CreateThread(NULL, 0, HeatThreadFunc, this, 0, NULL);
}

void ZXEngine::StopHeatThread()
{
	if (hHeatThread != NULL)
	{
		bHeatExit = true;
		DWORD dwStart = GetTickCount();
		while (1)
		{
			if (GetTickCount() - dwStart > 5000)
			{
				TerminateThread(hHeatThread, 0);
				break;
			}
			if (WaitForSingleObject(hHeatThread, 1000) == WAIT_OBJECT_0)
			{
				break;
			}
		}
		CloseHandle(hHeatThread);
		hHeatThread = NULL;
	}
}

DWORD WINAPI ZXEngine::WorkThreadFunc(LPVOID data)
{
	ZXEngine *pZXEngine = (ZXEngine*)data;
	RTC_LOG(LS_INFO) << "WorkThreadFunc start";
	OutputDebugStringA("WorkThreadFunc start");
	while (!pZXEngine->bWorkExit)
	{
		if (pZXEngine->socket_close_ || pZXEngine->room_close_ || pZXEngine->bWorkExit)
		{
			RTC_LOG(LS_INFO) << "WorkThreadFunc stop1";
			OutputDebugStringA("WorkThreadFunc stop1");
			return 0;
		}

		if (pZXEngine->mStatus == 2)
		{
			// 判断推流
			if (pZXEngine->mLocalPeer.nLive == 0)
			{
				pZXEngine->mLocalPeer.StartPublish();
			}

			if (pZXEngine->socket_close_ || pZXEngine->room_close_ || pZXEngine->bWorkExit)
			{
				RTC_LOG(LS_INFO) << "WorkThreadFunc stop2";
				OutputDebugStringA("WorkThreadFunc stop2");
				return 0;
			}

			// 判断拉流
			pZXEngine->mutex.lock();
			std::map<std::string, ZXPeerRemote*>::iterator it;
			for (it = pZXEngine->vtRemotePeers.begin(); it != pZXEngine->vtRemotePeers.end(); it++)
			{
				ZXPeerRemote* pRemote = it->second;
				if (pRemote != nullptr)
				{
					if (pRemote->nLive == 0)
					{
						pRemote->StartSubscribe();
					}
				}
				
				if (pZXEngine->socket_close_ || pZXEngine->room_close_ || pZXEngine->bWorkExit)
				{
					pZXEngine->mutex.unlock();
					RTC_LOG(LS_INFO) << "WorkThreadFunc stop3";
					OutputDebugStringA("WorkThreadFunc stop3");
					return 0;
				}
			}
			pZXEngine->mutex.unlock();
		}
		else
		{
			// 停止推流
			pZXEngine->stopPublish();

			if (pZXEngine->socket_close_ || pZXEngine->room_close_ || pZXEngine->bWorkExit)
			{
				pZXEngine->mutex.unlock();
				RTC_LOG(LS_INFO) << "WorkThreadFunc stop4";
				OutputDebugStringA("WorkThreadFunc stop4");
				return 0;
			}

			// 停止拉流
			pZXEngine->mutex.lock();
			std::map<std::string, ZXPeerRemote*>::iterator it;
			for (it = pZXEngine->vtRemotePeers.begin(); it != pZXEngine->vtRemotePeers.end(); it++)
			{
				ZXPeerRemote* pRemote = it->second;
				if (pRemote != nullptr)
				{
					pRemote->StopSubscribe();
					delete pRemote;
				}
				pZXEngine->vtRemotePeers.erase(it);

				if (pZXEngine->socket_close_ || pZXEngine->room_close_ || pZXEngine->bWorkExit)
				{
					pZXEngine->mutex.unlock();
					RTC_LOG(LS_INFO) << "WorkThreadFunc stop5";
					OutputDebugStringA("WorkThreadFunc stop5");
					return 0;
				}
			}
			pZXEngine->mutex.unlock();
		}

		// 延时
		for (int i = 0; i < 20; i++)
		{
			if (pZXEngine->socket_close_ || pZXEngine->room_close_ || pZXEngine->bWorkExit)
			{
				RTC_LOG(LS_INFO) << "WorkThreadFunc stop6";
				OutputDebugStringA("WorkThreadFunc stop6");
				return 0;
			}
			Sleep(100);
		}
	}
	RTC_LOG(LS_INFO) << "WorkThreadFunc stop";
	OutputDebugStringA("WorkThreadFunc stop");
	return 0;
}

DWORD WINAPI ZXEngine::HeatThreadFunc(LPVOID data)
{
	ZXEngine *pZXEngine = (ZXEngine*)data;
	RTC_LOG(LS_INFO) << "HeatThreadFunc start";
	OutputDebugStringA("HeatThreadFunc start");
	int nCount = 1;
	while (!pZXEngine->bHeatExit)
	{
		if (pZXEngine->socket_close_ || pZXEngine->room_close_ || pZXEngine->bHeatExit)
		{
			RTC_LOG(LS_INFO) << "HeatThreadFunc stop1";
			OutputDebugStringA("HeatThreadFunc stop1");
			return 0;
		}

		if (pZXEngine->mStatus == 0)
		{
			nCount = 1;
			if (pZXEngine->mZXClient.Start(pZXEngine->strUrl))
			{
				pZXEngine->mStatus = 1;
			}

			if (pZXEngine->socket_close_ || pZXEngine->room_close_ || pZXEngine->bHeatExit)
			{
				RTC_LOG(LS_INFO) << "HeatThreadFunc stop2";
				OutputDebugStringA("HeatThreadFunc stop2");
				return 0;
			}
		}
		else if (pZXEngine->mStatus == 1)
		{
			nCount = 10;
			if (pZXEngine->mZXClient.SendJoin())
			{
				nCount = 200;
				pZXEngine->mStatus = 2;
			}

			if (pZXEngine->socket_close_ || pZXEngine->room_close_ || pZXEngine->bHeatExit)
			{
				RTC_LOG(LS_INFO) << "HeatThreadFunc stop3";
				OutputDebugStringA("HeatThreadFunc stop3");
				return 0;
			}
		}
		else if (pZXEngine->mStatus == 2)
		{
			nCount = 200;
			static int nTmp = 0;
			bool bSuc = pZXEngine->mZXClient.SendAlive();
			if (bSuc)
			{
				nTmp = 0;
			}
			else
			{
				nTmp++;
			}
			if (nTmp >= 2)
			{
				nCount = 1;
				pZXEngine->mStatus = 0;
				// 停止socket
				pZXEngine->mZXClient.Stop();
			}
		}
		// 延时
		for (int i = 0; i < nCount; i++)
		{
			if (pZXEngine->socket_close_ || pZXEngine->room_close_ || pZXEngine->bHeatExit)
			{
				RTC_LOG(LS_INFO) << "HeatThreadFunc stop4";
				OutputDebugStringA("HeatThreadFunc stop4");
				return 0;
			}
			Sleep(100);
		}
	}
	RTC_LOG(LS_INFO) << "HeatThreadFunc stop";
	OutputDebugStringA("HeatThreadFunc stop");
	return 0;
}
