#include "pch.h"
#include "ZXBase.h"
#include "ZXEngine.h"
#include "ZXPeerRemote.h"

ZXPeerRemote::ZXPeerRemote()
{
	strUid = "";
	strMid = "";
	strSfu = "";
	strSid = "";

	nLive = 0;
	bClose = false;
	pZXEngine = nullptr;

	sdp_ = "";
	peer_connection_ = nullptr;
	remote_video_observer_.pZXPeerRemote = this;

	pOfferCreateSdpObserver = OfferCreateSessionDescriptionObserver::Create();
	pOfferCreateSdpObserver->pZXPeerRemote = this;
	pOfferSetSdpObserver = OfferSetSessionDescriptionObserver::Create();
	pOfferSetSdpObserver->pZXPeerRemote = this;
	pAnswerSetSdpObserver = AnswerSetSessionDescriptionObserver::Create();
	pAnswerSetSdpObserver->pZXPeerRemote = this;
}

ZXPeerRemote::~ZXPeerRemote()
{
	pOfferCreateSdpObserver = nullptr;
	pOfferSetSdpObserver = nullptr;
	pAnswerSetSdpObserver = nullptr;
}

void ZXPeerRemote::OnMessage(rtc::Message * msg)
{
	if (msg->message_id == set_offer_sdp_ok)
	{
		SendSubscribe(sdp_);
	}
}

void ZXPeerRemote::OnData(const void * audio_data, int bits_per_sample, int sample_rate, size_t number_of_channels, size_t number_of_frames)
{
	//ZXEngine::writeLog("remote peer audio callback = ");
}

void ZXPeerRemote::StartSubscribe()
{
	InitPeerConnection();
	CreateOffer();
}

void ZXPeerRemote::StopSubscribe()
{
	SendUnSubscribe();
	FreePeerConnection();
}

void ZXPeerRemote::CreateOffer()
{
	if (peer_connection_ != nullptr)
	{
		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		options.offer_to_receive_audio = true;
		options.offer_to_receive_video = true;
		peer_connection_->CreateOffer(pOfferCreateSdpObserver, options);
	}
}

void ZXPeerRemote::CreateSdpSuc(webrtc::SessionDescriptionInterface * sdp)
{
	std::string msg = "remote peer create offer sdp ok = " + strMid;
	ZXEngine::writeLog(msg);
	if (bClose)
	{
		return;
	}

	sdp->ToString(&sdp_);
	SetLocalDescription(sdp);
}

void ZXPeerRemote::CreateSdpFail(std::string error)
{
	std::string msg = "remote peer create offer sdp fail mid = " + strMid + ", err = " + error;
	ZXEngine::writeLog(msg);

	nLive = 0;
}

void ZXPeerRemote::SetLocalDescription(webrtc::SessionDescriptionInterface* sdp)
{
	if (peer_connection_ != nullptr)
	{
		peer_connection_->SetLocalDescription(pOfferSetSdpObserver, sdp);
	}
}

void ZXPeerRemote::OnSetLocalSdpSuc()
{
	std::string msg = "remote peer set offer sdp ok = " + strMid;
	ZXEngine::writeLog(msg);
	if (bClose)
	{
		return;
	}

	if (pZXEngine != nullptr && pZXEngine->g_ws_thread_.get() != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		pZXEngine->g_ws_thread_->Post(loc, this, set_offer_sdp_ok);
	}
}

void ZXPeerRemote::OnSetLoaclSdpFail(std::string error)
{
	std::string msg = "remote peer set offer sdp fail mid = " + strMid + ", err = " + error;
	ZXEngine::writeLog(msg);

	nLive = 0;
}

void ZXPeerRemote::SetRemoteDescription(webrtc::SessionDescriptionInterface* sdp)
{
	if (peer_connection_ != nullptr)
	{
		peer_connection_->SetRemoteDescription(pAnswerSetSdpObserver, sdp);
	}
}

void ZXPeerRemote::OnSetRemoteSdpSuc()
{
	std::string msg = "remote peer set answer sdp ok = " + strMid;
	ZXEngine::writeLog(msg);
	if (bClose)
	{
		return;
	}

	nLive = 3;
}

void ZXPeerRemote::OnSetRemoteSdpFail(std::string error)
{
	std::string msg = "remote peer set answer sdp fail mid = " + strMid + ", err = " + error;
	ZXEngine::writeLog(msg);

	nLive = 0;
}

bool ZXPeerRemote::InitPeerConnection()
{
	FreePeerConnection();
	// 配置参数
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
	config.enable_dtls_srtp = true;
	config.disable_ipv6 = true;
	config.set_cpu_adaptation(true);
	config.bundle_policy = webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle;
	config.tcp_candidate_policy = webrtc::PeerConnectionInterface::kTcpCandidatePolicyDisabled;
	config.continual_gathering_policy = webrtc::PeerConnectionInterface::GATHER_CONTINUALLY;
	// 配置ICE
	webrtc::PeerConnectionInterface::IceServer stunServer;
	stunServer.uri = "stun:" + g_relay_server_ip;
	config.servers.push_back(stunServer);
	webrtc::PeerConnectionInterface::IceServer turnServer1;
	turnServer1.uri = "turn:" + g_relay_server_ip + "?transport=udp";
	turnServer1.username = "demo";
	turnServer1.password = "123456";
	config.servers.push_back(turnServer1);
	webrtc::PeerConnectionInterface::IceServer turnServer2;
	turnServer2.uri = "turn:" + g_relay_server_ip + "?transport=tcp";
	turnServer2.username = "demo";
	turnServer2.password = "123456";
	config.servers.push_back(turnServer2);
	// 创建peer
	if (pZXEngine != nullptr && pZXEngine->peer_connection_factory_ != nullptr)
	{
		std::string msg = "remote peer create start = " + strMid;
		ZXEngine::writeLog(msg);

		peer_connection_ = pZXEngine->peer_connection_factory_->CreatePeerConnection(config, nullptr, nullptr, this);
		if (peer_connection_ != nullptr)
		{
			peer_connection_->AddTransceiver(cricket::MediaType::MEDIA_TYPE_AUDIO);
			peer_connection_->AddTransceiver(cricket::MediaType::MEDIA_TYPE_VIDEO);

			nLive = 1;
			bClose = false;

			std::string msg = "remote peer create stop = " + strMid;
			ZXEngine::writeLog(msg);
			return true;
		}
		msg = "remote peer create stop1 = " + strMid;
		ZXEngine::writeLog(msg);
	}
	return false;
}

void ZXPeerRemote::FreePeerConnection()
{
	if (bClose)
	{
		return;
	}

	nLive = 0;
	bClose = true;
	sdp_ = "";
	std::string msg = "remote peer free start = " + strMid;
	ZXEngine::writeLog(msg);
	if (peer_connection_ != nullptr)
	{
		peer_connection_->Close();
		peer_connection_ = nullptr;
	}
	msg = "remote peer free stop = " + strMid;
	ZXEngine::writeLog(msg);
}

void ZXPeerRemote::SendSubscribe(std::string sdp)
{
	if (bClose) 
	{
		return;
	}

	if (pZXEngine != nullptr && sdp != "")
	{
		if (pZXEngine->mZXClient.SendSubscribe(sdp, strMid, strSfu)) 
		{
			nLive = 2;
			// 处理返回
			strSid = pZXEngine->mZXClient.strSid;
			std::string answer = pZXEngine->mZXClient.strSdp;
			std::unique_ptr<webrtc::SessionDescriptionInterface> pAnswer = webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, answer);
			SetRemoteDescription(pAnswer.release());
			return;
		}
	}

	if (bClose) 
	{
		return;
	}

	nLive = 0;
}

void ZXPeerRemote::SendUnSubscribe()
{
	if (strSid == "")
	{
		return;
	}

	if (pZXEngine != nullptr)
	{
		pZXEngine->mZXClient.SendUnSubscribe(strMid, strSid, strSfu);
	}

	strSid = "";
}

void ZXPeerRemote::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
	if (!stream->GetVideoTracks().empty())
	{
		stream->GetVideoTracks()[0]->AddOrUpdateSink(&remote_video_observer_, rtc::VideoSinkWants());
	}
	if (!stream->GetAudioTracks().empty())
	{
		stream->GetAudioTracks()[0]->AddSink(this);
	}
}

void ZXPeerRemote::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state)
{
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kConnected)
	{
		std::string msg = "remote peer kConnected = " + strMid;
		ZXEngine::writeLog(msg);
		nLive = 4;
	}
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kDisconnected)
	{
		std::string msg = "remote peer kDisconnected = " + strMid;
		ZXEngine::writeLog(msg);
		nLive = 0;
	}
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kFailed)
	{
		std::string msg = "remote peer kFailed = " + strMid;
		ZXEngine::writeLog(msg);
		nLive = 0;
	}
}

