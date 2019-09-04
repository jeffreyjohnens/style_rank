#define CATCH_CONFIG_MAIN
#include "catch.h"
#include <vector>
#include <array>
#include "../src/style_rank/parse.hpp"
#include "../src/style_rank/features.hpp"
#include "../src/style_rank/feature_map.hpp"
#include "../src/style_rank/utils.hpp"

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

TEST_CASE("SIMPLE_MIDI_PARSE")
{
    Piece *p = new Piece(example_notes);

    REQUIRE(p->unique_onsets.size() == 7); // one extra for last onset
    REQUIRE(p->chords.size() == 6); // has six chords
    REQUIRE(p->notes.size() == 10); // has ten notes
    REQUIRE(p->max_duration == 4); // max duration is 4

    // check the chord sizes are correct
    REQUIRE(p->chords[0].notes.size() == 2);
    REQUIRE(p->chords[1].notes.size() == 3);
    REQUIRE(p->chords[2].notes.size() == 2);
    REQUIRE(p->chords[3].notes.size() == 2);
    REQUIRE(p->chords[4].notes.size() == 2);
    REQUIRE(p->chords[5].notes.size() == 3);

    // check the durations are correct
    REQUIRE(p->chords[0].duration == 1);
    REQUIRE(p->chords[1].duration == 2);
    REQUIRE(p->chords[2].duration == 1);
    REQUIRE(p->chords[3].duration == 1);
    REQUIRE(p->chords[4].duration == 1);
    REQUIRE(p->chords[5].duration == 2);

    delete p;
}

TEST_CASE("EMPTY_MIDI_PARSE")
{
    std::vector<std::array<int,3>> notes;
    Piece *p = new Piece(notes);

    REQUIRE(p->unique_onsets.size() == 0); // has no onsets
    REQUIRE(p->chords.size() == 0); // has no chords
    REQUIRE(p->notes.size() == 0); // has no notes

    delete p;
}

TEST_CASE("CHORD_DISTINCT_DURATION_RATIO")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordDistinctDurationRatio"](p);

    REQUIRE((*D)[NOMINAL_TUPLE(2,2).value] == 2);
    REQUIRE((*D)[NOMINAL_TUPLE(2,3).value] == 1);
    REQUIRE((*D)[NOMINAL_TUPLE(3,3).value] == 1);
    REQUIRE((*D)[NOMINAL_TUPLE(1,2).value] == 2);

    delete p;
}

TEST_CASE("CHORD_DURATION")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordDuration"](p);
    REQUIRE((*D)[8] == 4);
    REQUIRE((*D)[16] == 2);

    delete p;
}

TEST_CASE("CHORD_ONSET")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordOnset"](p);
    REQUIRE((*D)[std::stoi("111", nullptr, 2)] == 3);
    REQUIRE((*D)[std::stoi("1100", nullptr, 2)] == 1);
    REQUIRE((*D)[std::stoi("110", nullptr, 2)] == 1);
    REQUIRE((*D)[std::stoi("1110", nullptr, 2)] == 1);

    delete p;
}

TEST_CASE("CHORD_ONSET_RATIO")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordOnsetRatio"](p);
    REQUIRE((*D)[NOMINAL_TUPLE(2,2).value] == 3);
    REQUIRE((*D)[NOMINAL_TUPLE(1,3).value] == 1);
    REQUIRE((*D)[NOMINAL_TUPLE(1,2).value] == 1);
    REQUIRE((*D)[NOMINAL_TUPLE(2,3).value] == 1);

    delete p;
}

TEST_CASE("CHORD_PCD")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordPCD"](p);
    REQUIRE((*D)[pcd[std::stoi("001000000001", nullptr, 2)]] == 1);
    REQUIRE((*D)[pcd[std::stoi("001000010001", nullptr, 2)]] == 4);
    REQUIRE((*D)[pcd[std::stoi("001000000100", nullptr, 2)]] == 2);
    REQUIRE((*D)[pcd[std::stoi("000001000100", nullptr, 2)]] == 1);

    delete p;
}

TEST_CASE("CHORD_PC_SIZE_RATIO")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordPCSizeRatio"](p);
    REQUIRE((*D)[NOMINAL_TUPLE(2,2).value] == 4);
    REQUIRE((*D)[NOMINAL_TUPLE(3,3).value] == 2);

    delete p;
}

TEST_CASE("CHORD_SHAPE")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordShape"](p);
    REQUIRE((*D)[pcd[std::stoi("1001", nullptr, 2)]] == 1);
    REQUIRE((*D)[pcd[std::stoi("10001001", nullptr, 2)]] == 2);
    REQUIRE((*D)[pcd[std::stoi("10000001", nullptr, 2)]] == 2);
    REQUIRE((*D)[pcd[std::stoi("1000000001", nullptr, 2)]] == 1);
    REQUIRE((*D)[pcd[std::stoi("100100001", nullptr, 2)]] == 2);

    delete p;
}

TEST_CASE("CHORD_SIZE")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordSize"](p);
    REQUIRE((*D).find(0) == (*D).end()); // has no chords of size zero
    REQUIRE((*D).find(0) == (*D).end()); // has no chords of size 1
    REQUIRE((*D)[2] == 4);
    REQUIRE((*D)[3] == 2);

    delete p;
}

TEST_CASE("CHORD_RANGE")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordRange"](p);
    REQUIRE((*D)[3] == 1);
    REQUIRE((*D)[5] == 2);
    REQUIRE((*D)[7] == 1);
    REQUIRE((*D)[8] == 2);

    delete p;
}

TEST_CASE("CHORD_LOWEST_INTERVAL")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordLowestInterval"](p);
    REQUIRE((*D)[3] == 2);
    REQUIRE((*D)[5] == 3);
    REQUIRE((*D)[8] == 1);

    delete p;
}

TEST_CASE("CHORD_TRAN_VOICE_MOTION")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordTranVoiceMotion"](p);
    REQUIRE((*D)[static_cast<uint64_t>(VOICE_MOTION_TYPE::NO_CHANGE)]==1);
    REQUIRE((*D)[static_cast<uint64_t>(VOICE_MOTION_TYPE::CONTRARY_MOTION)]==1);
    REQUIRE((*D)[static_cast<uint64_t>(VOICE_MOTION_TYPE::PARALLEL_MOTION)]==1);
    REQUIRE((*D)[static_cast<uint64_t>(VOICE_MOTION_TYPE::OBLIQUE_MOTION)]==2);

    delete p;
}

TEST_CASE("CHORD_TRAN_DISTANCE")
{
    Piece *p = new Piece(example_notes);
    auto D = m["ChordTranDistance"](p);
    REQUIRE((*D)[0] == 1);
    REQUIRE((*D)[2] == 1);
    REQUIRE((*D)[3] == 1);
    REQUIRE((*D)[4] == 2);

    delete p;
}

TEST_CASE("INTERVAL_CLASS_DIST")
{
    Piece *p = new Piece(example_notes);
    auto D = m["IntervalClassDist"](p);
    REQUIRE((*D).find(0) == (*D).end());
    REQUIRE((*D).find(1) == (*D).end());
    REQUIRE((*D).find(2) == (*D).end());
    REQUIRE((*D)[3] == 5);
    REQUIRE((*D)[4] == 5);
    REQUIRE((*D)[5] == 6);
    REQUIRE((*D).find(6) == (*D).end());

    delete p;
}

TEST_CASE("INTERVAL_DIST")
{
    Piece *p = new Piece(example_notes);
    auto D = m["IntervalDist"](p);
    REQUIRE((*D).find(0) == (*D).end());
    REQUIRE((*D).find(1) == (*D).end());
    REQUIRE((*D).find(2) == (*D).end());
    REQUIRE((*D)[3] == 5);
    REQUIRE((*D)[4] == 2);
    REQUIRE((*D)[5] == 4);
    REQUIRE((*D).find(6) == (*D).end());
    REQUIRE((*D)[7] == 2);
    REQUIRE((*D)[8] == 3);
    REQUIRE((*D).find(9) == (*D).end());
    REQUIRE((*D).find(10) == (*D).end());
    REQUIRE((*D).find(11) == (*D).end());

    delete p;
}





