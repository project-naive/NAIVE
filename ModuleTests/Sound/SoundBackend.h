#pragma once

#include <fstream>
#include <iostream>
//#define SOUNDIO_STATIC_LIBRARY
#include <soundio/soundio.h>
#include <cassert>
#include <spdlog/spdlog.h>

struct SoundIOBackendInitInfo;
class SoundIODevice;
class SoundIO {
protected:
	friend SoundIODevice;
	SoundIo* soundio;
	SoundIoBackend prefered_backend;
	char* app_name = nullptr;
	static bool connect_default_backend(SoundIO* __this) {
		int err;
		if (__this->prefered_backend == SoundIoBackendNone) {
			err = soundio_connect(__this->soundio);
		}
		else if (soundio_have_backend(__this->prefered_backend)) {
			err = soundio_connect_backend(__this->soundio, __this->prefered_backend);
		}
		else {
			spdlog::warn("Sound backend: Prefered backend unavailable, defaulting to default");
			err = soundio_connect(__this->soundio);
		}
		if (err) {
			spdlog::error("Sound: Backend: SoundIO:  Cannot connect to backend: {}.", soundio_strerror(err));
			soundio_destroy(__this->soundio);
			__this->soundio = nullptr;
			return false;
		}
		return true;
	}
public:
	SoundIO(const SoundIOBackendInitInfo& info);
	~SoundIO();
	bool is_valid() {
		return soundio;
	}
	SoundIoBackend get_prefered_backend() {
		return prefered_backend;
	}
	SoundIoBackend get_current_backend() {
		return soundio->current_backend;
	}
	static void rtprio_warn_default() {
		spdlog::warn("Sound: Backend: SoundIO: Thread not initialized with realtime priority");
	}
	static void jack_error_default(const char* msg) {
		spdlog::error("Sound: Backend: SoundIO: Jack Error: {}", msg);
	}
	static void jack_info_default(const char* msg) {
		spdlog::error("Sound: Backend: SoundIO: Jack Info: {}", msg);
	}
	static void on_disconnect_default(SoundIo* sio, int err) {
		connect_default_backend((SoundIO*)(sio->userdata));
		//notify the streams and devices
	}
	static void on_change_default(SoundIo* sio) {
		//notify devices manager if the default has changed.
	}
	static void on_events_default(SoundIo* sio) {
		//do nothing by default
	}
};

struct SoundIOBackendInitInfo {
	const char* app_name = nullptr;
	SoundIoBackend prefered_backend = SoundIoBackendNone;
	void(*rtprio_warning_callback)() = SoundIO::rtprio_warn_default;
	void(*jack_error_callback)(const char* msg) = SoundIO::jack_error_default;
	void(*jack_info_callback)(const char* msg) = SoundIO::jack_info_default;
	void(*on_backend_disconnect)(SoundIo* sio, int err) = SoundIO::on_disconnect_default;
	void(*on_devices_change)(SoundIo* sio) = SoundIO::on_change_default;
	void(*on_events_signal)(SoundIo* sio) = SoundIO::on_events_default;
};

class SoundIOOutStream;

class SoundIODevice {
	friend SoundIOOutStream;
	SoundIODevice(SoundIO& parent, bool input = false, int device_index = 0) {
		if (input) {
			if (!device_index) {
				device = soundio_get_input_device(parent.soundio, soundio_default_input_device_index(parent.soundio));
			}
			else {
				device = soundio_get_input_device(parent.soundio, device_index);
			}
			if (!device && device_index) {
				spdlog::warn("Sound: Device: Requested device unavailable, defaulting to default.");
				device = soundio_get_input_device(parent.soundio, soundio_default_output_device_index(parent.soundio));;
			}
		}
		else {
			if (!device_index) {
				device = soundio_get_output_device(parent.soundio, soundio_default_output_device_index(parent.soundio));
			}
			else {
				device = soundio_get_output_device(parent.soundio, device_index);
			}
			if (!device && device_index) {
				spdlog::warn("Sound: Device: Requested device unavailable, defaulting to default.");
				device = soundio_get_output_device(parent.soundio, soundio_default_output_device_index(parent.soundio));;
			}
		}
		if (!device) {
			spdlog::error("Sound: Device: Cannot open device! Will not open up audio for now.");
			return;
		}
		spdlog::info("Sound: Device: Connected to device {}, with ID {}.", device->name, device->id);
		if (device->probe_error) {
			spdlog::error("Sound: Device: Cannot probe device: {}", soundio_strerror(device->probe_error));
			soundio_device_unref(device);
			device = nullptr;
			return;
		}
	}
	~SoundIODevice() {
		soundio_device_unref(device);
	}
	SoundIODevice(const SoundIODevice& src) {
		soundio_device_ref(src.device);
		device = src.device;
	}
	SoundIODevice(SoundIODevice&& src) {
		device = src.device;
	}
	SoundIODevice& operator=(const SoundIODevice& src) {
		if (device) soundio_device_unref(device);
		soundio_device_ref(src.device);
		device = src.device;
	}
	SoundIODevice& operator=(SoundIODevice&& src) {
		if (device) soundio_device_unref(device);
		device = src.device;
	}
	SoundIoDevice* device = nullptr;
};

struct SoundIOStreamInitInfo {
	SoundIoChannelLayout* prefered_layouts;
	int prefered_layout_count;
	SoundIoFormat prefered_format = SoundIoFormatInvalid;
	bool non_terminal_hint = false;
	char* name = nullptr;
	int prefered_sample_rate;
	//must implement
	void(*write_callback)(SoundIoOutStream* stream, int frame_count_min, int frame_count_max);
	void(*error_callback)(SoundIoOutStream* stream, int err) = nullptr;
	void(*underflow_callback)(SoundIoOutStream* stream) = nullptr;
};

class SoundIOOutStream {
	SoundIOOutStream(SoundIODevice& parent, const SoundIOStreamInitInfo& info);
	~SoundIOOutStream();
	SoundIODevice device;
	SoundIoOutStream* stream;
	char* stream_name = nullptr;
	SoundIoFormat prefered;
	int sample_rate;
	SoundIoChannelLayout GetLayout();
	SoundIoFormat GetFormat();
	int GetInternalSampleRate();
	int GetSampleRate();
	bool is_valid();
	static bool SetupLayout(SoundIOOutStream* __this, SoundIODevice& parent, const SoundIOStreamInitInfo& info);
	static bool SetupFormat(SoundIOOutStream* __this, SoundIODevice& parent, const SoundIOStreamInitInfo& info);

};

class SoundMixer {

};



