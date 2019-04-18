#include "WebMPlayer.h"

#include <fstream>
#include <algorithm>

#include <vpx/vp8dx.h>

//Two buffers to work on
bool file_stream_thread(char** buffer, std::atomic_flag* use_state, std::atomic<bool>* load_state, 
	std::atomic<bool>* done, uint32_t* buff_size, uint32_t* frame_num,
	short* width, short* height, uint32_t* fourcc,
	std::atomic<bool>* all_done) {
	std::ifstream ifs;
	ifs.open("1.ivf", std::ios::binary);
	ifs.seekg(0, std::ios::end);
	int64_t size = ifs.tellg();
	ifs.seekg(8);
	ifs.read(reinterpret_cast<char*>(fourcc), 4);
	ifs.read(reinterpret_cast<char*>(width), 2);
	ifs.read(reinterpret_cast<char*>(height), 2);
	buffer[0] = new char[(*width)*(*height)]{};
	buffer[1] = new char[(*width)*(*height)]{};
	int frame_num0 = 0;
	int frame_num1 = 0;
	ifs.seekg(32);
	while (ifs && size > 0) {
		if (size <= 0) break;
		uint64_t offset = 0;
		while (use_state[0].test_and_set()) _mm_pause();
		while (true) {
			if (size <= 0) break;
			frame_num[0] = 0;
			uint32_t val = 0;
			ifs.read(reinterpret_cast<char*>(&val), 4);
			if (val > size) { ifs.seekg(-4, std::ios::cur); break; }
			ifs.seekg(8, std::ios::cur);
			size -= 12;
			ifs.read(buffer[0] + offset, val);
			size -= val;
			buff_size[0]+= val;
			frame_num[0]++;
			offset += val;
		}
		load_state[0] = true;
		use_state[0].clear();
		offset = 0;
		frame_num = 0;
		if (size <= 0) break;
		uint64_t offset = 0;
		while (use_state[1].test_and_set()) _mm_pause();
		while (true) {
			if (size <= 0) break;
			frame_num[1] = 0;
			uint32_t val = 0;
			ifs.read(reinterpret_cast<char*>(&val), 4);
			if (val > size) { ifs.seekg(-4, std::ios::cur); break; }
			ifs.seekg(8, std::ios::cur);
			size -= 12;
			ifs.read(buffer[1] + offset, val);
			size -= val;
			buff_size[1] += val;
			frame_num[1]++;
			offset += val;
		}
		load_state[1] = true;
		use_state[1].clear();
	}
	*done = true;
	while (!*all_done) _mm_pause();
	delete[] buffer[0];
	delete[] buffer[1];
	return false;
}

bool decode_thread(char** buffer, std::atomic_flag* use_state, std::atomic<bool>* load_state,
	std::atomic<bool>* done, uint32_t* buff_size, uint32_t* frame_num,
	short* width, short* height, uint32_t* fourcc,
	std::atomic<bool>* all_done, std::atomic<bool>* decode_done,
	std::atomic<bool>* buff_decode, GLuint** pixelbuffers,
	int* wy, int* wuv, int* hy, int* huv) {
	vpx_codec_ctx_t ctx;
	const vpx_codec_iface_t* decoder;
	vpx_codec_iter_t iter = nullptr;
	vpx_image* img = nullptr;
	while (!load_state[0]) _mm_pause();
#define VP8_FOURCC 0x30385056
#define VP9_FOURCC 0x30395056
	if (*fourcc == VP8_FOURCC) decoder = vpx_codec_vp8_dx();
	else if (*fourcc == VP9_FOURCC) decoder = vpx_codec_vp9_dx();
	else return false;
	vpx_codec_dec_init(&ctx, decoder, NULL, 0);
	while (!*done) {
		while (!load_state[0]) _mm_pause();
		while (use_state[0].test_and_set()) _mm_pause();
		size_t frame = 0;
		vpx_codec_decode(&ctx, reinterpret_cast<const uint8_t*>(buffer[0]), buff_size[0], nullptr,0);
		int buffer_offset[2][3]{};
		img = vpx_codec_get_frame(&ctx, &iter);
		*wy = (img->d_w) 
			* ((img->fmt & VPX_IMG_FMT_HIGHBITDEPTH) ? 2 : 1);
		*wuv = ((img->d_w + 1) >> img->x_chroma_shift) 
			* ((img->fmt & VPX_IMG_FMT_HIGHBITDEPTH) ? 2 : 1);
		*hy = img->d_h;
		*huv = (img->d_h + 1) >> img->y_chroma_shift;
		do {
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelbuffers[0][0]);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, (*wy)*(*hy)*frame_num[0], nullptr, GL_STREAM_DRAW);
			const unsigned char *buf = img->planes[0];
			const int stride = img->stride[0];
			for (int y = 0; y < *hy; ++y) {
				memcpy(
					glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, buffer_offset[0][0], *wy,
						GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT),
					buf, *wy);
				buffer_offset[0][0] += *wy;
				buf += stride;
			}
			for (int plane = 1; plane < 3; ++plane) {
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelbuffers[0][plane]);
				glBufferData(GL_PIXEL_UNPACK_BUFFER, (*wuv)*(*huv)*frame_num[0], nullptr, GL_STREAM_DRAW);
				const unsigned char *buf = img->planes[plane];
				const int stride = img->stride[plane];
				for (int y = 0; y < *huv; ++y) {
					memcpy(
						glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, buffer_offset[0][plane], *wuv,
							GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT),
						buf, *wuv);
					buffer_offset[0][plane] += *wuv;
					buf += stride;
				}
			}
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		} while ((img = vpx_codec_get_frame(&ctx, &iter)) != NULL);
		buff_decode[0] = true;
		load_state[0] = false;
		if (*done) break;
		while (!load_state[1]) _mm_pause();
		while (use_state[1].test_and_set()) _mm_pause();
		frame = 0;
		vpx_codec_decode(&ctx, reinterpret_cast<const uint8_t*>(buffer[1]), buff_size[1], nullptr, 0);
		while ((img = vpx_codec_get_frame(&ctx, &iter)) != NULL) {
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelbuffers[1][0]);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, (*wy)*(*hy)*frame_num[1], nullptr, GL_STREAM_DRAW);
			const unsigned char *buf = img->planes[0];
			const int stride = img->stride[0];
			for (int y = 0; y < *hy; ++y) {
				memcpy(
					glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, buffer_offset[1][0], *wy,
						GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT),
					buf, *wy);
				buffer_offset[1][0] += *wy;
				buf += stride;
			}
			for (int plane = 1; plane < 3; ++plane) {
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelbuffers[1][plane]);
				glBufferData(GL_PIXEL_UNPACK_BUFFER, (*wuv)*(*huv)*frame_num[1], nullptr, GL_STREAM_DRAW);
				const unsigned char *buf = img->planes[plane];
				const int stride = img->stride[plane];
				for (int y = 0; y < *huv; ++y) {
					memcpy(
						glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, buffer_offset[1][plane], *wuv,
							GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT),
						buf, *wuv);
					buffer_offset[1][plane] += *wuv;
					buf += stride;
				}
			}
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		};
		buff_decode[1] = true;
		load_state[1] = false;
	}
	*decode_done = true;
}

SimplePlay::SimplePlay(Engine::Core::GlobalManagers & given_managers):
	Engine::Core::State(given_managers), thread_pool(*(new ThreadPool(3))) {
	glGenBuffers(6, pixelbuffers[0]);
	thread_pool.ThreadPush(std::bind(file_stream_thread, buffer, use_state, load_state,
		&done, buff_size, frame_num, &width, &height, &fourcc, &all_done), 0);
	GLuint** ptr = *&pixelbuffers;
	std::function<bool()> a = std::bind(decode_thread, buffer, use_state, load_state,
		&done, buff_size, frame_num, &width, &height, &fourcc, &all_done,
		&decode_done, buff_decode, pixelbuffers, &wy, &wuv, &hy, &huv);
	tex_model = new State_2048::Models::TextureQuad(
		Managers.ShaderManager,
		Managers.ShaderManager.addGeneric(
			*(new State_2048::Shaders::TextureQuad(Managers.ShaderManager))));
	glGenTextures(2, textures);
}

SimplePlay::~SimplePlay() {
	delete &thread_pool;
}

void SimplePlay::Loop() {
	while (running) {
		glfwWaitEvents();
	}
}

void SimplePlay::Refresh(size_t ID) {

}

WebMPlayer::WebMPlayer(Engine::Graphics::WindowInfo & info)
	:Engine::Core::Game(info) {

}

WebMPlayer::~WebMPlayer() {
}

void WebMPlayer::MainLoop() {
	state->Loop();
}
