#include "pch.h"
#include "ZXVideo.h"
#include "ZXEngine.h"
#include "ZXPeerScreen.h"
#include "ZXPeerRemote.h"

ZXVideoObserver::ZXVideoObserver()
{
	pZXPeerScreen = nullptr;
	pZXPeerRemote = nullptr;
}

ZXVideoObserver::~ZXVideoObserver()
{

}

void ZXVideoObserver::OnFrame(const webrtc::VideoFrame & frame)
{
	rtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer = frame.video_frame_buffer();
	if (buffer != nullptr)
	{
		rtc::scoped_refptr<webrtc::I420BufferInterface> i420_buffer = buffer->ToI420();
		if (i420_buffer != nullptr)
		{
			if (pZXPeerScreen != nullptr)
			{
				if (pZXPeerScreen->pZXEngine != nullptr)
				{
					if (pZXPeerScreen->pZXEngine->local_callback_ != nullptr)
					{
						pZXPeerScreen->pZXEngine->local_callback_(pZXPeerScreen->strUid.c_str(), 1, 
							i420_buffer->DataY(), i420_buffer->DataU(), i420_buffer->DataV(),
							i420_buffer->StrideY(), i420_buffer->StrideU(), i420_buffer->StrideV(),
							i420_buffer->width(), i420_buffer->height());
					}
				}
			}
			if (pZXPeerRemote != nullptr)
			{
				if (pZXPeerRemote->pZXEngine != nullptr)
				{
					if (pZXPeerRemote->pZXEngine->remote_callback_ != nullptr)
					{
						pZXPeerRemote->pZXEngine->remote_callback_(pZXPeerRemote->strUid.c_str(), 1,
							i420_buffer->DataY(), i420_buffer->DataU(), i420_buffer->DataV(),
							i420_buffer->StrideY(), i420_buffer->StrideU(), i420_buffer->StrideV(),
							i420_buffer->width(), i420_buffer->height());
					}
				}
			}
			i420_buffer = nullptr;
		}
		buffer = nullptr;
	}
}
