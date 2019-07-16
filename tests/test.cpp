#include <vector>
#include <array>
#include "parse.hpp"
#include "catch/catch.hpp"

//#include "parse.hpp"
//#include "features.hpp"
//#include "collector.hpp"

/*
g++ -I../src/style_rank -I../src/style_rank/deps -o test test_features.cpp ../src/style_rank/deps/Binasc.cpp ../src/style_rank/deps/MidiEvent.cpp ../src/style_rank/deps/MidiEventList.cpp ../src/style_rank/deps/MidiFile.cpp ../src/style_rank/deps/MidiMessage.cpp -std=c++14
*/

/*
-##-----
---#-###
##--#---
------##
####----
----#---
-----##-
*/

static std::vector<std::array<int,3>> example_notes = {
    {60,0,2},
    {57,0,4},
    {64,1,2},
    {62,3,1},
    {60,4,1},
    {55,4,1},
    {54,5,2},
    {62,5,1},
    {62,6,2},
    {59,6,2}
};

int main() {

}

/*
TEST_CASE("SIMPLE_MIDI_PARSE")
{
    // pitch, onset, duration
    std::vector<std::array<int,3>> notes = {{62, 0, 1}, {60, 0, 1}, {60, 1, 2}};
    Piece *p = new Piece(notes);

    REQUIRE(p->unique_onsets.size() == 3); // one extra for last onset
    REQUIRE(p->chords.size() == 2); // has two chords
    REQUIRE(p->notes.size() == 3); // has three notes
    REQUIRE(p->chords[0].notes[0]->pitch == 60); // notes should be sorted

}

TEST_CASE("EMPTY_MIDI_PARSE")
{
    std::vector<std::array<int,3>> notes;
    Piece *p = new Piece(notes);

    REQUIRE(p->unique_onsets.size() == 0); // has no onsets
    REQUIRE(p->chords.size() == 0); // has no chords
    REQUIRE(p->notes.size() == 0); // has no notes

}

// helper function for testing
TestCollector getFeature(Piece *p, void(*fn)(Piece*, Collector*, FEATURE_INFO&)) {
    TestCollector c;
    auto info = FEATURE_INFO("test");
    fn(p, &c, info);
    return c;
}

TEST_CASE("CHORD_DISTINCT_DURATION_RATIO")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordDistinctDurationRatio).getDiscreteDist();
    REQUIRE(D[NOMINAL_TUPLE(2,2).value] == 2);
    REQUIRE(D[NOMINAL_TUPLE(2,3).value] == 1);
    REQUIRE(D[NOMINAL_TUPLE(3,3).value] == 1);
    REQUIRE(D[NOMINAL_TUPLE(1,2).value] == 2);
}

TEST_CASE("CHORD_DURATION")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordDuration).getDiscreteDist();
    REQUIRE(D[1] == 4);
    REQUIRE(D[2] == 2);
}

TEST_CASE("CHORD_ONSET")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordOnset).getDiscreteDist();
    REQUIRE(D[std::stoi("111", nullptr, 2)] == 3);
    REQUIRE(D[std::stoi("1100", nullptr, 2)] == 1);
    REQUIRE(D[std::stoi("110", nullptr, 2)] == 1);
    REQUIRE(D[std::stoi("1110", nullptr, 2)] == 1);
}

TEST_CASE("CHORD_ONSET_RATIO")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordOnsetRatio).getDiscreteDist();
    REQUIRE(D[NOMINAL_TUPLE(2,2).value] == 3);
    REQUIRE(D[NOMINAL_TUPLE(1,3).value] == 1);
    REQUIRE(D[NOMINAL_TUPLE(1,2).value] == 1);
    REQUIRE(D[NOMINAL_TUPLE(2,3).value] == 1);
}

TEST_CASE("CHORD_PCD")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordPCD).getDiscreteDist();
    REQUIRE(D[pcd[std::stoi("001000000001", nullptr, 2)]] == 1);
    REQUIRE(D[pcd[std::stoi("001000010001", nullptr, 2)]] == 4);
    REQUIRE(D[pcd[std::stoi("001000000100", nullptr, 2)]] == 2);
    REQUIRE(D[pcd[std::stoi("000001000100", nullptr, 2)]] == 1);
}

TEST_CASE("CHORD_PC_SIZE_RATIO")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordPCSizeRatio).getDiscreteDist();
    REQUIRE(D[NOMINAL_TUPLE(2,2).value] == 4);
    REQUIRE(D[NOMINAL_TUPLE(3,3).value] == 2);
}

TEST_CASE("CHORD_SHAPE")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordShape).getDiscreteDist();
    REQUIRE(D[pcd[std::stoi("1001", nullptr, 2)]] == 1);
    REQUIRE(D[pcd[std::stoi("10001001", nullptr, 2)]] == 2);
    REQUIRE(D[pcd[std::stoi("10000001", nullptr, 2)]] == 2);
    REQUIRE(D[pcd[std::stoi("1000000001", nullptr, 2)]] == 1);
    REQUIRE(D[pcd[std::stoi("100100001", nullptr, 2)]] == 2);
}

TEST_CASE("CHORD_SIZE")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordSize).getDiscreteDist();
    REQUIRE(D.find(0) == D.end()); // has no chords of size zero
    REQUIRE(D.find(0) == D.end()); // has no chords of size 1
    REQUIRE(D[2] == 4);
    REQUIRE(D[3] == 2);
}

TEST_CASE("CHORD_RANGE")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordRange).getDiscreteDist();
    REQUIRE(D[3] == 1);
    REQUIRE(D[5] == 2);
    REQUIRE(D[7] == 1);
    REQUIRE(D[8] == 2);
}

TEST_CASE("CHORD_LOWEST_INTERVAL")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordLowestInterval).getDiscreteDist();
    REQUIRE(D[3] == 2);
    REQUIRE(D[5] == 3);
    REQUIRE(D[8] == 1);
}

TEST_CASE("CHORD_TRAN_VOICE_MOTION")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordTranVoiceMotion).getDiscreteDist();
    REQUIRE(D[static_cast<uint64_t>(VOICE_MOTION_TYPE::NO_CHANGE)] == 1);
    REQUIRE(D[static_cast<uint64_t>(VOICE_MOTION_TYPE::CONTRARY_MOTION)] == 1);
    REQUIRE(D[static_cast<uint64_t>(VOICE_MOTION_TYPE::PARALLEL_MOTION)] == 1);
    REQUIRE(D[static_cast<uint64_t>(VOICE_MOTION_TYPE::OBLIQUE_MOTION)] == 2);
}

TEST_CASE("CHORD_TRAN_DISTANCE")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, ChordTranDistance).getDiscreteDist();
    REQUIRE(D[0] == 1);
    REQUIRE(D[2] == 1);
    REQUIRE(D[3] == 1);
    REQUIRE(D[4] == 2);

}

TEST_CASE("INTERVAL_CLASS_DIST")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, IntervalClassDist).getDiscreteDist();
    REQUIRE(D.find(0) == D.end());
    REQUIRE(D.find(1) == D.end());
    REQUIRE(D.find(2) == D.end());
    REQUIRE(D[3] == 5);
    REQUIRE(D[4] == 5);
    REQUIRE(D[5] == 6);
    REQUIRE(D.find(6) == D.end());
}

TEST_CASE("INTERVAL_DIST")
{
    Piece *p = new Piece(example_notes);
    auto D = getFeature(p, IntervalDist).getDiscreteDist();
    REQUIRE(D.find(0) == D.end());
    REQUIRE(D.find(1) == D.end());
    REQUIRE(D.find(2) == D.end());
    REQUIRE(D[3] == 5);
    REQUIRE(D[4] == 2);
    REQUIRE(D[5] == 4);
    REQUIRE(D.find(6) == D.end());
    REQUIRE(D[7] == 2);
    REQUIRE(D[8] == 3);
    REQUIRE(D.find(9) == D.end());
    REQUIRE(D.find(10) == D.end());
    REQUIRE(D.find(11) == D.end());
}

TEST_CASE("CHORD_PARSE")
{
    Piece *p = new Piece(example_notes);

    REQUIRE(p->chords[0].notes[0]->pitch == 57);
    REQUIRE(p->chords[0].notes[1]->pitch == 60);
    
    REQUIRE(p->chords[1].notes[0]->pitch == 57);
    REQUIRE(p->chords[1].notes[1]->pitch == 60);
    REQUIRE(p->chords[1].notes[2]->pitch == 64);

    REQUIRE(p->chords[2].notes[0]->pitch == 57);
    REQUIRE(p->chords[2].notes[1]->pitch == 62);

    REQUIRE(p->chords[3].notes[0]->pitch == 55);
    REQUIRE(p->chords[3].notes[1]->pitch == 60);

    REQUIRE(p->chords[4].notes[0]->pitch == 54);
    REQUIRE(p->chords[4].notes[1]->pitch == 62);

    REQUIRE(p->chords[5].notes[0]->pitch == 54);
    REQUIRE(p->chords[5].notes[1]->pitch == 59);
    REQUIRE(p->chords[5].notes[2]->pitch == 62);
}

*/





