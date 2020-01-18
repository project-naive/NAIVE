#include <fstream>
#include <iostream>
#include <soundio/soundio.h>
#include <atomic>
#include <intrin.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <thread>
//#include "../../dep-repo/openal-soft/alc/alu.h"

#include <rigtorp/SPSCQueue.h>
//#include <MI/mimalloc.h>

//#include "SoundBackend.h"

volatile float* buffer = nullptr;
size_t size;
volatile size_t offset = 0;
volatile bool paused = false;
volatile float volume = 1.0;
std::atomic_bool done = false;
SoundIo* soundio;
ALCdevice* aldevice;
ALCint attrs[16];
ALCcontext* alcontext;

volatile size_t underflowcount = 0;

static LPALCLOOPBACKOPENDEVICESOFT alcLoopbackOpenDeviceSOFT;
static LPALCISRENDERFORMATSUPPORTEDSOFT alcIsRenderFormatSupportedSOFT;
static LPALCRENDERSAMPLESSOFT alcRenderSamplesSOFT;

struct buff { float val[48000 * 2]; };

void write_callback(SoundIoOutStream* stream, int frame_count_min, int frame_count_max) {
	double float_sample_rate = stream->sample_rate;
	double second_per_frame = 1.0 / float_sample_rate;
	SoundIoChannelArea* areas = nullptr;
	frame_count_max = 540;
	int frames_left = (frame_count_max < ((size - offset) >> 0x01)) ? frame_count_max : ((size - offset) >> 0x01);
	if (paused) {
		while (frames_left) {
			int frames_count = frames_left;
			soundio_outstream_begin_write(stream, &areas, &frames_count);
			for (int j = 0; j < frames_count; ++j) {
				for (int i = 0; i < 2; ++i) {
					*((float*)areas[i].ptr) = 0.0;
					areas[i].ptr += areas[i].step;
					//					++offset;
				}
			}
			frames_left -= frames_count;
			soundio_outstream_end_write(stream);
		}
		return;
	}
	while (frames_left/* && !paused*/) {
		int frame_count = frames_left;
		soundio_outstream_begin_write(stream, &areas, &frame_count);
		const SoundIoChannelLayout* layout = &stream->layout;
		for (int frame = 0; frame < frame_count && offset < size; frame+= frame_count) {
			/*			for (int channel = 0; channel < layout->channel_count; ++channel) {
							*(float*)areas[channel].ptr = buffer[offset];
							areas[channel].ptr += areas[channel].step;
						}
						++offset;
			*/
			buff temp;
//			while (!temp) { temp = queue.front(); };
//			alcRenderSamplesSOFT(aldevice, (void*)temp, 10000);
			alcRenderSamplesSOFT(aldevice, temp.val, frame_count);
			for (int j = 0; j < frame_count; ++j) {
				for (int i = 0; i < 2; ++i) {
					*((float*)areas[i].ptr) = temp.val[(j << 1) + i] * volume;
					areas[i].ptr += areas[i].step;
//					++offset;
				}
			}

//			queue.pop();
		}
		frames_left -= frame_count;
		soundio_outstream_end_write(stream);
	}
	/*
	if (paused) {
		while (frames_left) {
			int frames_count = frames_left;
			soundio_outstream_begin_write(stream, &areas, &frames_count);
			for (int j = 0; j < frames_count; ++j) {
				for (int i = 0; i < 2; ++i) {
					*((float*)areas[i].ptr) = 0.0;
					areas[i].ptr += areas[i].step;
					//					++offset;
				}
			}
			frames_left -= frames_count;
			soundio_outstream_end_write(stream);
		}
	}
	*/
	if (offset == size) {
		done.store(true, std::memory_order_release);
//		soundio_wakeup(soundio);
	}
}

void error_callback(SoundIoOutStream* stream, int err) {
	std::cerr << "SoundIO Error: " << soundio_strerror(err);
}

void underflow_callback(SoundIoOutStream* stream) {
	++underflowcount;
//	std::clog << "Underflow\n";
}

int main() {
//	system("set ALSOFT_LOGLEVEL=3");
//	std::this_thread::sleep_for(std::chrono::seconds(1));
//	mi_stats_reset();
#define LOAD_PROC(x, y)  ((x) = (y)alcGetProcAddress(NULL, #x))
	LOAD_PROC(alcLoopbackOpenDeviceSOFT, LPALCLOOPBACKOPENDEVICESOFT);
	LOAD_PROC(alcIsRenderFormatSupportedSOFT, LPALCISRENDERFORMATSUPPORTEDSOFT);
	LOAD_PROC(alcRenderSamplesSOFT, LPALCRENDERSAMPLESSOFT);
#undef LOAD_PROC
//	mi_collect(true);
//	mi_stats_print(stdout);
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
	std::clog << "Backend: " << soundio_backend_name(soundio->current_backend) << '\n';
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
	double latency;
	if (!soundio_outstream_get_latency(outstream, &latency))
		std::clog << "Cannot determine latency.";
	std::cout << latency << '\n';
	aldevice = alcLoopbackOpenDeviceSOFT(nullptr);
	attrs[0] = ALC_FORMAT_CHANNELS_SOFT;
	attrs[1] = ALC_STEREO_SOFT;
	attrs[2] = ALC_FORMAT_TYPE_SOFT;
	attrs[3] = ALC_FLOAT_SOFT;
	attrs[4] = ALC_FREQUENCY;
	attrs[5] = 48000;
	attrs[6] = 0;
	alcontext = alcCreateContext(aldevice, attrs);
	alcMakeContextCurrent(alcontext);
	err = alGetError();
	ALuint source[2]{}; ALuint buffero;
	alGenBuffers(1, &buffero);
	alBufferData(buffero, AL_FORMAT_STEREO_FLOAT32, (void*)buffer, size * 4, 48000);
	free((void*)buffer);
	alGenSources(2, source);
	alSourcei(source[0], AL_BUFFER, buffero);
	alSourcei(source[1], AL_BUFFER, buffero);
	alSourcei(source[0], AL_LOOPING, AL_TRUE);
	alSourcePlay(source[0]);
//	std::thread t(thread_func);
	soundio_outstream_start(outstream);
	while (true) {
		while (!done.load(std::memory_order_relaxed)) {
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
			case ' ':
				paused = !paused;
				break;
			case 'd':
				done = true;
				break;
			default:
				break;
			}
			printf("%d", underflowcount);
		}
		done.store(false, std::memory_order_relaxed);
		offset = 0;
		soundio_wakeup(soundio);
//		break;
	}
	alDeleteBuffers(1, &buffero);
	return 0;
}
