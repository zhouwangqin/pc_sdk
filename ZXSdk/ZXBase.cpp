#include "pch.h"
#include "ZXBase.h"

const char kStreamId[] = "ARDAMS";
const char kAudioLabel[] = "ARDAMSa0";
const char kVideoLabel[] = "ARDAMSv0";

// �����������ַ
uint16_t g_server_port = 8443;
//std::string g_server_ip = "120.238.78.213";
std::string g_server_ip = "49.235.93.74";
// ת����������ַ
std::string g_relay_server_ip = "120.238.78.214:3478";

// �¼�����
const int socket_disconnet_ = 1000;
const int stream_add_ = 1010;
const int stream_remove_ = 1020;

// ��ƵVP8�������
const std::string VIDEO_VP8_INTEL_HW_ENCODER_FIELDTRIAL = "WebRTC-IntelVP8/Enabled/";
const std::string VIDEO_FLEXFEC_FIELDTRIAL = "WebRTC-FlexFEC-03-Advertised/Enabled/WebRTC-FlexFEC-03/Enabled/";
