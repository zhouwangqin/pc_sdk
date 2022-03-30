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
	if (msg->message_id == 2000)
	{
		static int nCount = 0;
		bool bSuc = mZXClient.SendAlive();
		if (bSuc)
		{
			nCount = 0;
		}
		else
		{
			nCount++;
		}
		if (nCount >= 2)
		{
			respSocketEvent();
			return;
		}
		if (g_ws_thread_ != nullptr)
		{
			rtc::Location loc(__FUNCTION__, __FILE__);
			g_ws_thread_->PostDelayed(loc, 20000, this, 2000);
		}
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
	freeLocalPeer();
	freeAllRemotePeer();
	freePeerConnectionFactory();
}

// 启动连接
bool ZXEngine::start()
{
	if (peer_connection_factory_ == nullptr)
	{
		return false;
	}

	char cport[16];
	memset(cport, 0, sizeof(char) * 16);
	_itoa(g_server_port, cport, 10);
	std::string port_ = cport;
	std::string url = "ws://" + g_server_ip + ":" + port_ + "/ws?peer=" + strUid;
	strUrl = url;
	return mZXClient.Start();
}

// 停止连接
void ZXEngine::stop()
{
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
	if (peer_connection_factory_ == nullptr)
	{
		return false;
	}

	if (rid == "")
	{
		return false;
	}

	strRid = rid;
	if (mZXClient.SendJoin())
	{
		if (g_ws_thread_.get() != nullptr)
		{
			rtc::Location loc(__FUNCTION__, __FILE__);
			g_ws_thread_->PostDelayed(loc, 20000, this, 2000);
		}
		return true;
	}
	return false;
}

// 离开房间
void ZXEngine::leaveRoom()
{
	if (g_ws_thread_.get() != nullptr)
	{
		g_ws_thread_->Clear(this, 2000);
	}
	mZXClient.SendLeave();
}

// 启动推流
void ZXEngine::startPublish()
{
	mLocalPeer.StartPublish();
}

// 停止推流
void ZXEngine::stopPublish()
{
	if (!mLocalPeer.bClose)
	{
		mLocalPeer.StopPublish();
	}
}

// 启动拉流
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
		if (!pRemote->bClose)
		{
			pRemote->StopSubscribe();
		}
		delete pRemote;
		vtRemotePeers.erase(mid);
	}
	mutex.unlock();
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
	}
}

void ZXEngine::respPeerJoin(Json::Value jsonObject)
{
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
	}
}

void ZXEngine::respPeerLeave(Json::Value jsonObject)
{
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
	}
}

void ZXEngine::respStreamAdd(Json::Value jsonObject)
{
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
	}
}

void ZXEngine::respStreamRemove(Json::Value jsonObject)
{
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
	}
}

void ZXEngine::respPeerKick(Json::Value jsonObject)
{
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
	}
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
		freePeerConnectionFactory();
		return false;
	}
	RTC_LOG(LS_ERROR) << "peer_connection_factory_ create stop";
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

void ZXEngine::freeLocalPeer()
{
	if (!mLocalPeer.bClose)
	{
		mLocalPeer.StopPublish();
	}
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
			if (!pRemote->bClose)
			{
				pRemote->StopSubscribe();
			}
			delete pRemote;
		}
		vtRemotePeers.erase(it);
	}
	mutex.unlock();
}
