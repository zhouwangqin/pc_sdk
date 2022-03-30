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
	// 设置服务器
	WEBRTC_API void setServerIp(char *ip, unsigned short port);

	// 设置sdk回调
	WEBRTC_API void setSdkListen(msg_callback observer);

	// 初始化sdk
	WEBRTC_API bool initSdk(char *uid);

	// 释放sdk
	WEBRTC_API void freeSdk();

	// 启动连接
	WEBRTC_API bool start();

	// 停止连接
	WEBRTC_API void stop();

	// 返回连接状态
	WEBRTC_API bool getConnect();

	// 加入房间
	WEBRTC_API bool joinRoom(char *rid);

	// 离开房间
	WEBRTC_API void leaveRoom();

	// 启动推流
	WEBRTC_API void startPublish();

	// 停止推流
	WEBRTC_API void stopPublish();

	// 启动拉流
	WEBRTC_API void startSubscribe(char *uid, char *mid, char *sfu);
	
	// 停止拉流
	WEBRTC_API void stopSubscribe(char *mid);

	// 设置麦克风
	WEBRTC_API void setMicrophoneMute(bool mute);

	// 获取麦克风状态
	WEBRTC_API bool getMicrophoneMute();

#ifdef __cplusplus
}
#endif