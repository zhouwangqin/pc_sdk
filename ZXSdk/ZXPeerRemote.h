#pragma once
#include "ZXVideo.h"
#include "ZXSdpListen.h"
#include "api/peerconnectioninterface.h"

class ZXEngine;
class ZXPeerRemote : public webrtc::PeerConnectionObserver,
	public webrtc::AudioTrackSinkInterface,
	public rtc::MessageHandler
{
public:
	ZXPeerRemote();
	virtual ~ZXPeerRemote();

	// 启动拉流
	void StartSubscribe();
	// 取消拉流
	void StopSubscribe();

	// 创建Offer
	void CreateOffer();
	// offer创建成功
	void CreateSdpSuc(webrtc::SessionDescriptionInterface* sdp);
	// offer创建失败
	void CreateSdpFail(std::string error);

	// 设置本地sdp
	void SetLocalDescription(webrtc::SessionDescriptionInterface* sdp);
	// 设置本地sdp成功
	void OnSetLocalSdpSuc();
	// 设置本地sdp失败
	void OnSetLoaclSdpFail(std::string error);

	// 设置远端sdp
	void SetRemoteDescription(webrtc::SessionDescriptionInterface* sdp);
	// 设置远端sdp成功
	void OnSetRemoteSdpSuc();
	// 设置远端sdp失败
	void OnSetRemoteSdpFail(std::string error);

	// 发送订阅
	void SendSubscribe(std::string sdp);
	// 发送取消订阅
	void SendUnSubscribe();

private:
	// PeerConnection对象
	bool InitPeerConnection();
	void FreePeerConnection();

	//
	// PeerConnectionObserver implementation.
	//
	void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {}
	void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
	void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
	void OnRenegotiationNeeded() override {}
	void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {}
	void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;
	void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}
	void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {}

	// MessageHandler implementation
	void OnMessage(rtc::Message* msg) override;

	// AudioTrackSinkInterface implementation
	void OnData(const void* audio_data, int bits_per_sample, int sample_rate, size_t number_of_channels, size_t number_of_frames) override;

public:
	// 参数
	std::string strUid;
	std::string strMid;
	std::string strSfu;
	std::string strSid;

	// 连接状态
	int nLive;
	// 退出标记
	bool bClose;
	// 上层对象
	ZXEngine* pZXEngine;
	// 视频回调对象
	ZXVideoObserver remote_video_observer_;

private:
	// offer sdp
	std::string sdp_;
	// rtc 对象
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

	// sdp 回调
	rtc::scoped_refptr<OfferCreateSessionDescriptionObserver> pOfferCreateSdpObserver;
	rtc::scoped_refptr<OfferSetSessionDescriptionObserver> pOfferSetSdpObserver;
	rtc::scoped_refptr<AnswerSetSessionDescriptionObserver> pAnswerSetSdpObserver;
};

