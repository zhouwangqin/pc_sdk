// ZXSdk.cpp : 定义静态库的函数。
//

#include "pch.h"
#include "framework.h"
#include "ZXSdk.h"
#include "ZXEngine.h"

// 全局对象
ZXEngine mZXEngine;

WEBRTC_API void setServerIp(char *ip, unsigned short port)
{
	mZXEngine.setServerIp(ip, port);
}

WEBRTC_API void setSdkListen(msg_callback observer)
{
	mZXEngine.setSdkListen(observer);
}

WEBRTC_API bool initSdk(char *uid)
{
	return mZXEngine.initSdk(uid);
}

WEBRTC_API void freeSdk()
{
	mZXEngine.freeSdk();
}

WEBRTC_API bool start()
{
	return mZXEngine.start();
}

WEBRTC_API void stop()
{
	mZXEngine.stop();
}

WEBRTC_API bool joinRoom(char *rid)
{
	return mZXEngine.joinRoom(rid);
}

WEBRTC_API void leaveRoom()
{
	mZXEngine.leaveRoom();
}

WEBRTC_API void setMicrophoneMute(bool mute)
{
	mZXEngine.setMicrophoneMute(mute);
}

WEBRTC_API bool getMicrophoneMute()
{
	return mZXEngine.getMicrophoneMute();
}
