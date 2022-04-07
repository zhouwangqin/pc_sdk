#include "pch.h"
#include "ZXSdpListen.h"
#include "ZXPeerLocal.h"
#include "ZXPeerRemote.h"

OfferCreateSessionDescriptionObserver::OfferCreateSessionDescriptionObserver()
{
	pZXPeerLocal = nullptr;
	pZXPeerRemote = nullptr;
}

OfferCreateSessionDescriptionObserver::~OfferCreateSessionDescriptionObserver()
{
	
}

OfferCreateSessionDescriptionObserver* OfferCreateSessionDescriptionObserver::Create()
{
	return new rtc::RefCountedObject<OfferCreateSessionDescriptionObserver>();
}

void OfferCreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
	if (pZXPeerLocal != nullptr && desc != nullptr)
	{
		pZXPeerLocal->CreateSdpSuc(desc);
	}
	if (pZXPeerRemote != nullptr && desc != nullptr)
	{
		pZXPeerRemote->CreateSdpSuc(desc);
	}
}

void OfferCreateSessionDescriptionObserver::OnFailure(webrtc::RTCError error)
{
	if (pZXPeerLocal != nullptr)
	{
		pZXPeerLocal->CreateSdpFail(error.message());
	}
	if (pZXPeerRemote != nullptr)
	{
		pZXPeerRemote->CreateSdpFail(error.message());
	}
}

OfferSetSessionDescriptionObserver::OfferSetSessionDescriptionObserver()
{
	pZXPeerLocal = nullptr;
	pZXPeerRemote = nullptr;
}

OfferSetSessionDescriptionObserver::~OfferSetSessionDescriptionObserver()
{

}

OfferSetSessionDescriptionObserver* OfferSetSessionDescriptionObserver::Create()
{
	return new rtc::RefCountedObject<OfferSetSessionDescriptionObserver>();
}

void OfferSetSessionDescriptionObserver::OnSuccess()
{
	if (pZXPeerLocal != nullptr)
	{
		pZXPeerLocal->OnSetLocalSdpSuc();
	}
	if (pZXPeerRemote != nullptr)
	{
		pZXPeerRemote->OnSetLocalSdpSuc();
	}
}

void OfferSetSessionDescriptionObserver::OnFailure(webrtc::RTCError error)
{
	if (pZXPeerLocal != nullptr)
	{
		pZXPeerLocal->OnSetLoaclSdpFail(error.message());
	}
	if (pZXPeerRemote != nullptr)
	{
		pZXPeerRemote->OnSetLoaclSdpFail(error.message());
	}
}

AnswerSetSessionDescriptionObserver::AnswerSetSessionDescriptionObserver()
{
	pZXPeerLocal = nullptr;
	pZXPeerRemote = nullptr;
}

AnswerSetSessionDescriptionObserver::~AnswerSetSessionDescriptionObserver()
{

}

AnswerSetSessionDescriptionObserver* AnswerSetSessionDescriptionObserver::Create()
{
	return new rtc::RefCountedObject<AnswerSetSessionDescriptionObserver>();
}

void AnswerSetSessionDescriptionObserver::OnSuccess()
{
	if (pZXPeerLocal != nullptr)
	{
		pZXPeerLocal->OnSetRemoteSdpSuc();
	}
	if (pZXPeerRemote != nullptr)
	{
		pZXPeerRemote->OnSetRemoteSdpSuc();
	}
}

void AnswerSetSessionDescriptionObserver::OnFailure(webrtc::RTCError error)
{
	if (pZXPeerLocal != nullptr)
	{
		pZXPeerLocal->OnSetRemoteSdpFail(error.message());
	}
	if (pZXPeerRemote != nullptr)
	{
		pZXPeerRemote->OnSetRemoteSdpFail(error.message());
	}
}