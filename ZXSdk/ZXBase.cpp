#include "pch.h"
#include "ZXBase.h"

// rtc相关
const char kStreamId[] = "ARDAMS";
const char kAudioLabel[] = "ARDAMSa0";
const char kVideoLabel[] = "ARDAMSv0";

// 视频VP8编码参数
const std::string VIDEO_VP8_INTEL_HW_ENCODER_FIELDTRIAL = "WebRTC-IntelVP8/Enabled/";
const std::string VIDEO_FLEXFEC_FIELDTRIAL = "WebRTC-FlexFEC-03-Advertised/Enabled/WebRTC-FlexFEC-03/Enabled/";

// 信令服务器地址
uint16_t g_server_port = 8443;
//std::string g_server_ip = "120.238.78.213";
std::string g_server_ip = "49.235.93.74";
// 转发服务器地址
std::string g_relay_server_ip = "120.238.78.214:3478";

// 自定义消息
const int socket_disconnet_ = 1000;
const int set_offer_sdp_ok = 10000;

// 日志对象
log_callback log_callback_ = nullptr;