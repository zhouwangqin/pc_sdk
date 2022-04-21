#pragma once

// video frame callback
// video_type = 0 camera, 1 screen, 2 other
// video_frame type = i420
typedef void(*video_frame_callback)(const char* uid, int video_type, const unsigned char* data_y, const unsigned char* data_u, const unsigned char* data_v,
	int stride_y, int stride_u, int stride_v, unsigned int width, unsigned int height);

// audio frame callback
typedef void(*audio_frame_callback)(const char* uid, const void* audio_data, int bits_per_sample, int sample_rate, int number_of_channels, int number_of_frames);

// log callback
typedef void(*log_callback)(const char* msg);
