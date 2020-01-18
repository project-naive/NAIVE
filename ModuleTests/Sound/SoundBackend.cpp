#include "SoundBackend.h"

SoundIO::SoundIO(const SoundIOBackendInitInfo & info) {
	soundio = soundio_create();
	if (!soundio)
		return;
	soundio->userdata = this;
	if (info.app_name) {
		size_t length = strlen(info.app_name);
		app_name = (char*)malloc(length + 1);
		assert(app_name);
		strcpy(app_name, info.app_name);
		soundio->app_name = app_name;
	}
	prefered_backend = info.prefered_backend;
	if (info.rtprio_warning_callback)
		soundio->emit_rtprio_warning = info.rtprio_warning_callback;
	if (info.jack_error_callback)
		soundio->jack_error_callback = info.jack_error_callback;
	if (info.jack_info_callback)
		soundio->jack_info_callback = info.jack_info_callback;
	if (info.on_backend_disconnect)
		soundio->on_backend_disconnect = info.on_backend_disconnect;
	if (info.on_devices_change)
		soundio->on_devices_change = info.on_devices_change;
	if (info.on_events_signal)
		soundio->on_events_signal = info.on_events_signal;
	int num_backend = soundio_backend_count(soundio);
	if (!num_backend) {
		spdlog::error("Sound: SoundIO Error: No backend available!");
		soundio_destroy(soundio);
		soundio = nullptr;
		return;
	}
	spdlog::info("Sound: Backend: SoundIO: A total of {} backends available.", num_backend);
	for (int i = 0; i < num_backend; ++i) {
		spdlog::info("Available sound backend: {}", soundio_backend_name(soundio_get_backend(soundio, i)));
	}
	if (!connect_default_backend(this)) {
		soundio_destroy(soundio);
		soundio = nullptr;
		return;
	}
	spdlog::info("Sound: Backend: SoundIO: Connected to backend {}.", soundio_backend_name(soundio->current_backend));
	soundio_flush_events(soundio);
	int num_devices = soundio_output_device_count(soundio);
	spdlog::info("Sound: Backend: SoundIO: {} output devices available.", num_devices);
	std::string master_string;
	master_string.reserve(500);
	for (int i = 0; i < num_devices; ++i) {
		master_string = "\nDevice: {}\nID: {}\nRaw: {}\n{} layouts available.";
		SoundIoDevice* device = soundio_get_output_device(soundio, i);
		int num_layout = device->layout_count;
		for (int j = 0; j < num_layout; ++j) {
			master_string += "\n  Layout: ";
			master_string += device->layouts[j].name;
			master_string += "\n    Channels:";
			int num_channel = device->layouts[j].channel_count;
			for (int k = 0; k < num_channel; ++k) {
				master_string += "  ";
				master_string += soundio_get_channel_name(device->layouts[j].channels[k]);
			}
		}
		master_string += "\n{} Formats available.\n  Formats:";
		int num_formats = device->format_count;
		for (int j = 0; j < num_formats; ++j) {
			master_string += "\n    ";
			master_string += soundio_format_string(device->formats[j]);
		}
		master_string += "\n{} Sample Rates available.\n  Rates:";
		int num_rate = device->sample_rate_count;
		for (int j = 0; j < num_rate; ++j) {
			char buffer[25];
			master_string += "\n     ";
			master_string += itoa(device->sample_rates[j].min, buffer, 10);
			master_string += '~';
			master_string += itoa(device->sample_rates[j].max, buffer, 10);
		}
		spdlog::info(master_string,
			device->name, device->id, device->is_raw, num_layout, num_formats, num_rate);
		if (device->probe_error) {
			std::cerr << "Cannot porbe device: " << soundio_strerror(device->probe_error) << '\n';
		}
		soundio_device_unref(device);
	}
	num_devices = soundio_input_device_count(soundio);
	spdlog::info("Sound: Backend: SoundIO: {} input devices available.", num_devices);
	for (int i = 0; i < num_devices; ++i) {
		master_string = "\nDevice: {}\nID: {}\nRaw: {}\n{} layouts available.";
		SoundIoDevice* device = soundio_get_input_device(soundio, i);
		int num_layout = device->layout_count;
		for (int j = 0; j < num_layout; ++j) {
			master_string += "\n  Layout: ";
			master_string += device->layouts[j].name;
			master_string += "\n    Channels:";
			int num_channel = device->layouts[j].channel_count;
			for (int k = 0; k < num_channel; ++k) {
				master_string += "  ";
				master_string += soundio_get_channel_name(device->layouts[j].channels[k]);
			}
		}
		master_string += "\n{} Formats available.\n  Formats:";
		int num_formats = device->format_count;
		for (int j = 0; j < num_formats; ++j) {
			master_string += "\n    ";
			master_string += soundio_format_string(device->formats[j]);
		}
		master_string += "\n{} Sample Rates available.\n  Rates:";
		int num_rate = device->sample_rate_count;
		for (int j = 0; j < num_rate; ++j) {
			char buffer[25];
			master_string += "\n     ";
			master_string += itoa(device->sample_rates[j].min, buffer, 10);
			master_string += '~';
			master_string += itoa(device->sample_rates[j].max, buffer, 10);
		}
		spdlog::info(master_string,
			device->name, device->id, device->is_raw, num_layout, num_formats, num_rate);
		if (device->probe_error) {
			std::cerr << "Cannot porbe device: " << soundio_strerror(device->probe_error) << '\n';
		}
		soundio_device_unref(device);
	}
}

SoundIO::~SoundIO() {
	if (soundio) {
		soundio_disconnect(soundio);
		soundio_destroy(soundio);
	}
	if (app_name) {
		free(app_name);
	}
}

SoundIOOutStream::SoundIOOutStream(SoundIODevice & parent, const SoundIOStreamInitInfo & info) :
	device(parent) {
	stream = soundio_outstream_create(parent.device);
	if (!stream)
		return;
	if (!SetupLayout(this, parent, info)) {
		soundio_outstream_destroy(stream);
		stream = nullptr;
		return;
	}
	if (!SetupFormat(this, parent, info)) {
		soundio_outstream_destroy(stream);
		stream = nullptr;
		return;
	}
	if (soundio_device_supports_sample_rate(parent.device, info.prefered_sample_rate)) {
		stream->sample_rate = info.prefered_sample_rate;
	}
	else {
		if (!parent.device->sample_rate_count) {
			spdlog::error("Sound: SoundIo Outstream: Cannot detect device sample rate!");
			soundio_outstream_destroy(stream);
			stream = nullptr;
		}
		stream->sample_rate = soundio_device_nearest_sample_rate(parent.device, info.prefered_sample_rate);
		//setup sample rate conversion
	}
	stream->userdata = this;
	stream->volume = 1.0;
	//set up the callbacks
	stream->write_callback = info.write_callback;
	if (info.error_callback)
		stream->error_callback = info.error_callback;
	if (info.underflow_callback)
		stream->underflow_callback = info.underflow_callback;
	stream->non_terminal_hint = info.non_terminal_hint;
	if (info.name) {
		stream_name = (char*)malloc(strlen(info.name) + 1);
		assert(stream_name);
		strcpy(stream_name, info.name);
		stream->name = stream_name;
	}
	int err = soundio_outstream_open(stream);
	if (err) {
		spdlog::error("Sound: Soundio Outstream: Opening stream failed with error: {}.", soundio_strerror(err));
		soundio_outstream_destroy(stream);
		stream = nullptr;
		return;
	}
	double latency;
	soundio_outstream_get_latency(stream, &latency);
	spdlog::info("Sound: Soundio Outstream: Stream opened with latency: {}.", latency);
}

SoundIOOutStream::~SoundIOOutStream() {
	if (stream)
		soundio_outstream_destroy(stream);
	if (stream_name)
		free(stream_name);
}

SoundIoChannelLayout SoundIOOutStream::GetLayout() {
	return stream->layout;
}

SoundIoFormat SoundIOOutStream::GetFormat() {
	return stream->format;
}

int SoundIOOutStream::GetInternalSampleRate() {
	return stream->sample_rate;
}

int SoundIOOutStream::GetSampleRate() {
	return sample_rate;
}

bool SoundIOOutStream::is_valid() {
	return stream;
}

bool SoundIOOutStream::SetupLayout(SoundIOOutStream * __this, SoundIODevice & parent, const SoundIOStreamInitInfo & info) {
	const SoundIoChannelLayout* gotten_layout =
		soundio_best_matching_channel_layout(
			info.prefered_layouts, info.prefered_layout_count,
			parent.device->layouts, parent.device->layout_count);
	if (!gotten_layout) {
		if (soundio_device_supports_layout(parent.device,
			soundio_channel_layout_get_builtin(SoundIoChannelLayoutIdStereo))) {
			gotten_layout = soundio_channel_layout_get_builtin(SoundIoChannelLayoutIdStereo);
		}
		else {
			if (!(parent.device->layout_count)) {
				spdlog::error("Soundio: Soundio outstream: cannot detect supported layout.");
				return false;
			}
			soundio_sort_channel_layouts(parent.device->layouts, parent.device->layout_count);
			SoundIoChannelLayout default_to = parent.device->layouts[0];
			if (default_to.channel_count == 1) {
				//only mono supported...
				spdlog::error("Sound: Soundio outstream: cannot find capable layout, "
					"Using the only available channel for mono output \"{}\". "
					"Quality may be impaired.", default_to.name);
				//set up the channel mapping here, use stero and mix down to mono in mapping

			}
			else {
				spdlog::error("Sound: Soundio outstream: cannot find capable layout, "
					"using first two channels as stero output. Original layout: {}. "
					"Quality may be impaired.", default_to.name);
				//set up the channel mapping here, use stero and write only to the first two channels,
				//filling the rest with zeros.

			}
			__this->stream->layout = default_to;
		}
	}
	else {
		__this->stream->layout = *gotten_layout;
	}
	return true;
}

bool SoundIOOutStream::SetupFormat(SoundIOOutStream * __this, SoundIODevice & parent, const SoundIOStreamInitInfo & info) {
	if (!parent.device->format_count) {
		spdlog::error("Sound: Soundio outstream: Cannot detect device available formats.");
		return false;
	}
	if (info.prefered_format) {
		if (soundio_device_supports_format(parent.device, info.prefered_format)) {
			__this->stream->format = info.prefered_format;
			//set up conversion
			spdlog::info("Sound: SoundIO Outstream: Set up stream to use prefered format: {}.",
				soundio_format_string(info.prefered_format));
			return true;
		}
		else {
			spdlog::warn("Sound: SoundIO Outstream: Cannot apply prefered Format, using default.");
		}
	}
	if (soundio_device_supports_format(parent.device, SoundIoFormatFloat32NE)) {
		__this->stream->format = SoundIoFormatFloat32NE;
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatFloat32FE)) {
		__this->stream->format = SoundIoFormatFloat32FE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatFloat64NE)) {
		__this->stream->format = SoundIoFormatFloat64NE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatFloat64FE)) {
		__this->stream->format = SoundIoFormatFloat64FE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatU32NE)) {
		__this->stream->format = SoundIoFormatU32NE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatU32FE)) {
		__this->stream->format = SoundIoFormatU32FE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatS32NE)) {
		__this->stream->format = SoundIoFormatS32NE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatS32FE)) {
		__this->stream->format = SoundIoFormatS32FE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatU24NE)) {
		__this->stream->format = SoundIoFormatU24NE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatU24FE)) {
		__this->stream->format = SoundIoFormatU24FE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatU24NE)) {
		__this->stream->format = SoundIoFormatU24NE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatS24FE)) {
		__this->stream->format = SoundIoFormatS24FE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatU16NE)) {
		__this->stream->format = SoundIoFormatU16NE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatU16FE)) {
		__this->stream->format = SoundIoFormatU16FE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatS16NE)) {
		__this->stream->format = SoundIoFormatS16NE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatS16FE)) {
		__this->stream->format = SoundIoFormatS16FE;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatU8)) {
		__this->stream->format = SoundIoFormatU8;
		//set up conversion
	}
	else if (soundio_device_supports_format(parent.device, SoundIoFormatS8)) {
		__this->stream->format = SoundIoFormatS8;
		//set up conversion
	}
	else {
		spdlog::error("Sound: SoundIO Outstream: Setup failed to recognized available stream format.");
		return false;
	}
	spdlog::info("Sound: SoundIO Outstream: Setup format to be default of {}.",
		soundio_format_string(__this->stream->format));
	return true;
}
