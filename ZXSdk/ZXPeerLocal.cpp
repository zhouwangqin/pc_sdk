#include "pch.h"
#include "ZXBase.h"
#include "ZXEngine.h"
#include "ZXPeerLocal.h"

const int create_offer_sdp_ok = 1000;
const int create_offer_sdp_fail = 1001;
const int set_offer_sdp_ok = 1002;
const int set_offer_sdp_fail = 1003;
const int set_remote_sdp_ok = 1004;
const int set_remote_sdp_fail = 1005;
const int pub_send_fail = 1010;
const int peer_connect_fail = 1011;

ZXPeerLocal::ZXPeerLocal()
{
	strUid = "";
	strMid = "";
	strSfu = "";

	nLive = 0;
	bClose = false;
	pZXEngine = nullptr;

	error_ = "";
	offer_sdp_ = nullptr;
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
	if (msg->message_id == create_offer_sdp_ok)
	{
		SetLocalDescription(offer_sdp_);
	}
	if (msg->message_id == create_offer_sdp_fail)
	{
		nLive = 0;
		if (pZXEngine != nullptr) {
			pZXEngine->OnPeerPublish(false, error_);
		}
	}
	if (msg->message_id == set_offer_sdp_ok)
	{
		std::string sdp_;
		offer_sdp_->ToString(&sdp_);
		SendPublish(sdp_);
	}
	if (msg->message_id == set_offer_sdp_fail)
	{
		nLive = 0;
		if (pZXEngine != nullptr) {
			pZXEngine->OnPeerPublish(false, error_);
		}
	}
	if (msg->message_id == set_remote_sdp_ok)
	{
		nLive = 3;
		if (pZXEngine != nullptr) {
			pZXEngine->OnPeerPublish(true, "");
		}
	}
	if (msg->message_id == set_remote_sdp_fail)
	{
		nLive = 0;
		if (pZXEngine != nullptr) {
			pZXEngine->OnPeerPublish(false, error_);
		}
	}
	if (msg->message_id == pub_send_fail)
	{
		nLive = 0;
		if (pZXEngine != nullptr)
		{
			pZXEngine->OnPeerPublish(false, error_);
		}
	}
	if (msg->message_id == peer_connect_fail)
	{
		nLive = 0;
		if (pZXEngine != nullptr)
		{
			pZXEngine->OnPeerPublishError();
		}
	}
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
	if (audio_track_ != NULL)
	{
		audio_track_->set_enabled(bEnable);
	}
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
	RTC_LOG(LS_ERROR) << "local peer create offer sdp ok";

	if (bClose)
	{
		return;
	}

	offer_sdp_ = sdp;
	if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		pZXEngine->g_signaling_thread->Post(loc, this, create_offer_sdp_ok);
	}
}

void ZXPeerLocal::CreateSdpFail(std::string error)
{
	RTC_LOG(LS_ERROR) << "local peer create offer sdp fail";

	if (bClose) 
	{
		return;
	}

	error_ = error;
	if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		pZXEngine->g_signaling_thread->Post(loc, this, create_offer_sdp_fail);
	}
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
	RTC_LOG(LS_ERROR) << "local peer set offer sdp ok";

	if (bClose) 
	{
		return;
	}

	if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		pZXEngine->g_signaling_thread->Post(loc, this, set_offer_sdp_ok);
	}
}

void ZXPeerLocal::OnSetLoaclSdpFail(std::string error)
{
	RTC_LOG(LS_ERROR) << "local peer set offer sdp fail";

	if (bClose) 
	{
		return;
	}

	error_ = error;
	if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		pZXEngine->g_signaling_thread->Post(loc, this, set_offer_sdp_fail);
	}
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
	RTC_LOG(LS_ERROR) << "local peer set answer sdp ok";

	if (bClose) 
	{
		return;
	}

	if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		pZXEngine->g_signaling_thread->Post(loc, this, set_remote_sdp_ok);
	}
}

void ZXPeerLocal::OnSetRemoteSdpFail(std::string error)
{
	RTC_LOG(LS_ERROR) << "local peer set answer sdp fail";

	if (bClose) 
	{
		return;
	}

	error_ = error;
	if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		pZXEngine->g_signaling_thread->Post(loc, this, set_remote_sdp_fail);
	}
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
		RTC_LOG(LS_ERROR) << "local peer create start";
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
				}
			}

			nLive = 1;
			bClose = false;
			RTC_LOG(LS_ERROR) << "local peer create stop";
			return true;
		}
		RTC_LOG(LS_ERROR) << "local peer create stop";
	}
	return false;
}

void ZXPeerLocal::FreePeerConnection()
{
	if (!bClose)
	{
		nLive = 0;
		bClose = true;
		error_ = "";
		offer_sdp_ = nullptr;
		audio_track_ = nullptr;
		RTC_LOG(LS_ERROR) << "local peer free start";
		if (peer_connection_ != nullptr)
		{
			peer_connection_->Close();
			peer_connection_ = nullptr;
		}
		RTC_LOG(LS_ERROR) << "local peer free stop";
	}
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
			strSfu = pZXEngine->mZXClient.sfuId;
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

	error_ = "local peer send publish fail";
	if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		pZXEngine->g_signaling_thread->Post(loc, this, pub_send_fail);
	}
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
		RTC_LOG(LS_ERROR) << "local peer kConnected";
		nLive = 4;
	}
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kDisconnected)
	{
		RTC_LOG(LS_ERROR) << "local peer kDisconnected";
		if (bClose) 
		{
			return;
		}
		if (nLive != 0) 
		{
			if (nLive == 4) 
			{
				if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
				{
					rtc::Location loc(__FUNCTION__, __FILE__);
					pZXEngine->g_signaling_thread->Post(loc, this, peer_connect_fail);
				}
			}
			if (nLive != 4) 
			{
				error_ = "local peer ice disconnect";
				if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
				{
					rtc::Location loc(__FUNCTION__, __FILE__);
					pZXEngine->g_signaling_thread->Post(loc, this, pub_send_fail);
				}
			}
			nLive = 0;
		}
	}
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kFailed)
	{
		RTC_LOG(LS_ERROR) << "local peer kFailed";
		if (bClose) 
		{
			return;
		}
		if (nLive != 0) 
		{
			if (nLive == 4)
			{
				if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
				{
					rtc::Location loc(__FUNCTION__, __FILE__);
					pZXEngine->g_signaling_thread->Post(loc, this, peer_connect_fail);
				}
			}
			if (nLive == 3)
			{
				error_ = "local peer ice disconnect";
				if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
				{
					rtc::Location loc(__FUNCTION__, __FILE__);
					pZXEngine->g_signaling_thread->Post(loc, this, pub_send_fail);
				}
			}
			nLive = 0;
		}
	}
}
