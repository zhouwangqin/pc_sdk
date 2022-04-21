#pragma once
#include <string>
#include "ZXListen.h"

// rtc相关
extern const char kStreamId[];
extern const char kAudioLabel[];
extern const char kVideoLabel[];

extern const std::string VIDEO_VP8_INTEL_HW_ENCODER_FIELDTRIAL;
extern const std::string VIDEO_FLEXFEC_FIELDTRIAL;

// 服务器地址
extern uint16_t g_server_port;
extern std::string g_server_ip;
extern std::string g_relay_server_ip;

// 自定义消息
extern const int socket_disconnet_;
extern const int set_offer_sdp_ok;

// 日志对象
extern log_callback log_callback_;