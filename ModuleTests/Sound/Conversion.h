#pragma once

#include <cstdint>
#include <cassert>
#include <soundio/soundio.h>
#include <vector>

typedef void(*SoundFormatConvertFunc)(void* src, void* dest, int num);
#define SoundIoFormatLast SoundIoFormatFloat64BE + 1

extern SoundFormatConvertFunc Funcs[SoundIoFormatLast][SoundIoFormatLast];

//currently only implemented c functions for converting to float of native endianess
void InitConversionFuncs();

struct mixer_node {
public:
	//may require more memory than necessary
	//due to duplicate size and position storage
	struct Connection {
		SoundIoRingBuffer* buffer;
		size_t buffer_size;
		mixer_node* peer;
	};
	std::vector<Connection> parents;
	std::vector<Connection> children;
	size_t ref_count = 0;
	struct stream_map {
		struct channel_map {
			float weight = 1.0;
			unsigned delay_frame = 0;
		};
		int in_channels = 2;
		int out_channels = 2;
		int in_rate;
		int out_rate;
		float weight = 1.0;
		channel_map** map;
//		void(*mix_func)(channel_map**, float weight, unsigned delay_frame, void* in_buffer, void* out_buffer, int num_frame);
		//Delay not currently implemented
		static void simple_mix_func(const mixer_node& node, size_t in_index, size_t out_index) {
			size_t in_offset = 0;
			size_t out_offset = 0;
			//Can be optimized with simd
			for (int frame = 0; frame < node.frame_count; ++frame) {
				float* write_ptr = (float*)soundio_ring_buffer_write_ptr(node.children[out_index].buffer);
				float* read_ptr = (float*)soundio_ring_buffer_read_ptr(node.parents[in_index].buffer);
				for (int i = 0; i < node.matrix[in_index][out_index].in_channels; ++i) {
					for (int j = 0; j < node.matrix[in_index][out_index].out_channels; ++j) {
						write_ptr[j] +=
							read_ptr[i] * (node.matrix[in_index][out_index].map[i][j].weight)*node.matrix[in_index][out_index].weight;
					}
				}
				soundio_ring_buffer_advance_read_ptr(node.children[out_index].buffer, node.matrix[in_index][out_index].in_channels * 4);
				soundio_ring_buffer_advance_write_ptr(node.children[out_index].buffer, node.matrix[in_index][out_index].out_channels * 4);
			}
		}
		void(*mix_func)(const mixer_node& node, size_t in_index, size_t out_index) = simple_mix_func;
	};
	stream_map** matrix;
	bool mix(int num_frames) {
//		for (int i = 0; i < num_parent; ++i) {
//			if (!in_buffers[i].ready)
//				return false;
//		}
		size_t num_parent = parents.size();
		size_t num_child = children.size();
		for (int i = 0; i < num_parent; ++i) {
			for (int j = 0; j < num_child; ++j) {
				matrix[i][j].mix_func(*this, i, j);
			}
		}
	}
	void reserve_output(size_t num_out) {
		children.reserve(num_out);
	}
	//nullptr of in means raw output handled elsewhere
	void register_output(SoundIoRingBuffer* buffer, mixer_node* out = nullptr) {
		children.emplace_back(Connection{ buffer,soundio_ring_buffer_capacity(buffer),out });
	}
	void reserve_input(int num_in) {
		parents.reserve(num_in);
	}
	//nullptr of in means raw input handled elsewhere
	void register_input(SoundIoRingBuffer* buffer, mixer_node* in = nullptr) {
		parents.emplace_back(Connection{ buffer,soundio_ring_buffer_capacity(buffer),in });
	}
	int frame_count;
	//for scheduling execution
	int min_layer_abs;
};

//squentially execute mixing
//currently do not support echoing effect
//(using a looped structure with delay)
class mixer_sync {
	std::vector<mixer_node::Connection> inputs;
	std::vector<mixer_node::Connection> outputs;
	std::vector<mixer_node*>  mixer_sequence;
	void reserve_input(size_t num) {
		inputs.reserve(num);
	}
	void reserve_output(size_t num) {
		outputs.reserve(num);
	}
	void register_input(const mixer_node::Connection&  in) {
		inputs.emplace_back(in);
	}
	void register_output(const mixer_node::Connection& out) {
		outputs.emplace_back(out);
	}
	void init_mix_order() {

	}
	void mix(double microseconds) {

	}

};
