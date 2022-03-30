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
		RTC_LOG(LS_ERROR) << "local offer sdp create error = " << webrtc::ToString(error.type()) << ": " << error.message();
		pZXPeerLocal->CreateSdpFail(error.message());
	}
	if (pZXPeerRemote != nullptr)
	{
		RTC_LOG(LS_ERROR) << "remote offer sdp create error = " << webrtc::ToString(error.type()) << ": " << error.message();
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
		RTC_LOG(LS_ERROR) << "local offer sdp set error = " << webrtc::ToString(error.type()) << ": " << error.message();
		pZXPeerLocal->OnSetLoaclSdpFail(error.message());
	}
	if (pZXPeerRemote != nullptr)
	{
		RTC_LOG(LS_ERROR) << "remote offer sdp set error = " << webrtc::ToString(error.type()) << ": " << error.message();
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
		RTC_LOG(LS_ERROR) << "local answer sdp set error = " << webrtc::ToString(error.type()) << ": " << error.message();
		pZXPeerLocal->OnSetRemoteSdpFail(error.message());
	}
	if (pZXPeerRemote != nullptr)
	{
		RTC_LOG(LS_ERROR) << "remote answer sdp set error = " << webrtc::ToString(error.type()) << ": " << error.message();
		pZXPeerRemote->OnSetRemoteSdpFail(error.message());
	}
}