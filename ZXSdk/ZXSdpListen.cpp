#include "pch.h"
#include "ZXSdpListen.h"
#include "ZXPeerLocal.h"
#include "ZXPeerScreen.h"
#include "ZXPeerRemote.h"

OfferCreateSessionDescriptionObserver::OfferCreateSessionDescriptionObserver()
{
	pZXPeerLocal = nullptr;
	pZXPeerScreen = nullptr;
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
	if (pZXPeerScreen != nullptr && desc != nullptr)
	{
		pZXPeerScreen->CreateSdpSuc(desc);
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
	if (pZXPeerScreen != nullptr)
	{
		pZXPeerScreen->CreateSdpFail(error.message());
	}
	if (pZXPeerRemote != nullptr)
	{
		pZXPeerRemote->CreateSdpFail(error.message());
	}
}

OfferSetSessionDescriptionObserver::OfferSetSessionDescriptionObserver()
{
	pZXPeerLocal = nullptr;
	pZXPeerScreen = nullptr;
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
	if (pZXPeerScreen != nullptr)
	{
		pZXPeerScreen->OnSetLocalSdpSuc();
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
	if (pZXPeerScreen != nullptr)
	{
		pZXPeerScreen->OnSetLoaclSdpFail(error.message());
	}
	if (pZXPeerRemote != nullptr)
	{
		pZXPeerRemote->OnSetLoaclSdpFail(error.message());
	}
}

AnswerSetSessionDescriptionObserver::AnswerSetSessionDescriptionObserver()
{
	pZXPeerLocal = nullptr;
	pZXPeerScreen = nullptr;
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
	if (pZXPeerScreen != nullptr)
	{
		pZXPeerScreen->OnSetRemoteSdpSuc();
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
	if (pZXPeerScreen != nullptr)
	{
		pZXPeerScreen->OnSetRemoteSdpFail(error.message());
	}
	if (pZXPeerRemote != nullptr)
	{
		pZXPeerRemote->OnSetRemoteSdpFail(error.message());
	}
}