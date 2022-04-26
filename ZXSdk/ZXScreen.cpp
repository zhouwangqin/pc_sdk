#include "pch.h"
#include "ZXScreen.h"
#include "ZXEngine.h"
#include "ZXPeerScreen.h"
#include "third_party/libyuv/include/libyuv.h"

ZXScreen::ZXScreen()
{
	pZXPeerScreen = nullptr;

	open_ = false;
	i420_buffer_ = nullptr;
}

ZXScreen::~ZXScreen()
{

}

bool ZXScreen::initCapturer()
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto options = webrtc::DesktopCaptureOptions::CreateDefault();
	//options.set_allow_directx_capturer(true);
	std::unique_ptr<webrtc::DesktopCapturer> capturer = webrtc::DesktopCapturer::CreateScreenCapturer(options);
	capturer_.reset(new webrtc::DesktopAndCursorComposer(std::move(capturer), options));
	capturer_->Start(this);
	return true;
}

void ZXScreen::freeCapturer()
{
	stopCapture();

	std::lock_guard<std::mutex> lock(mutex_);
	capturer_.reset();
}

bool ZXScreen::startCapturer()
{
	if (pZXPeerScreen != nullptr)
	{
		if (pZXPeerScreen->pZXEngine != nullptr && pZXPeerScreen->pZXEngine->g_ws_thread_.get() != nullptr)
		{
			rtc::Location loc(__FUNCTION__, __FILE__);
			pZXPeerScreen->pZXEngine->g_ws_thread_->PostDelayed(loc, 1000 / pZXPeerScreen->nCapFrameRate, this, 2000);
		}
		open_ = true;
		return true;
	}
	return false;
}

void ZXScreen::stopCapture()
{
	if (open_)
	{
		open_ = false;
		if (pZXPeerScreen != nullptr)
		{
			if (pZXPeerScreen->pZXEngine != nullptr && pZXPeerScreen->pZXEngine->g_ws_thread_.get() != nullptr)
			{
				pZXPeerScreen->pZXEngine->g_ws_thread_->Clear(this, 2000);
			}
		}
	}
}

webrtc::MediaSourceInterface::SourceState ZXScreen::state() const
{
	return webrtc::MediaSourceInterface::kLive;
}

bool ZXScreen::remote() const
{
	return false;
}

bool ZXScreen::is_screencast() const
{
	return true;
}

absl::optional<bool> ZXScreen::needs_denoising() const
{
	return false;
}

void ZXScreen::OnMessage(rtc::Message * msg)
{
	if (msg->message_id == 2000)
	{
		if (!open_)
		{
			return;
		}

		CaptureFrame();

		if (!open_)
		{
			return;
		}

		if (pZXPeerScreen != nullptr)
		{
			if (pZXPeerScreen->pZXEngine != nullptr)
			{
				rtc::Location loc(__FUNCTION__, __FILE__);
				pZXPeerScreen->pZXEngine->g_ws_thread_->PostDelayed(loc, 1000 / pZXPeerScreen->nCapFrameRate, this, 2000);
			}
		}
	}
}

void ZXScreen::OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame)
{
	if (result != webrtc::DesktopCapturer::Result::SUCCESS)
	{
		return;
	}

	int width = frame->size().width();
	int height = frame->size().height();

	if (i420_buffer_ == nullptr || i420_buffer_->width() != width || i420_buffer_->height() != height)
	{
		i420_buffer_ = webrtc::I420Buffer::Create(width, height);
		//i420_buffer_low = webrtc::I420Buffer::Create(1280, 720);
	}
	libyuv::ConvertToI420(frame->data(), 0, i420_buffer_->MutableDataY(),
		i420_buffer_->StrideY(), i420_buffer_->MutableDataU(),
		i420_buffer_->StrideU(), i420_buffer_->MutableDataV(),
		i420_buffer_->StrideV(), 0, 0, width, height, width,
		height, libyuv::kRotate0, libyuv::FOURCC_ARGB);

	/*
	libyuv::I420Scale(i420_buffer_->MutableDataY(), i420_buffer_->StrideY(),
		i420_buffer_->MutableDataU(), i420_buffer_->StrideU(),
		i420_buffer_->MutableDataV(), i420_buffer_->StrideV(),
		width, height, i420_buffer_low->MutableDataY(), i420_buffer_low->StrideY(),
		i420_buffer_low->MutableDataU(), i420_buffer_low->StrideU(),
		i420_buffer_low->MutableDataV(), i420_buffer_low->StrideV(),
		1280, 720, libyuv::kFilterNone);

	libyuv::I420Rotate(i420_buffer_->MutableDataY(), i420_buffer_->StrideY(),
		i420_buffer_->MutableDataU(), i420_buffer_->StrideU(),
		i420_buffer_->MutableDataV(), i420_buffer_->StrideV(),
		i420_buffer_rotate->MutableDataY(), i420_buffer_rotate->StrideY(),
		i420_buffer_rotate->MutableDataU(), i420_buffer_rotate->StrideU(),
		i420_buffer_rotate->MutableDataV(), i420_buffer_rotate->StrideV(),
		width, height, libyuv::kRotate90);
	*/

	OnFrame(webrtc::VideoFrame(i420_buffer_, 0, 0, webrtc::kVideoRotation_0));
	frame.reset();
}

void ZXScreen::CaptureFrame()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (capturer_.get() != nullptr)
	{
		capturer_->CaptureFrame();
	}
}