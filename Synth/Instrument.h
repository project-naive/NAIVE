#pragma once

class SoundSample {

};

//The class for a class of sound, typically with the same timbre
class Instrument {
	SoundSample sample;
	struct NoteInfo {
		
	};
	int RenderNote(int min_samples, int max_samples, NoteInfo& info, int timeout);
};

