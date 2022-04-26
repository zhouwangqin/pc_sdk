#pragma once
#include <mutex>
#include "api/video/i420_buffer.h"
#include "api/peerconnectioninterface.h"
#include "media/base/adaptedvideotracksource.h"
#include "modules/desktop_capture/desktop_and_cursor_composer.h"

class ZXPeerScreen;
class ZXScreen : public rtc::AdaptedVideoTrackSource,
				 public webrtc::DesktopCapturer::Callback,
				 public rtc::MessageHandler
{
public:
	ZXScreen();
	virtual ~ZXScreen();

	// 采集操作
	bool initCapturer();
	void freeCapturer();
	bool startCapturer();
	void stopCapture();

private:
	// MessageHandler implementation
	void OnMessage(rtc::Message* msg) override;

	// AdaptedVideoTrackSource implementation
	bool is_screencast() const override;
	absl::optional<bool> needs_denoising() const override;
	webrtc::MediaSourceInterface::SourceState state() const override;
	bool remote() const override;

	// DesktopCapturer::Callback implementation
	void OnCaptureResult(webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame) override;

	// 采集一帧
	void CaptureFrame();

public:
	ZXPeerScreen *pZXPeerScreen;

private:
	bool open_;
	std::mutex mutex_;
	// 采集对象
	rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer_;
	std::unique_ptr<webrtc::DesktopAndCursorComposer> capturer_;
};

