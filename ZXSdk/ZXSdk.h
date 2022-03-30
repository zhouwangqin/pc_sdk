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
	// ���÷�����
	WEBRTC_API void setServerIp(char *ip, unsigned short port);

	// ����sdk�ص�
	WEBRTC_API void setSdkListen(msg_callback observer);

	// ��ʼ��sdk
	WEBRTC_API bool initSdk(char *uid);

	// �ͷ�sdk
	WEBRTC_API void freeSdk();

	// ��������
	WEBRTC_API bool start();

	// ֹͣ����
	WEBRTC_API void stop();

	// ��������״̬
	WEBRTC_API bool getConnect();

	// ���뷿��
	WEBRTC_API bool joinRoom(char *rid);

	// �뿪����
	WEBRTC_API void leaveRoom();

	// ��������
	WEBRTC_API void startPublish();

	// ֹͣ����
	WEBRTC_API void stopPublish();

	// ��������
	WEBRTC_API void startSubscribe(char *uid, char *mid, char *sfu);
	
	// ֹͣ����
	WEBRTC_API void stopSubscribe(char *mid);

	// ������˷�
	WEBRTC_API void setMicrophoneMute(bool mute);

	// ��ȡ��˷�״̬
	WEBRTC_API bool getMicrophoneMute();

#ifdef __cplusplus
}
#endif