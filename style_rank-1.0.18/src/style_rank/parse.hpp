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
  NOTE(int pit, int ons, int dur, int vel) {
    assert(pit >= 0);
    assert(pit < 128);
    assert(dur > 0);
    assert(ons >= 0);
    assert(vel >= 0);
    pitch = pit;
    duration = dur;
    velocity = vel;
    onset = ons;
    end = ons + dur;
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
    copy_if(x.begin(), x.end(), back_inserter(onset_notes), [_onset](NOTE *a)   {return a->onset == _onset;} );
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
  PCINT(vector<int>::iterator b, vector<int>::iterator e) {
    value = 0;
    for (auto it = b; it != e; it++) {
      value |= (1 << mod(*it, 12));
    }
  }
  PCINT(vector<unique_ptr<NOTE>>::iterator b, vector<unique_ptr<NOTE>>::iterator e) {
    value = 0;
    for (auto it = b; it != e; it++) {
      value |= (1 << mod((*it)->pitch, 12));
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

  set<int> onsets;
  set<int> onsets_and_offsets;

  vector<CHORD> chords;
  vector<CHORD> chords_w_rests;
  vector<unique_ptr<NOTE>> notes;

  int ticks;
  int track_count;
  int max_duration;
  int r;

  // this is for for testing
  Piece (vector<array<int,3>> &notes, bool include_offsets=false) {
    max_duration = 0;
    ticks = 1;
    for (const auto &note : notes) {
      if (note[2] > max_duration) {
        max_duration = note[2];
      }
      addNote(note[0], note[1], note[2]);
    }
    findChords(include_offsets);
  }

  Piece(string filepath, int resolution=0, bool include_offsets=false, bool skip_chords=false) {

    smf::MidiFile midifile;
    QUIET_CALL(midifile.read(filepath));
    midifile.linkNotePairs();
    track_count = midifile.getTrackCount();
    ticks = midifile.getTicksPerQuarterNote();
    max_duration = 0;
    r = resolution;
    if (r==0) r=ticks;

    int pitch, duration, velocity, onset;

    for (int track=0; track<track_count; track++) {
      for (int event=0; event<midifile[track].size(); event++) {
        if (midifile[track][event].isNoteOn()) {

          pitch = (int)midifile[track][event][1];
          duration = midifile[track][event].getTickDuration();
          velocity = (int)midifile[track][event][2];
          onset = midifile[track][event].tick;
          assert(onset >= 0);

          if (resolution != 0) {
            duration = quantize(duration, ticks, r);
            onset = quantize(onset, ticks, r);
          }

          if (duration > max_duration) {
            max_duration = duration;
          }

          addNote(pitch, onset, duration, velocity);
        }
      }
    }
    if (!skip_chords) {
      findChords(include_offsets);
    }
  }

  void addNote(int pitch, int onset, int duration, int velocity=100) {
    if (duration <= 0) return;

    notes.push_back( 
      unique_ptr<NOTE>(new NOTE(pitch, onset, duration, velocity)));

    onsets.insert( onset );
    onsets_and_offsets.insert( onset );
    onsets_and_offsets.insert( onset + duration );
  }

  void findChords(bool include_offsets) {
    if (notes.size() <= 0) return;

    for (const auto &note : notes) {
      stree.insert( make_pair(note->onset, note.get()) );
      etree.insert( make_pair(note->end, note.get()) );
    }

    vector<int> bounds;
    if (include_offsets) {
      copy(
        onsets_and_offsets.begin(), 
        onsets_and_offsets.end(), 
        back_inserter(bounds));
    }
    else {
      copy(onsets.begin(), onsets.end(), back_inserter(bounds));
      bounds.push_back(*onsets_and_offsets.rbegin());
    }

    for (int i=0; i<(int)bounds.size() - 1; i++) {
      int length = bounds[i+1] - bounds[i];
      auto notes = findOverlapping(bounds[i], bounds[i+1]);
      auto chord = CHORD(notes, length, bounds[i]);
      if (!notes.empty()) {
        chords.push_back( chord );
      }
      else {
        chords_w_rests.push_back( chord );
      }
    }
  }

  // this is a faster way to find the notes belonging to
  // a segment using the red-black trees
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