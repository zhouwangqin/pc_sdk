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
	// ������־���
	WEBRTC_API void setLogDebug(log_callback callback);

	// ���÷�������ַ
	WEBRTC_API void setServerIp(char *ip, unsigned short port);

	// ���ñ�����Ƶ���ص�
	WEBRTC_API void setLocalVideo(video_frame_callback callback);
	
	// ����Զ����Ƶ���ص�
	WEBRTC_API void setRemoteVideo(video_frame_callback callback);

	// ����sdk
	WEBRTC_API bool initSdk(char *uid);

	// �ͷ�sdk
	WEBRTC_API void freeSdk();

	// ���뷿��
	WEBRTC_API bool joinRoom(char *rid);

	// �뿪����
	WEBRTC_API void leaveRoom();

	// �����Ƿ�����
	WEBRTC_API void setPublish(bool bPub);

	// �����Ƿ���Ļ�����֡��(5-30)
	WEBRTC_API void setScreen(bool bPub);
	WEBRTC_API void setScreenPub(bool bPub);
	WEBRTC_API void setFrameRate(int nFrameRate);

	// ������˷�
	WEBRTC_API void setMicrophoneMute(bool mute);

	// ��ȡ��˷�״̬
	WEBRTC_API bool getMicrophoneMute();

#ifdef __cplusplus
}
#endif