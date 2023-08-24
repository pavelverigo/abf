#pragma once

#include <cstdint>
#include <istream>
#include <ostream>
#include <vector>

enum InstTag {
    Add,
    Move,   
    Write,
    Read,
    JumpL,
    JumpR,

    // Advanced
    ZeroAdd // TODO: better name
};

using AddData = uint8_t;

using MoveData = int16_t;

// TODO: have an dynamic array of (shift, mul), needed deallocation of Inst then.
#define ZA_ARR_SIZE 5
struct ZeroAddData {
    int16_t shift[ZA_ARR_SIZE];
    uint8_t mul[ZA_ARR_SIZE];

    int8_t cnt;
};

struct Inst {
    InstTag tag;
    union {
        AddData add;
        MoveData move;
        ZeroAddData za;
    };
};

void rawbf_in(std::istream& in, std::vector<Inst>& data);

void textinst_out(std::ostream& out, const std::vector<Inst>& data, int indent = 2, bool add_sign = true);

void textllvm_heap_out(std::ostream& out, const std::vector<Inst>& data);

void textqbe_out(std::ostream& out, const std::vector<Inst>& data);

void concat_pass(const std::vector<Inst>& in, std::vector<Inst>& out);

void zeroadd_pass(const std::vector<Inst>& in, std::vector<Inst>& out);