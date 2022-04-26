#pragma once
#include "ZXListen.h"

#ifdef WEBRTC_EXPORTS
#define WEBRTC_API __declspec(dllexport)
#else
#define WEBRTC_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	// 设置日志输出
	WEBRTC_API void setLogDebug(log_callback callback);

	// 设置服务器地址
	WEBRTC_API void setServerIp(char *ip, unsigned short port);

	// 设置本地视频流回调
	WEBRTC_API void setLocalVideo(video_frame_callback callback);
	
	// 设置远端视频流回调
	WEBRTC_API void setRemoteVideo(video_frame_callback callback);

	// 加载sdk
	WEBRTC_API bool initSdk(char *uid);

	// 释放sdk
	WEBRTC_API void freeSdk();

	// 加入房间
	WEBRTC_API bool joinRoom(char *rid);

	// 离开房间
	WEBRTC_API void leaveRoom();

	// 设置是否推流
	WEBRTC_API void setPublish(bool bPub);

	// 设置是否屏幕共享和帧率(5-30)
	WEBRTC_API void setScreen(bool bPub);
	WEBRTC_API void setScreenPub(bool bPub);
	WEBRTC_API void setFrameRate(int nFrameRate);

	// 设置麦克风
	WEBRTC_API void setMicrophoneMute(bool mute);

	// 获取麦克风状态
	WEBRTC_API bool getMicrophoneMute();

#ifdef __cplusplus
}
#endif