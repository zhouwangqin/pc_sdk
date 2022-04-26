#pragma once
#include "ZXVideo.h"
#include "ZXScreen.h"
#include "ZXSdpListen.h"
#include "api/peerconnectioninterface.h"

class ZXEngine;
class ZXPeerScreen : public webrtc::PeerConnectionObserver,
					 public rtc::MessageHandler
{
public:
	ZXPeerScreen();
	virtual ~ZXPeerScreen();

	// 启动推流
	void StartPublish();
	// 取消推流
	void StopPublish();

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

	// 发送推流
	void SendPublish(std::string sdp);
	// 发送取消推流
	void SendUnPublish();

private:
	// PeerConnection对象
	bool InitPeerConnection();
	void FreePeerConnection();

	// 设置发送端码率
	void SetBitrate();

	//
	// PeerConnectionObserver implementation
	//
	void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {}
	void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
	void OnRenegotiationNeeded() override {}
	void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {}
	void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;
	void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}
	void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {}

	// MessageHandler implementation
	void OnMessage(rtc::Message* msg) override;

public:
	// 参数
	std::string strUid;
	std::string strMid;
	std::string strSfu;

	// 连接状态
	int nLive;
	// 退出标记
	bool bClose;
	// 上层对象
	ZXEngine* pZXEngine;
	// 视频回调对象
	ZXVideoObserver local_video_observer_;

	// 采集帧率
	int nCapFrameRate;
	// 最低码率kbps
	int nMinBitRate;
	// 最高码率kbps
	int nMaxBitRate;
	// 初始码率kbps
	int nStartBitRate;

private:
	// offer sdp
	std::string sdp_;
	// rtc 对象
	rtc::scoped_refptr<ZXScreen> video_device_;
	rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_;
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

	// sdp 回调
	rtc::scoped_refptr<OfferCreateSessionDescriptionObserver> pOfferCreateSdpObserver;
	rtc::scoped_refptr<OfferSetSessionDescriptionObserver> pOfferSetSdpObserver;
	rtc::scoped_refptr<AnswerSetSessionDescriptionObserver> pAnswerSetSdpObserver;
};

