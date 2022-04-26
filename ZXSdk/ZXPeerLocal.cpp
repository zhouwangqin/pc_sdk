#include "pch.h"
#include "ZXBase.h"
#include "ZXEngine.h"
#include "ZXPeerLocal.h"

ZXPeerLocal::ZXPeerLocal()
{
	strUid = "";
	strMid = "";
	strSfu = "";

	nLive = 0;
	bClose = false;
	pZXEngine = nullptr;

	sdp_ = "";
	audio_track_ = nullptr;
	peer_connection_ = nullptr;

	pOfferCreateSdpObserver = OfferCreateSessionDescriptionObserver::Create();
	pOfferCreateSdpObserver->pZXPeerLocal = this;
	pOfferSetSdpObserver = OfferSetSessionDescriptionObserver::Create();
	pOfferSetSdpObserver->pZXPeerLocal = this;
	pAnswerSetSdpObserver = AnswerSetSessionDescriptionObserver::Create();
	pAnswerSetSdpObserver->pZXPeerLocal = this;
}

ZXPeerLocal::~ZXPeerLocal()
{
	pOfferCreateSdpObserver = nullptr;
	pOfferSetSdpObserver = nullptr;
	pAnswerSetSdpObserver = nullptr;
}

void ZXPeerLocal::OnMessage(rtc::Message * msg)
{
	if (msg->message_id == set_offer_sdp_ok)
	{
		SendPublish(sdp_);
	}
}

void ZXPeerLocal::OnData(const void * audio_data, int bits_per_sample, int sample_rate, size_t number_of_channels, size_t number_of_frames)
{
	ZXEngine::writeLog("local peer audio callback");
}

void ZXPeerLocal::StartPublish()
{
	InitPeerConnection();
	CreateOffer();
}

void ZXPeerLocal::StopPublish()
{
	SendUnPublish();
	FreePeerConnection();
}

void ZXPeerLocal::SetAudioEnable(bool bEnable)
{
	if (audio_track_ != nullptr)
	{
		audio_track_->set_enabled(bEnable);
	}
}

bool ZXPeerLocal::GetAudioEnable()
{
	if (audio_track_ != nullptr)
	{
		return audio_track_->enabled();
	}
	return false;
}

void ZXPeerLocal::CreateOffer()
{
	if (peer_connection_ != nullptr)
	{
		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		options.offer_to_receive_audio = false;
		options.offer_to_receive_video = false;
		peer_connection_->CreateOffer(pOfferCreateSdpObserver, options);
	}
}

void ZXPeerLocal::CreateSdpSuc(webrtc::SessionDescriptionInterface* sdp)
{
	ZXEngine::writeLog("local peer create offer sdp ok");
	if (bClose)
	{
		return;
	}
	
	sdp->ToString(&sdp_);
	SetLocalDescription(sdp);
}

void ZXPeerLocal::CreateSdpFail(std::string error)
{
	std::string msg = "local peer create offer sdp fail = " + error;
	ZXEngine::writeLog(msg);

	nLive = 0;
}

void ZXPeerLocal::SetLocalDescription(webrtc::SessionDescriptionInterface* sdp)
{
	if (peer_connection_ != nullptr)
	{
		peer_connection_->SetLocalDescription(pOfferSetSdpObserver, sdp);
	}
}

void ZXPeerLocal::OnSetLocalSdpSuc()
{
	ZXEngine::writeLog("local peer set offer sdp ok");
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

void ZXPeerLocal::OnSetLoaclSdpFail(std::string error)
{
	std::string msg = "local peer set offer sdp fail = " + error;
	ZXEngine::writeLog(msg);

	nLive = 0;
}

void ZXPeerLocal::SetRemoteDescription(webrtc::SessionDescriptionInterface* sdp)
{
	if (peer_connection_ != nullptr)
	{
		peer_connection_->SetRemoteDescription(pAnswerSetSdpObserver, sdp);
	}
}

void ZXPeerLocal::OnSetRemoteSdpSuc()
{
	ZXEngine::writeLog("local peer set answer sdp ok");
	if (bClose)
	{
		return;
	}

	nLive = 3;
}

void ZXPeerLocal::OnSetRemoteSdpFail(std::string error)
{
	std::string msg = "local peer set answer sdp fail = " + error;
	ZXEngine::writeLog(msg);

	nLive = 0;
}

bool ZXPeerLocal::InitPeerConnection()
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
		ZXEngine::writeLog("local peer create start");
		peer_connection_ = pZXEngine->peer_connection_factory_->CreatePeerConnection(config, NULL, NULL, this);
		if (peer_connection_ != nullptr)
		{
			cricket::AudioOptions options;
			options.echo_cancellation = false;
			rtc::scoped_refptr<webrtc::AudioSourceInterface> audio_source_ = pZXEngine->peer_connection_factory_->CreateAudioSource(options);
			if (audio_source_ != nullptr)
			{
				audio_track_ = pZXEngine->peer_connection_factory_->CreateAudioTrack(kAudioLabel, audio_source_);
				if (audio_track_ != nullptr)
				{
					peer_connection_->AddTrack(audio_track_, { kStreamId });
					audio_track_->AddSink(this);
				}
			}

			nLive = 1;
			bClose = false;
			ZXEngine::writeLog("local peer create stop");
			return true;
		}
		ZXEngine::writeLog("local peer create stop1");
	}
	return false;
}

void ZXPeerLocal::FreePeerConnection()
{
	if (bClose)
	{
		return;
	}

	nLive = 0;
	bClose = true;
	sdp_ = "";
	audio_track_ = nullptr;
	ZXEngine::writeLog("local peer free start");
	if (peer_connection_ != nullptr)
	{
		peer_connection_->Close();
		peer_connection_ = nullptr;
	}
	ZXEngine::writeLog("local peer free stop");
}

void ZXPeerLocal::SendPublish(std::string sdp)
{
	if (bClose) 
	{
		return;
	}

	if (pZXEngine != nullptr && sdp != "")
	{
		if (pZXEngine->mZXClient.SendPublish(sdp, true, false, 0)) 
		{
			nLive = 2;
			// 处理返回
			strMid = pZXEngine->mZXClient.strMid;
			strSfu = pZXEngine->mZXClient.strSfu;
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

void ZXPeerLocal::SendUnPublish()
{
	if (strMid == "")
	{
		return;
	}

	if (pZXEngine != nullptr)
	{
		pZXEngine->mZXClient.SendUnPublish(strMid, strSfu);
	}

	strMid = "";
	strSfu = "";
}

void ZXPeerLocal::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state)
{
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kConnected)
	{
		ZXEngine::writeLog("local peer kConnected");
		nLive = 4;
	}
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kDisconnected)
	{
		ZXEngine::writeLog("local peer kDisconnected");
		nLive = 0;
	}
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kFailed)
	{
		ZXEngine::writeLog("local peer kFailed");
		nLive = 0;
	}
}
