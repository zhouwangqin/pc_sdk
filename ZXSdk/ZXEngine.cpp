#include "pch.h"
#include "ZXBase.h"
#include "ZXEngine.h"
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

	bPublish = true;
	bScreen = false;
	bScreenPub = true;

	mStatus = 0;
	bRoomClose = false;

	bWorkExit = false;
	bHeatExit = false;
	hWorkThread = nullptr;
	hHeatThread = nullptr;

	log_callback_ = nullptr;
	local_callback_ = nullptr;
	remote_callback_ = nullptr;

	mZXClient.pZXEngine = this;
	mLocalPeer.pZXEngine = this;
	mScreenPeer.pZXEngine = this;

	vtRemotePeers.clear();
	peer_connection_factory_ = nullptr;

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

void ZXEngine::setLogDebug(log_callback callback)
{
	log_callback_ = callback;
}

void ZXEngine::setServerIp(std::string ip, uint16_t port)
{
	g_server_ip = ip;
	g_server_port = port;
}

void ZXEngine::setLocalVideo(video_frame_callback callback)
{
	local_callback_ = callback;
}

void ZXEngine::setRemoteVideo(video_frame_callback callback)
{
	remote_callback_ = callback;
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
	mScreenPeer.strUid = uid;
	writeLog("start initSdk");
	return initPeerConnectionFactory();
}

// 释放sdk
void ZXEngine::freeSdk()
{
	if (strUid == "")
	{
		return;
	}

	writeLog("start freeSdk");
	freePeerConnectionFactory();
	strUid = "";
	writeLog("start freeSdk ok");
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
	char cport[16];
	memset(cport, 0, sizeof(char) * 16);
	_itoa(g_server_port, cport, 10);
	std::string port_ = cport;
	strUrl = "ws://" + g_server_ip + ":" + port_ + "/ws?peer=" + strUid;

	bRoomClose = false;
	writeLog("start socket");
	if (mZXClient.Start(strUrl))
	{
		writeLog("start socket ok");
		writeLog("start join room");
		if (mZXClient.SendJoin())
		{
			writeLog("start join room ok");
			mStatus = 1;
			// 启动线程
			StartWorkThread();
			StartHeatThread();
			return true;
		}
		else
		{
			writeLog("start join room fail");
			mStatus = 0;
			mZXClient.Stop();
		}
	}
	else
	{
		writeLog("start socket fail");
		mStatus = 0;
		mZXClient.Stop();
	}
	return false;
}

// 离开房间
void ZXEngine::leaveRoom()
{
	if (strRid == "")
	{
		return;
	}

	if (bRoomClose)
	{
		return;
	}

	mStatus = 0;
	bRoomClose = true;
	writeLog("stop thread");
	StopHeatThread();
	StopWorkThread();
	writeLog("stop all remote peer");
	freeAllRemotePeer();
	writeLog("stop screen peer");
	stopScreen();
	writeLog("stop audio peer");
	stopPublish();
	writeLog("leave room");
	mZXClient.SendLeave();
	writeLog("close socket");
	mZXClient.Stop();
	strRid = "";
}

void ZXEngine::setPublish(bool bPub)
{
	bPublish = bPub;
}

void ZXEngine::setScreen(bool bPub)
{
	bScreen = bPub;
}

void ZXEngine::setScreenPub(bool bPub)
{
	if (bScreenPub == bPub)
	{
		return;
	}
	bScreenPub = bPub;
	mScreenPeer.StopPublish();
}

void ZXEngine::setFrameRate(int nFrameRate)
{
	mScreenPeer.nCapFrameRate = nFrameRate;
}

// 设置麦克风
void ZXEngine::setMicrophoneMute(bool bMute)
{
	mLocalPeer.SetAudioEnable(!bMute);
}

// 获取麦克风状态
bool ZXEngine::getMicrophoneMute()
{
	return !mLocalPeer.GetAudioEnable();
}

void ZXEngine::respSocketEvent()
{
	if (g_ws_thread_.get() != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		g_ws_thread_->Post(loc, this, socket_disconnet_);
	}
}

void ZXEngine::respPeerJoin(Json::Value jsonObject)
{
	
}

void ZXEngine::respPeerLeave(Json::Value jsonObject)
{
	
}

void ZXEngine::respStreamAdd(Json::Value jsonObject)
{
	if (bRoomClose)
	{
		return;
	}

	std::string uid;
	if (!rtc::GetStringFromJsonObject(jsonObject, "uid", &uid))
	{
		writeLog("recv stream add uid error");
		return;
	}

	std::string mid;
	if (!rtc::GetStringFromJsonObject(jsonObject, "mid", &mid))
	{
		writeLog("recv stream add mid error");
		return;
	}

	std::string sfu;
	if (!rtc::GetStringFromJsonObject(jsonObject, "sfuid", &sfu))
	{
		writeLog("recv stream add sfu error");
		return;
	}

	startSubscribe(uid, mid, sfu);
}

void ZXEngine::respStreamRemove(Json::Value jsonObject)
{
	std::string mid;
	if (!rtc::GetStringFromJsonObject(jsonObject, "mid", &mid))
	{
		writeLog("recv stream remove mid error");
		return;
	}

	stopSubscribe(mid);
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

void ZXEngine::writeLog(std::string msg)
{
	RTC_LOG(LS_ERROR) << msg;
	if (log_callback_ != nullptr)
	{
		log_callback_(msg.c_str());
	}
}

void ZXEngine::OnMessage(rtc::Message * msg)
{
	if (msg->message_id == socket_disconnet_)
	{
		mStatus = 0;
		mZXClient.Stop();
	}
}

bool ZXEngine::initPeerConnectionFactory()
{
	freePeerConnectionFactory();

	// rtc线程
	g_signaling_thread = rtc::Thread::Create();
	g_signaling_thread->SetName("signaling_thread", nullptr);
	g_signaling_thread->Start();

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
		writeLog("peer_connection_factory_ = null");
		freePeerConnectionFactory();
		return false;
	}
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

void ZXEngine::stopPublish()
{
	mLocalPeer.StopPublish();
}

void ZXEngine::stopScreen()
{
	mScreenPeer.StopPublish();
}

void ZXEngine::startSubscribe(std::string uid, std::string mid, std::string sfu)
{
	std::lock_guard<std::mutex> lock(mutex);
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
	//pRemote->StartSubscribe();
}

// 停止拉流
void ZXEngine::stopSubscribe(std::string mid)
{
	std::lock_guard<std::mutex> lock(mutex);
	ZXPeerRemote* pRemote = vtRemotePeers[mid];
	if (pRemote != nullptr)
	{
		pRemote->StopSubscribe();
		delete pRemote;
		vtRemotePeers.erase(mid);
	}
}

void ZXEngine::freeAllRemotePeer()
{
	std::lock_guard<std::mutex> lock(mutex);
	std::map<std::string, ZXPeerRemote*>::iterator it;
	for (it = vtRemotePeers.begin(); it != vtRemotePeers.end(); it++)
	{
		ZXPeerRemote* pRemote = it->second;
		if (pRemote != nullptr)
		{
			pRemote->StopSubscribe();
			delete pRemote;
		}
	}
	vtRemotePeers.clear();
}

void ZXEngine::StartWorkThread()
{
	StopWorkThread();
	// 启动工作线程
	bWorkExit = false;
	hWorkThread = CreateThread(nullptr, 0, WorkThreadFunc, this, 0, nullptr);
}

void ZXEngine::StopWorkThread()
{
	if (hWorkThread != nullptr)
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
		hWorkThread = nullptr;
	}
}

void ZXEngine::StartHeatThread()
{
	StopHeatThread();
	// 启动心跳线程
	bHeatExit = false;
	hHeatThread = CreateThread(nullptr, 0, HeatThreadFunc, this, 0, nullptr);
}

void ZXEngine::StopHeatThread()
{
	if (hHeatThread != nullptr)
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
		hHeatThread = nullptr;
	}
}

DWORD WINAPI ZXEngine::WorkThreadFunc(LPVOID data)
{
	writeLog("WorkThreadFunc start");
	ZXEngine *pZXEngine = (ZXEngine*)data;
	while (!pZXEngine->bWorkExit)
	{
		if (pZXEngine->bRoomClose || pZXEngine->bWorkExit)
		{
			writeLog("WorkThreadFunc stop1");
			return 0;
		}

		if (pZXEngine->mStatus == 1)
		{
			// 判断推流
			if (pZXEngine->bPublish)
			{
				if (pZXEngine->mLocalPeer.nLive == 0)
				{
					writeLog("start audio pub");
					pZXEngine->mLocalPeer.StartPublish();
				}
			}
			else
			{
				pZXEngine->mLocalPeer.StopPublish();
			}
			
			if (pZXEngine->bRoomClose || pZXEngine->bWorkExit)
			{
				writeLog("WorkThreadFunc stop2");
				return 0;
			}

			// 判断屏幕共享
			if (pZXEngine->bScreen)
			{
				if (pZXEngine->mScreenPeer.nLive == 0)
				{
					writeLog("start screen pub");
					pZXEngine->mScreenPeer.StartPublish();
				}
			}
			else
			{
				pZXEngine->mScreenPeer.StopPublish();
			}

			if (pZXEngine->bRoomClose || pZXEngine->bWorkExit)
			{
				writeLog("WorkThreadFunc stop3");
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
						writeLog("start remote sub = " + pRemote->strMid);
						pRemote->StartSubscribe();
					}
				}
				
				if (pZXEngine->bRoomClose || pZXEngine->bWorkExit)
				{
					pZXEngine->mutex.unlock();
					writeLog("WorkThreadFunc stop4");
					return 0;
				}
			}
			pZXEngine->mutex.unlock();
		}
		else
		{
			// 停止推流
			pZXEngine->stopPublish();

			if (pZXEngine->bRoomClose || pZXEngine->bWorkExit)
			{
				writeLog("WorkThreadFunc stop5");
				return 0;
			}

			// 停止屏幕共享
			pZXEngine->stopScreen();

			if (pZXEngine->bRoomClose || pZXEngine->bWorkExit)
			{
				writeLog("WorkThreadFunc stop6");
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

				if (pZXEngine->bRoomClose || pZXEngine->bWorkExit)
				{
					pZXEngine->mutex.unlock();
					writeLog("WorkThreadFunc stop7");
					return 0;
				}
			}
			pZXEngine->vtRemotePeers.clear();
			pZXEngine->mutex.unlock();
		}

		// 延时
		for (int i = 0; i < 10; i++)
		{
			if (pZXEngine->bRoomClose || pZXEngine->bWorkExit)
			{
				writeLog("WorkThreadFunc stop8");
				return 0;
			}
			Sleep(100);
		}
	}
	writeLog("WorkThreadFunc stop");
	return 0;
}

DWORD WINAPI ZXEngine::HeatThreadFunc(LPVOID data)
{
	writeLog("HeatThreadFunc start");
	int nCount = 1;
	ZXEngine *pZXEngine = (ZXEngine*)data;
	while (!pZXEngine->bHeatExit)
	{
		if (pZXEngine->bRoomClose || pZXEngine->bHeatExit)
		{
			writeLog("HeatThreadFunc stop1");
			return 0;
		}

		if (pZXEngine->mStatus == 0)
		{
			nCount = 1;
			writeLog("restart socket");
			if (pZXEngine->mZXClient.Start(pZXEngine->strUrl))
			{
				writeLog("restart socket ok");

				if (pZXEngine->bRoomClose || pZXEngine->bHeatExit)
				{
					writeLog("HeatThreadFunc stop2");
					return 0;
				}

				writeLog("restart join room");

				nCount = 10;
				if (pZXEngine->mZXClient.SendJoin())
				{
					writeLog("restart join room ok");
					nCount = 200;
					pZXEngine->mStatus = 1;
				}
				else
				{
					writeLog("restart join room fail");
					pZXEngine->mZXClient.Stop();
				}
			}
			else
			{
				writeLog("restart socket fail");
				pZXEngine->mZXClient.Stop();
			}

			if (pZXEngine->bRoomClose || pZXEngine->bHeatExit)
			{
				writeLog("HeatThreadFunc stop3");
				return 0;
			}
		}
		else if (pZXEngine->mStatus == 1)
		{
			writeLog("start send alive");
			nCount = 200;
			pZXEngine->mZXClient.SendAlive();

			if (pZXEngine->bRoomClose || pZXEngine->bHeatExit)
			{
				writeLog("HeatThreadFunc stop4");
				return 0;
			}
		}
		
		// 延时
		for (int i = 0; i < nCount; i++)
		{
			if (pZXEngine->bRoomClose || pZXEngine->bHeatExit)
			{
				writeLog("HeatThreadFunc stop5");
				return 0;
			}
			Sleep(100);
		}
	}
	writeLog("HeatThreadFunc stop");
	return 0;
}
