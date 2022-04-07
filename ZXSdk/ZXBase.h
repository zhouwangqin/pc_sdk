#pragma once
#include <string>

extern const char kStreamId[];
extern const char kAudioLabel[];
extern const char kVideoLabel[];

extern uint16_t g_server_port;
extern std::string g_server_ip;
extern std::string g_relay_server_ip;

extern const int socket_disconnet_;
extern const int stream_add_;
extern const int stream_remove_;

extern const std::string VIDEO_VP8_INTEL_HW_ENCODER_FIELDTRIAL;
extern const std::string VIDEO_FLEXFEC_FIELDTRIAL;