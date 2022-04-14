#pragma once
#include "api/peerconnectioninterface.h"

class ZXPeerLocal;
class ZXPeerScreen;
class ZXPeerRemote;
class OfferCreateSessionDescriptionObserver : public webrtc::CreateSessionDescriptionObserver
{
public:
	OfferCreateSessionDescriptionObserver();
	virtual ~OfferCreateSessionDescriptionObserver();

	static OfferCreateSessionDescriptionObserver* Create();

	void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
	void OnFailure(webrtc::RTCError error) override;

public:
	ZXPeerLocal *pZXPeerLocal;
	ZXPeerScreen *pZXPeerScreen;
	ZXPeerRemote *pZXPeerRemote;
};

class OfferSetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver
{
public:
	OfferSetSessionDescriptionObserver();
	virtual ~OfferSetSessionDescriptionObserver();

	static OfferSetSessionDescriptionObserver* Create();

	void OnSuccess() override;
	void OnFailure(webrtc::RTCError error) override;

public:
	ZXPeerLocal *pZXPeerLocal;
	ZXPeerScreen *pZXPeerScreen;
	ZXPeerRemote *pZXPeerRemote;
};

class AnswerSetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver
{
public:
	AnswerSetSessionDescriptionObserver();
	virtual ~AnswerSetSessionDescriptionObserver();

	static AnswerSetSessionDescriptionObserver* Create();

	void OnSuccess() override;
	void OnFailure(webrtc::RTCError error) override;

public:
	ZXPeerLocal *pZXPeerLocal;
	ZXPeerScreen *pZXPeerScreen;
	ZXPeerRemote *pZXPeerRemote;
};