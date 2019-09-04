#ifndef STYLE_RANK_PARSE_H
#define STYLE_RANK_PARSE_H

#include <iostream>
#include <assert.h>
#include <vector>
#include <array>
#include <math.h>
#include <numeric>
#include <map>
#include <set>
#include <stack>

#include "./deps/MidiFile.h"
#include "utils.hpp"

using namespace std;

static const int MAX_CHORD_SIZE = 24;
static const int interval_class[12] = {0,1,2,3,4,5,6,5,4,3,2,1};

int quantize(int x, int ticks_per_beat, int resolution) {
	return (int)round((double)x / ticks_per_beat * resolution);
}

class NOTE {
public:
	int pitch, duration, velocity, onset, end;
	NOTE(int _pitch, int _onset, int _duration, int _velocity) {
		assert(_pitch >= 0);
		assert(_pitch < 128);
		assert(_duration > 0);
		assert(_onset >= 0);
        assert(_velocity >= 0);
		pitch = _pitch;
		duration = _duration;
		velocity = _velocity;
		onset = _onset;
		end = _onset + _duration;
	}
};

class CHORD {
public:
	vector<NOTE*> notes;
	vector<NOTE*> onset_notes;
	vector<NOTE*> tie_notes;
	int duration;
	int onset;
	CHORD(vector<NOTE*> x, int _duration, int _onset) {
		sort(x.begin(), x.end(), [](NOTE *a, NOTE *b){return a->pitch < b->pitch;});
		copy(x.begin(), x.end(), back_inserter(notes));
		copy_if(x.begin(), x.end(), back_inserter(onset_notes), [_onset](NOTE *a){return a->onset == _onset;} );
		copy_if(x.begin(), x.end(), back_inserter(tie_notes), [_onset](NOTE *a){return a->onset != _onset;});
		duration = _duration;
		onset = _onset;
	}
    string __repr__() const {
        string repr = "CHORD [ ";
        for (const auto &note : notes) {
            repr += to_string(note->pitch) + " ";
        }
        return repr + "]\n";
    }
};

class PCINT {
public:
    int value;
    PCINT(vector<int> &x) {
        value = 0;
        for (int i=0; i<(int)x.size(); i++)
            value |= (1 << mod(x[i], 12));
    }
	PCINT(vector<int>::iterator begin, vector<int>::iterator end) {
		value = 0;
        for (auto it = begin; it != end; it++) {
            value |= (1 << mod(*it, 12));
    }
}
    PCINT(const vector<NOTE*> &notes) {
        value = 0;
        for (const auto &note : notes) {
            value |= (1 << mod(note->pitch, 12));
        }
    }
};

class Piece {
public:

	multimap<int,NOTE*> stree;
	multimap<int,NOTE*> etree;

	set<int> unique_onsets_set;
	set<int> unique_offsets_set;
	vector<int> unique_onsets;
	vector<int> unique_offsets;

	vector<CHORD> chords;
	vector<unique_ptr<NOTE>> notes;

	int ticks;
	int track_count;
	int max_duration;

    // this is for for testing
	Piece (vector<array<int,3>> &notes) {
		max_duration = 0;
		ticks = 1;
		for (const auto &note : notes) {
			if (note[2] > max_duration) {
				max_duration = note[2];
			}
			addNote(note[0], note[1], note[2]);
		}
		findChords();
	}

	Piece(string filepath, int resolution=8, bool skip_chords=false) {

		smf::MidiFile midifile;
		QUIET_CALL(midifile.read(filepath));
		midifile.linkNotePairs();
		track_count = midifile.getTrackCount();
		ticks = midifile.getTicksPerQuarterNote();
		max_duration = 0;

		int pitch, duration, velocity, onset;

		for (int track=0; track<track_count; track++) {
			for (int event=0; event<midifile[track].size(); event++) {
				if (midifile[track][event].isNoteOn()) {

					pitch = (int)midifile[track][event][1];
					duration = midifile[track][event].getTickDuration();
					velocity = (int)midifile[track][event][2];
					onset = midifile[track][event].tick;
					assert(onset >= 0);

                    if (duration > max_duration) {
			            max_duration = duration;
		            }

					// quantization
					//duration = quantize(duration, ticks, resolution);
					//onset = quantize(onset, ticks, resolution);

					addNote(pitch, onset, duration, velocity);
				}
			}
		}
		if (!skip_chords) {
			findChords();
		}
	}

	void addNote(int pitch, int onset, int duration, int velocity=100) {
		if (duration <= 0) return;
		notes.push_back( 
            unique_ptr<NOTE>(new NOTE(pitch, onset, duration, velocity)) );

		unique_onsets_set.insert( onset );
		unique_offsets_set.insert( onset + duration );	
	}

	void findChords() {
		if (notes.size() <= 0) return;

		for (const auto &note : notes) {
			stree.insert( make_pair(note->onset, note.get()) );
			etree.insert( make_pair(note->end, note.get()) );
		}

		copy(unique_onsets_set.begin(), unique_onsets_set.end(), back_inserter(unique_onsets));

		copy(unique_offsets_set.begin(), unique_offsets_set.end(), back_inserter(unique_offsets));

        // add the last offset
		unique_onsets.push_back(unique_offsets.back());

		for (int i=0; i<(int)unique_onsets.size() - 1; i++) {
            int duration = unique_onsets[i+1] - unique_onsets[i];
            auto notes = findOverlapping(unique_onsets[i], unique_onsets[i+1]);
			auto chord = CHORD(notes, duration, unique_onsets[i]);
    	    chords.push_back( chord );
		}
	}

	vector<NOTE*> findOverlapping(int s, int e) {
		assert(s < e);
		vector<NOTE*> notevec;
		auto itend = etree.upper_bound(s+max_duration);
	    for (auto it = etree.upper_bound(s); it != itend; it++) {
			if (it->second->onset <= s) {
	    	    notevec.push_back(it->second);
			}
		}
		return notevec;
	}
};

#endif