#pragma once
#include "ZXSdpListen.h"
#include "api/peerconnectioninterface.h"

class ZXEngine;
class ZXPeerRemote : public webrtc::PeerConnectionObserver,
					 public rtc::MessageHandler
{
public:
	ZXPeerRemote();
	virtual ~ZXPeerRemote();

	// ��������
	void StartSubscribe();
	// ȡ������
	void StopSubscribe();

	// ����Offer
	void CreateOffer();
	// offer�����ɹ�
	void CreateSdpSuc(webrtc::SessionDescriptionInterface* sdp);
	// offer����ʧ��
	void CreateSdpFail(std::string error);

	// ���ñ���sdp
	void SetLocalDescription(webrtc::SessionDescriptionInterface* sdp);
	// ���ñ���sdp�ɹ�
	void OnSetLocalSdpSuc();
	// ���ñ���sdpʧ��
	void OnSetLoaclSdpFail(std::string error);

	// ����Զ��sdp
	void SetRemoteDescription(webrtc::SessionDescriptionInterface* sdp);
	// ����Զ��sdp�ɹ�
	void OnSetRemoteSdpSuc();
	// ����Զ��sdpʧ��
	void OnSetRemoteSdpFail(std::string error);

	//
	// PeerConnectionObserver implementation.
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

private:
	// PeerConnection����
	bool InitPeerConnection();
	void FreePeerConnection();

	// ��������
	void SendSubscribe(std::string sdp);
	// ����ȡ������
	void SendUnSubscribe();

public:
	// ����
	std::string strUid;
	std::string strMid;
	std::string strSfu;
	std::string strSid;

	// ����״̬
	int nLive;
	// �˳����
	bool bClose;
	// �ϲ����
	ZXEngine* pZXEngine;

private:
	// offer sdp
	std::string error_;
	webrtc::SessionDescriptionInterface* offer_sdp_;
	// rtc ����
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

	// sdp �ص�
	rtc::scoped_refptr<OfferCreateSessionDescriptionObserver> pOfferCreateSdpObserver;
	rtc::scoped_refptr<OfferSetSessionDescriptionObserver> pOfferSetSdpObserver;
	rtc::scoped_refptr<AnswerSetSessionDescriptionObserver> pAnswerSetSdpObserver;
};
