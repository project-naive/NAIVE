#include <fstream>
#include <iostream>
#include <soundio/soundio.h>
#include <atomic>
#include <intrin.h>

//#include "SoundBackend.h"

volatile float* buffer = nullptr;
size_t size;
volatile size_t offset = 0;
volatile bool paused = false;
volatile float volume = 1.0;
std::atomic_bool done = false;
SoundIo* soundio;

void write_callback(SoundIoOutStream* stream, int frame_count_min, int frame_count_max) {
	double float_sample_rate = stream->sample_rate;
	double second_per_frame = 1.0 / float_sample_rate;
	SoundIoChannelArea* areas;
	frame_count_max = 480;
	int frames_left = (frame_count_max < ((size - offset) >> 0x01)) ? frame_count_max : ((size - offset) >> 0x01);
	while (frames_left&& !paused) {
		int frame_count = frames_left;
		soundio_outstream_begin_write(stream, &areas, &frame_count);
		const SoundIoChannelLayout* layout = &stream->layout;
		for (int frame = 0; frame < frame_count && offset < size; ++frame) {
/*			for (int channel = 0; channel < layout->channel_count; ++channel) {
				*(float*)areas[channel].ptr = buffer[offset];
				areas[channel].ptr += areas[channel].step;
			}
			++offset;
*/
			for (int i = 0; i < 2; ++i) {
				*(float*)areas[i].ptr = buffer[offset] * volume;
				areas[i].ptr += areas[i].step;
				++offset;
			}
		}
		frames_left -= frame_count;
		soundio_outstream_end_write(stream);
	}
	if (offset == size) { 
		done.store(true, std::memory_order_release);
		soundio_wakeup(soundio);
	}
}

void error_callback(SoundIoOutStream* stream, int err) {
	std::cerr << "SoundIO Error: " << soundio_strerror(err);
}

void underflow_callback(SoundIoOutStream* stream) {

}

int main() {
//	SoundIOBackendInitInfo info{};
//	SoundIO b(info);
	{
		std::ifstream ifs;
		ifs.open("out.pcm");
		ifs.seekg(0, std::ios::end);
		size = ifs.tellg();
		ifs.seekg(0);
		buffer = (float*)malloc(size);
		ifs.read((char*)buffer, size);
		ifs.close();
		size >>= 2;
	}
	int err = 0;
	soundio = soundio_create();
	err = soundio_connect(soundio);
	std::clog << "Backend: " << soundio_backend_name(soundio->current_backend) <<'\n';
	soundio_flush_events(soundio);
	int device_index = soundio_default_output_device_index(soundio);
	SoundIoDevice* device = soundio_get_output_device(soundio, device_index);
	std::clog << "Output device: " << device->name << '\n';
	SoundIoOutStream* outstream = soundio_outstream_create(device);
	outstream->write_callback = write_callback;
	outstream->error_callback = error_callback;
	outstream->underflow_callback = underflow_callback;
	outstream->sample_rate = 0;
	outstream->software_latency = 0;
	outstream->name = nullptr;
	outstream->format = SoundIoFormatFloat32NE;
	err = soundio_outstream_open(outstream);
	soundio_flush_events(soundio);
	err = soundio_outstream_start(outstream);
	double latency;
	if (!soundio_outstream_get_latency(outstream, &latency))
		std::clog << "Cannot determine latency.";
	std::cout << latency << '\n';
	while (true) {
		while (!done.load(std::memory_order_acquire)) {
			soundio_flush_events(soundio);
			char c = getc(stdin);
			switch (c) {
				case 'p':
					paused = true;
					break;
				case 'c':
					paused = false;
					break;
				case 'i':
					volume /= 2.0;
					break;
				case 'o':
					volume *= 2.0;
					break;
				default:
					break;
			}
		}
		done.store(false, std::memory_order_relaxed);
		offset = 0;
		soundio_wakeup(soundio);
	}
	return 0;
}
