#include "pch.h"
#include "ZXBase.h"
#include "ZXEngine.h"
#include "ZXPeerRemote.h"

const int create_offer_sdp_ok = 1000;
const int create_offer_sdp_fail = 1001;
const int set_offer_sdp_ok = 1002;
const int set_offer_sdp_fail = 1003;
const int set_remote_sdp_ok = 1004;
const int set_remote_sdp_fail = 1005;
const int sub_send_fail = 1010;
const int peer_connect_fail = 1011;

ZXPeerRemote::ZXPeerRemote()
{
	strUid = "";
	strMid = "";
	strSfu = "";
	strSid = "";

	nLive = 0;
	bClose = false;
	pZXEngine = nullptr;

	error_ = "";
	offer_sdp_ = nullptr;
	peer_connection_ = nullptr;

	pOfferCreateSdpObserver = OfferCreateSessionDescriptionObserver::Create();
	pOfferCreateSdpObserver->pZXPeerRemote = this;
	pOfferSetSdpObserver = OfferSetSessionDescriptionObserver::Create();
	pOfferSetSdpObserver->pZXPeerRemote = this;
	pAnswerSetSdpObserver = AnswerSetSessionDescriptionObserver::Create();
	pAnswerSetSdpObserver->pZXPeerRemote = this;
}

void ZXPeerRemote::OnMessage(rtc::Message * msg)
{
	if (msg->message_id == create_offer_sdp_ok)
	{
		SetLocalDescription(offer_sdp_);
	}
	if (msg->message_id == create_offer_sdp_fail)
	{
		nLive = 0;
		if (pZXEngine != nullptr) 
		{
			pZXEngine->OnPeerSubscribe(strMid, false, error_);
		}
	}
	if (msg->message_id == set_offer_sdp_ok)
	{
		std::string sdp_;
		offer_sdp_->ToString(&sdp_);
		SendSubscribe(sdp_);
	}
	if (msg->message_id == set_offer_sdp_fail)
	{
		nLive = 0;
		if (pZXEngine != nullptr) 
		{
			pZXEngine->OnPeerSubscribe(strMid, false, error_);
		}
	}
	if (msg->message_id == set_remote_sdp_ok)
	{
		nLive = 3;
		if (pZXEngine != nullptr) 
		{
			pZXEngine->OnPeerSubscribe(strMid, true, "");
		}
	}
	if (msg->message_id == set_remote_sdp_fail)
	{
		nLive = 0;
		if (pZXEngine != nullptr) 
		{
			pZXEngine->OnPeerSubscribe(strMid, false, error_);
		}
	}
	if (msg->message_id == sub_send_fail)
	{
		nLive = 0;
		if (pZXEngine != nullptr) 
		{
			pZXEngine->OnPeerSubscribe(strMid, false, error_);
		}
	}
	if (msg->message_id == peer_connect_fail)
	{
		nLive = 0;
		if (pZXEngine != nullptr)
		{
			pZXEngine->OnPeerSubscribeError(strMid);
		}
	}
}

ZXPeerRemote::~ZXPeerRemote()
{
	pOfferCreateSdpObserver = nullptr;
	pOfferSetSdpObserver = nullptr;
	pAnswerSetSdpObserver = nullptr;
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
	if (peer_connection_ != NULL)
	{
		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		peer_connection_->CreateOffer(pOfferCreateSdpObserver, options);
	}
}

void ZXPeerRemote::CreateSdpSuc(webrtc::SessionDescriptionInterface * sdp)
{
	RTC_LOG(LS_ERROR) << "remote peer create offer sdp ok = " << strMid;

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

void ZXPeerRemote::CreateSdpFail(std::string error)
{
	RTC_LOG(LS_ERROR) << "remote peer create offer sdp fail = " << strMid;

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

void ZXPeerRemote::SetLocalDescription(webrtc::SessionDescriptionInterface* sdp)
{
	if (peer_connection_ != NULL)
	{
		peer_connection_->SetLocalDescription(pOfferSetSdpObserver, sdp);
	}
}

void ZXPeerRemote::OnSetLocalSdpSuc()
{
	RTC_LOG(LS_ERROR) << "remote peer set offer sdp ok = " << strMid;

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

void ZXPeerRemote::OnSetLoaclSdpFail(std::string error)
{
	RTC_LOG(LS_ERROR) << "remote peer set offer sdp fail = " << strMid;

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

void ZXPeerRemote::SetRemoteDescription(webrtc::SessionDescriptionInterface* sdp)
{
	if (peer_connection_ != NULL)
	{
		peer_connection_->SetRemoteDescription(pAnswerSetSdpObserver, sdp);
	}
}

void ZXPeerRemote::OnSetRemoteSdpSuc()
{
	RTC_LOG(LS_ERROR) << "remote peer set answer sdp ok = " << strMid;

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

void ZXPeerRemote::OnSetRemoteSdpFail(std::string error)
{
	RTC_LOG(LS_ERROR) << "remote peer set answer sdp fail = " << strMid;

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
	config.continual_gathering_policy = webrtc::PeerConnectionInterface::GATHER_ONCE;
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
		RTC_LOG(LS_ERROR) << "remote peer create start = " << strMid;
		peer_connection_ = pZXEngine->peer_connection_factory_->CreatePeerConnection(config, NULL, NULL, this);
		if (peer_connection_ != nullptr)
		{
			peer_connection_->AddTransceiver(cricket::MediaType::MEDIA_TYPE_AUDIO);

			nLive = 1;
			bClose = false;
			RTC_LOG(LS_ERROR) << "remote peer create stop = " << strMid;
			return true;
		}
		RTC_LOG(LS_ERROR) << "remote peer create stop = " << strMid;
	}
	return false;
}

void ZXPeerRemote::FreePeerConnection()
{
	if (!bClose)
	{
		nLive = 0;
		bClose = true;
		error_ = "";
		offer_sdp_ = nullptr;
		RTC_LOG(LS_ERROR) << "remote peer free start = " << strMid;
		if (peer_connection_ != nullptr)
		{
			peer_connection_->Close();
			peer_connection_ = nullptr;
		}
		RTC_LOG(LS_ERROR) << "remote peer free stop = " << strMid;
	}
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

	error_ = "remote peer send subscribe fail";
	if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
	{
		rtc::Location loc(__FUNCTION__, __FILE__);
		pZXEngine->g_signaling_thread->Post(loc, this, sub_send_fail);
	}
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

void ZXPeerRemote::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state)
{
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kConnected)
	{
		RTC_LOG(LS_ERROR) << "remote peer kConnected = " << strMid;
		nLive = 4;
	}
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kDisconnected)
	{
		RTC_LOG(LS_ERROR) << "remote peer kDisconnected = " << strMid;
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
				error_ = "remote peer ice disconnect";
				if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
				{
					rtc::Location loc(__FUNCTION__, __FILE__);
					pZXEngine->g_signaling_thread->Post(loc, this, sub_send_fail);
				}
			}
			nLive = 0;
		}
	}
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kFailed)
	{
		RTC_LOG(LS_ERROR) << "remote peer kFailed = " << strMid;
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
				error_ = "remote peer ice disconnect";
				if (pZXEngine != nullptr && pZXEngine->g_signaling_thread != nullptr)
				{
					rtc::Location loc(__FUNCTION__, __FILE__);
					pZXEngine->g_signaling_thread->Post(loc, this, sub_send_fail);
				}
			}
			nLive = 0;
		}
	}
}

