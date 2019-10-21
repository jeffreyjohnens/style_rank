#ifndef STYLE_RANK_FEATURES_H
#define STYLE_RANK_FEATURES_H

#include "utils.hpp"
#include "pcd.hpp"
#include <cmath>

#include "libpopcnt.h"
#include <assert.h>

using namespace std;

int lcm(int a, int b) {
  int max = (a > b) ? a : b;
  while(true) {
    if (max % a == 0 && max % b == 0) {
      break;
    }
    else
      ++max;
  }
  return max;
}

int rough_quantize(int x, int ticks) {
  return (int)round((double)x / ticks * 8);
}

unique_ptr<DISCRETE_DIST> IntervalDist(Piece *p) {
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    for (int j=0; j<chord.notes.size(); j++) {
      for (int k=j+1; k<(int)chord.notes.size(); k++) {
        (*d)[mod(chord.notes[k]->pitch - chord.notes[j]->pitch, 12)] += chord.duration;
      }
    }
  }
  return d;
}

unique_ptr<DISCRETE_DIST> IntervalClassDist(Piece *p) {
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    for (int j=0; j<(int)chord.notes.size(); j++) {
      for (int k=j+1; k<(int)chord.notes.size(); k++) {
        (*d)[interval_class[mod(chord.notes[k]->pitch - chord.notes[j]->pitch, 12)]] += chord.duration;
      }
    }
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordSize(Piece *p) {
  /*
  The number of pitches in a chord.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    (*d)[chord.notes.size()]++;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordPCSizeRatio(Piece *p) {
  /*
  The ratio of distinct pitch classes to number of pitches in a chord.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    int pc[12] = {0};
    int pccount = 0;
    for (const auto &note : chord.notes) {
      if (pc[mod(note->pitch,12)] == 0) {
        pc[mod(note->pitch,12)] = 1;
        pccount += 1;
      }
    }
    (*d)[NOMINAL_TUPLE(pccount, chord.notes.size()).value]++;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordOnsetRatio(Piece *p) {
  /*
  The ratio of onsets to number of pitches in a chord.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    int onset = 0;
    for (int i=0; i<(int)chord.notes.size(); i++) {
      if (chord.notes[i]->onset == chord.onset)
        onset++;
    }
    (*d)[NOMINAL_TUPLE(onset, chord.notes.size()).value]++;
  }
  return d;
}

// should these be logged first?
unique_ptr<DISCRETE_DIST> ChordDistinctDurationRatio(Piece *p) {
  /*
  The ratio of distinct durations to the number of pitches in a chord.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    map<int,int> durations;
    for (const auto &note : chord.notes) {
      durations[note->onset + note->duration - chord.onset] = 1;
    }
    (*d)[NOMINAL_TUPLE(durations.size(), chord.notes.size()).value]++;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordDuration(Piece *p) {
  /*
  The duration of a chord.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    (*d)[rough_quantize(chord.duration, p->ticks)]++;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordShape(Piece *p) {
  /*
  The pitches in a chord represented as bits in an integer, where the lowest pitch corresponding to the LSB.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    int m = chord.notes[0]->pitch;
    uint64_t shape = 0;
    for (const auto &note : chord.notes) {
      if ((note->pitch - m) < 64)
        shape |= (1 << (note->pitch - m));
    }
    (*d)[shape] += chord.duration;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordOnsetShape(Piece *p) {
  /*
  The onset pitches in a chord represeted as bits in an integer, where the lowest pitch corresponding to the LSB
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    int m = chord.onset_notes[0]->pitch;
    uint64_t shape = 0;
    for (const auto &note : chord.onset_notes) {
      if ((note->pitch - m) < 64) {
        shape |= (1 << (note->pitch - m));
      }
    }
    (*d)[shape] += chord.duration;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordPCD(Piece *p) {
  /*
  The distinct pitch class set of notes represented as bits in an integer.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    (*d)[pcd[PCINT(chord.notes).value]] += chord.duration;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordPCDWBass(Piece *p) {
  /*
  The distinct pitch class set of notes represented as bits in an integer. W bass
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    (*d)[mod(chord.notes.front()->pitch,12) + (pcd[PCINT(chord.notes).value] << 12)] += chord.duration;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordOnsetPCD(Piece *p) {
  /*
  The distinct pitch class set of onset notes represented as bits in an integer.
  */
 auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
 for (const auto &chord : p->chords) {
   (*d)[pcd[PCINT(chord.onset_notes).value]] += chord.duration;
 }
 return d;
}

unique_ptr<DISCRETE_DIST> ChordOnsetTiePCD(Piece *p) {
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    (*d)[pcd[PCINT(chord.onset_notes).value] + (pcd[PCINT(chord.tie_notes).value] << 12)] += chord.duration;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordOnsetTiePCDTogether(Piece *p) {
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    // get the number of rotations
    int r = rot[PCINT(chord.notes).value];
    int onsets = 0;
    int ties = 0;
    for (const auto &note : chord.onset_notes) {
      onsets |= (1 << mod(note->pitch + r, 12));
    }
    for (const auto &note : chord.tie_notes) {
      ties |= (1 << mod(note->pitch + r, 12));
    }
    (*d)[onsets + (ties << 12)] += chord.duration;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordTonnetz(Piece *p) {
  /*
  The distinct pitch class represented as bits in an integer.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    (*d)[tonnetz[PCINT(chord.notes).value]] += chord.duration;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordOnset(Piece *p) {
  /*
  Bits representing which notes in a chord are onsets in ascending order. The lowest pitch is the LSB.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    uint64_t onset = 0;
    for (int i=0; i<(int)chord.notes.size(); i++) {
      if (chord.notes[i]->onset == chord.onset)
        onset |= (1 << i);
    }
    onset |= (1 << chord.notes.size());
    (*d)[onset]++;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordRange(Piece *p) {
  /*
  The pitch range of notes in a chord.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    auto out = minmax_element(
    chord.notes.begin(), chord.notes.end(), [](NOTE *a, NOTE *b){return a->pitch < b->pitch;});
    (*d)[(*get<1>(out))->pitch - (*get<0>(out))->pitch]++;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordDissonance(Piece *p) {
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    if (chord.onset_notes.size() >= 2) {
      double periodicity = 0;
      for (const auto &i : chord.onset_notes) {
        int den_lcm = 1;
        double min_frac = 1;
        for (const auto &j : chord.onset_notes) {
          int ii = j->pitch - i->pitch + 128;
          double frac = (double)dissfracnum[ii] / dissfracden[ii];
          if (frac < min_frac) {
            min_frac = frac;
          }
          den_lcm = lcm(den_lcm, dissfracden[ii]);
        }
        periodicity += min_frac * den_lcm;
      }
      int p = (int)(periodicity / chord.onset_notes.size());
      (*d)[p] += chord.duration;
    }
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordTranDissonance(Piece *p) {
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (int k=0; k<(int)p->chords.size()-1; k++) {
    if ((p->chords[k].notes.size() >= 2) && (p->chords[k+1].notes.size() >= 2)) {
      double periodicity = 0;
      for (const auto &i : p->chords[k].notes) {
        int den_lcm = 1;
        double min_frac = 1;
        for (const auto &j : p->chords[k+1].notes) {
          int ii = j->pitch - i->pitch + 128;
          double frac = (double)dissfracnum[ii] / dissfracden[ii];
          if (frac < min_frac) {
            min_frac = frac;
          }
          den_lcm = lcm(den_lcm, dissfracden[ii]);
        }
        periodicity += min_frac * den_lcm;
      }
      int per = (int)(periodicity / p->chords[k].notes.size());
      (*d)[per]++; //= p->chords[k].duration;
    }
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordLowestInterval(Piece *p) {
  /*
  The interval between the lowest two pitches in a chord.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (const auto &chord : p->chords) {
    if (chord.notes.size() > 1) {
      (*d)[chord.notes[1]->pitch - chord.notes[0]->pitch]++;
    }
  }
  return d;
}

// 3 does not work very well
unique_ptr<DISCRETE_DIST> ChordSizeNgram(Piece *p) {
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (int i=0; i<(int)p->chords.size() - 2; i++) {
    (*d)[NOMINAL_TUPLE(p->chords[i].notes.size(), p->chords[i+1].notes.size(), p->chords[i+2].notes.size()).value]++; 
  }
  return d;
}

enum class VOICE_MOTION_TYPE : uint64_t {
  NO_CHANGE,
  OBLIQUE_MOTION,
  PARALLEL_MOTION,
  CONTRARY_MOTION,
};

unique_ptr<DISCRETE_DIST> ChordTranVoiceMotion(Piece *p) {
  /*
  The outer voice motion between two successive chords.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (int i=0; i<(int)p->chords.size()-1; i++) {
    int md = sgn(p->chords[i+1].notes.back()->pitch - p->chords[i].notes.back()->pitch);
    int bd = sgn(p->chords[i+1].notes[0]->pitch - p->chords[i].notes[0]->pitch);
    if (abs(md) + abs(bd) == 0) {
      (*d)[static_cast<uint64_t>(VOICE_MOTION_TYPE::NO_CHANGE)]++;
    }
    else if (abs(md) + abs(bd) == 1) {
      (*d)[static_cast<uint64_t>(VOICE_MOTION_TYPE::OBLIQUE_MOTION)]++;
    }
    else if (abs(md) + abs(bd) == 2) {
      if (md == bd) {
        (*d)[static_cast<uint64_t>(VOICE_MOTION_TYPE::PARALLEL_MOTION)]++;
      }
      else if (md != bd) {
        (*d)[static_cast<uint64_t>(VOICE_MOTION_TYPE::CONTRARY_MOTION)]++;
      } 
    }
  }
  return d;
}

// THIS IS REALLY BAD
unique_ptr<DISCRETE_DIST> ChordTranRepeat(Piece *p) {
  /*
  The frequency of complete chord repetition.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (int i=0; i<(int)p->chords.size()-1; i++) {
    bool allOnsets = true;
    for (const auto &note : p->chords[i+1].notes) {
      if (note->onset < p->chords[i+1].onset) {
        allOnsets = false;
      }
    }
    if ((allOnsets) && (p->chords[i].notes.size() == p->chords[i+1].notes.size())) {
      bool allMatch = true;
      for (int j=0; j<p->chords[i].notes.size(); j++) {
        if (p->chords[i].notes[j]->pitch != p->chords[i+1].notes[j]->pitch) {
          allMatch = false;
        }
      }
      (*d)[(int)allMatch]++;
    }
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordTranScaleDistance(Piece *p) {
  /*
  The distance in scale space between two successive chords.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (int i=0; i<(int)p->chords.size()-1; i++) {
    int a = PCINT(p->chords[i].notes).value;
    int b = PCINT(p->chords[i+1].notes).value;
    if (a == b) {
      (*d)[100]++;
    }
    else {
      uint64_t data = (pcscale[a] ^ pcscale[b]);
      (*d)[popcnt(&data, sizeof(uint64_t))]++;
      //assert(popcnt(&data, sizeof(uint64_t)) == __builtin_popcount(pcscale[a] ^ pcscale[b])); 
      //(*d)[__builtin_popcount(pcscale[a] ^ pcscale[b])]++;
    }
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordTranScaleUnion(Piece *p) {
  /*
  The distance in scale space between two successive chords.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (int i=0; i<(int)p->chords.size()-1; i++) {
    int a = PCINT(p->chords[i].notes).value;
    int b = PCINT(p->chords[i+1].notes).value;
    if (a == b) {
      (*d)[100]++;
    }
    else {
      uint64_t data = (pcscale[a] | pcscale[b]);
      (*d)[popcnt(&data, sizeof(uint64_t))]++;
      //assert(popcnt(&data, sizeof(uint64_t)) == __builtin_popcount(pcscale[a] | pcscale[b]));
      //(*d)[__builtin_popcount(pcscale[a] | pcscale[b])]++;
    }
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordTranDistance(Piece *p) {
  /*
  The distance between the highest and lowest notes in successive chords
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (int i=0; i<(int)p->chords.size()-1; i++) {
    int md = abs(p->chords[i+1].notes.back()->pitch - p->chords[i].notes.back()->pitch);
    int bd = abs(p->chords[i+1].notes[0]->pitch - p->chords[i].notes[0]->pitch);
    (*d)[md + bd]++;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordTranOuter(Piece *p) {
  /*
  The pitch class transition using only the outer notes.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (int i=0; i<(int)p->chords.size()-1; i++) {
    if ((p->chords[i+1].notes.front()->onset == p->chords[i+1].onset) || (p->chords[i+1].notes.back()->onset == p->chords[i+1].onset)) {
      int a = mod(p->chords[i].notes.back()->pitch - p->chords[i].notes.front()->pitch, 12);
      int b = mod(p->chords[i+1].notes.back()->pitch - p->chords[i+1].notes.front()->pitch, 12);
      int c = mod(p->chords[i+1].notes.front()->pitch - p->chords[i].notes.front()->pitch, 12);
      (*d)[NOMINAL_TUPLE(a,b,c).value]++;
    }
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordTranBassInterval(Piece *p) {
  /*
  The absolute interval between the lowest note in successive chords.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  vector<int> bass;
  for (const auto &chord : p->chords) {
    if (chord.notes.front()->onset == chord.onset) {
      bass.push_back(chord.notes.front()->pitch);
    }
  }
  for (int i=0; i<(int)bass.size() - 2; i++) {
    (*d)[mod(bass[i+1] - bass[i], 12)]++;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordTranMelodyInterval(Piece *p) {
  /*
  The absolute interval between the highest notes in successive chords.
  */
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  vector<int> melody;
  for (const auto &chord : p->chords) {
    if (chord.notes.back()->onset == chord.onset) {
      melody.push_back(chord.notes.back()->pitch);
    }
  }

  for (int i=0; i<(int)melody.size() - 5; i++) {
    (*d)[pcd[PCINT(melody.begin() + i, melody.begin() + i + 5).value]]++;
  }
  return d;
}

unique_ptr<DISCRETE_DIST> ChordMelodyNgram(Piece *p) {
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  vector<int> melody;
  for (const auto &chord : p->chords) {
    if (chord.notes.back()->onset == chord.onset) {
      melody.push_back(chord.notes.back()->pitch);
    }
  }
  for (int i=0; i<(int)melody.size() - 4; i++) {
    (*d)[NOMINAL_TUPLE(mod(melody[i] - melody[i+1], 12), mod(melody[i+1] - melody[i+2], 12), mod(melody[i+2] - melody[i+3], 12)).value]++;
  }
  return d;
}

uint64_t roll_to_min(uint64_t x, int n) {
  uint64_t min = x;
  for (int i=0; i<n; i++) {
    uint64_t tmp = (x >> i) + (x << (n-i)) & ((1<<n)-1);
    if (tmp < min) {
      min = tmp;
    }
  }
  return min;
}

unique_ptr<DISCRETE_DIST> PCDTran(Piece *p) {
  auto d = unique_ptr<DISCRETE_DIST>{new DISCRETE_DIST};
  for (int i=0; i<(int)p->unique_onsets.size() - 2; i++) {
    auto c1 = p->findOverlapping(p->unique_onsets[i], p->unique_onsets[i+1]);
    auto c2 = p->findOverlapping(p->unique_onsets[i+1], p->unique_onsets[i+2]);
    (*d)[ roll_to_min(PCINT(c1).value + (PCINT(c2).value << 12), 24) ]++;
  }
  return d;
}

#endif