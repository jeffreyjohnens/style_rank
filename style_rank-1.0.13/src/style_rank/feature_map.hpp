#ifndef STYLE_RANK_FEATURE_MAP_H
#define STYLE_RANK_FEATURE_MAP_H

#include "utils.hpp"

#include <unordered_map>
#include <string>

static std::unordered_map<std::string, std::unique_ptr<DISCRETE_DIST>(*)(Piece*)> m { 
  { "IntervalDist", &IntervalDist},
	{ "IntervalClassDist", &IntervalClassDist},
	{ "ChordSize", &ChordSize},
	{ "ChordPCSizeRatio", &ChordPCSizeRatio},
	{ "ChordOnsetRatio", &ChordOnsetRatio},
	{ "ChordDistinctDurationRatio", &ChordDistinctDurationRatio},
	{ "ChordDuration", &ChordDuration},
	{ "ChordShape", &ChordShape},
	{ "ChordOnsetShape", &ChordOnsetShape},
	{ "ChordPCD", &ChordPCD},
	{ "ChordPCDWBass", &ChordPCDWBass},
	{ "ChordOnsetPCD", &ChordOnsetPCD},
	{ "ChordOnsetTiePCD", &ChordOnsetTiePCD},
	{ "ChordOnsetTiePCDTogether", &ChordOnsetTiePCDTogether},
	{ "ChordTonnetz", &ChordTonnetz},
	{ "ChordOnset", &ChordOnset},
	{ "ChordRange", &ChordRange},
	{ "ChordDissonance", &ChordDissonance},
	{ "ChordTranDissonance", &ChordTranDissonance},
	{ "ChordLowestInterval", &ChordLowestInterval},
	{ "ChordSizeNgram", &ChordSizeNgram},
	{ "ChordTranVoiceMotion", &ChordTranVoiceMotion},
	{ "ChordTranRepeat", &ChordTranRepeat},
	{ "ChordTranScaleDistance", &ChordTranScaleDistance},
	{ "ChordTranScaleUnion", &ChordTranScaleUnion},
	{ "ChordTranDistance", &ChordTranDistance},
	{ "ChordTranOuter", &ChordTranOuter},
	{ "ChordTranBassInterval", &ChordTranBassInterval},
	{ "ChordTranMelodyInterval", &ChordTranMelodyInterval},
	{ "ChordMelodyNgram", &ChordMelodyNgram},
	{ "PCDTran", &PCDTran}
};

#endif