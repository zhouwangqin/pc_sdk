#include "pch.h"
#include "ZXBase.h"
#include "ZXEngine.h"
#include "ZXPeerScreen.h"

ZXPeerScreen::ZXPeerScreen()
{
	strUid = "";
	strMid = "";
	strSfu = "";

	nLive = 0;
	bClose = false;
	pZXEngine = nullptr;
	local_video_observer_.pZXPeerScreen = this;

	nCapFrameRate = 8;
	nMinBitRate = 100;
	nMaxBitRate = 1500;
	nStartBitRate = 1000;

	sdp_ = "";
	video_device_ = nullptr;
	video_track_ = nullptr;
	peer_connection_ = nullptr;

	pOfferCreateSdpObserver = OfferCreateSessionDescriptionObserver::Create();
	pOfferCreateSdpObserver->pZXPeerScreen = this;
	pOfferSetSdpObserver = OfferSetSessionDescriptionObserver::Create();
	pOfferSetSdpObserver->pZXPeerScreen = this;
	pAnswerSetSdpObserver = AnswerSetSessionDescriptionObserver::Create();
	pAnswerSetSdpObserver->pZXPeerScreen = this;
}

ZXPeerScreen::~ZXPeerScreen()
{
	pOfferCreateSdpObserver = nullptr;
	pOfferSetSdpObserver = nullptr;
	pAnswerSetSdpObserver = nullptr;
}

void ZXPeerScreen::StartPublish()
{
	InitPeerConnection();
	if (pZXEngine != nullptr && pZXEngine->bScreenPub)
	{
		CreateOffer();
	}
}

void ZXPeerScreen::StopPublish()
{
	SendUnPublish();
	FreePeerConnection();
}

void ZXPeerScreen::CreateOffer()
{
	if (peer_connection_ != nullptr)
	{
		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		options.offer_to_receive_audio = false;
		options.offer_to_receive_video = false;
		peer_connection_->CreateOffer(pOfferCreateSdpObserver, options);
	}
}

void ZXPeerScreen::CreateSdpSuc(webrtc::SessionDescriptionInterface* sdp)
{
	ZXEngine::writeLog("screen peer create offer sdp ok");
	if (bClose)
	{
		return;
	}

	sdp->ToString(&sdp_);
	SetLocalDescription(sdp);
}

void ZXPeerScreen::CreateSdpFail(std::string error)
{
	std::string msg = "screen peer create offer sdp fail = " + error;
	ZXEngine::writeLog(msg);

	nLive = 0;
}

void ZXPeerScreen::SetLocalDescription(webrtc::SessionDescriptionInterface* sdp)
{
	if (peer_connection_ != nullptr)
	{
		peer_connection_->SetLocalDescription(pOfferSetSdpObserver, sdp);
	}
}

void ZXPeerScreen::OnSetLocalSdpSuc()
{
	ZXEngine::writeLog("screen peer set offer sdp ok");
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

void ZXPeerScreen::OnSetLoaclSdpFail(std::string error)
{
	std::string msg = "screen peer set offer sdp fail = " + error;
	ZXEngine::writeLog(msg);

	nLive = 0;
}

void ZXPeerScreen::SetRemoteDescription(webrtc::SessionDescriptionInterface* sdp)
{
	if (peer_connection_ != nullptr)
	{
		peer_connection_->SetRemoteDescription(pAnswerSetSdpObserver, sdp);
	}
}

void ZXPeerScreen::OnSetRemoteSdpSuc()
{
	ZXEngine::writeLog("screen peer set answer sdp ok");
	if (bClose)
	{
		return;
	}

	nLive = 3;
}

void ZXPeerScreen::OnSetRemoteSdpFail(std::string error)
{
	std::string msg = "screen peer set answer sdp fail = " + error;
	ZXEngine::writeLog(msg);

	nLive = 0;
}

bool ZXPeerScreen::InitPeerConnection()
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
		ZXEngine::writeLog("screen peer create start");
		peer_connection_ = pZXEngine->peer_connection_factory_->CreatePeerConnection(config, nullptr, nullptr, this);
		if (peer_connection_ != nullptr)
		{
			video_device_ = new rtc::RefCountedObject<ZXScreen>();
			if (video_device_ != nullptr)
			{
				video_track_ = pZXEngine->peer_connection_factory_->CreateVideoTrack(kVideoLabel, video_device_);
				if (video_track_ != nullptr)
				{
					peer_connection_->AddTrack(video_track_, { kStreamId });
					video_track_->AddOrUpdateSink(&local_video_observer_, rtc::VideoSinkWants());
					// 启动屏幕共享
					video_device_->pZXPeerScreen = this;
					video_device_->initCapturer();
					video_device_->startCapturer();
				}
			}

			nLive = 1;
			bClose = false;
			ZXEngine::writeLog("screen peer create stop");
			return true;
		}
		ZXEngine::writeLog("screen peer create stop1");
	}
	return false;
}

void ZXPeerScreen::FreePeerConnection()
{
	if (bClose)
	{
		return;
	}

	nLive = 0;
	bClose = true;
	sdp_ = "";
	video_track_ = nullptr;
	ZXEngine::writeLog("screen peer free start");
	if (video_device_ != nullptr)
	{
		video_device_->freeCapturer();
		video_device_ = nullptr;
	}
	if (peer_connection_ != nullptr)
	{
		peer_connection_->Close();
		peer_connection_ = nullptr;
	}
	ZXEngine::writeLog("screen peer free stop");
}

void ZXPeerScreen::SetBitrate()
{
	if (peer_connection_ == nullptr)
	{
		return;
	}

	// 设置码率控制模块
	webrtc::BitrateSettings mBitrateSettings;
	mBitrateSettings.max_bitrate_bps = nMaxBitRate * 1000;
	mBitrateSettings.min_bitrate_bps = nMinBitRate * 1000;
	mBitrateSettings.start_bitrate_bps = nStartBitRate * 1000;
	peer_connection_->SetBitrate(mBitrateSettings);
	// 设置编码控制模块
	for (size_t i = 0; i < peer_connection_->GetSenders().size(); i++)
	{
		rtc::scoped_refptr<webrtc::RtpSenderInterface> rtpSend = peer_connection_->GetSenders()[i];
		if (rtpSend->media_type() == cricket::MEDIA_TYPE_VIDEO)
		{
			webrtc::RtpParameters rtpParam = rtpSend->GetParameters();
			for (size_t i = 0; i < rtpParam.encodings.size(); i++)
			{
				rtpParam.encodings[i].max_bitrate_bps = nMaxBitRate * 1000;
				rtpParam.encodings[i].min_bitrate_bps = nMinBitRate * 1000;
				rtpParam.encodings[i].max_framerate = nCapFrameRate;
			}
			rtpSend->SetParameters(rtpParam);
		}
	}
}

void ZXPeerScreen::SendPublish(std::string sdp)
{
	if (bClose)
	{
		return;
	}

	if (pZXEngine != nullptr && sdp != "")
	{
		if (pZXEngine->mZXClient.SendPublish(sdp, false, true, 1))
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

void ZXPeerScreen::SendUnPublish()
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

void ZXPeerScreen::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state)
{
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kConnected)
	{
		ZXEngine::writeLog("screen peer kConnected");
		nLive = 4;
	}
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kDisconnected)
	{
		ZXEngine::writeLog("screen peer kDisconnected");
		nLive = 0;
	}
	if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kFailed)
	{
		ZXEngine::writeLog("screen peer kFailed");
		nLive = 0;
	}
}

void ZXPeerScreen::OnMessage(rtc::Message * msg)
{
	if (msg->message_id == set_offer_sdp_ok)
	{
		SetBitrate();
		SendPublish(sdp_);
	}
}

