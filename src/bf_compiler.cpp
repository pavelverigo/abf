#include "bf_compiler.hpp"

#include <optional>
#include <unordered_map>

#include "common.hpp"

void rawbf_in(std::istream& in, std::vector<Inst>& data) {
    char c;
    while (in.get(c)) {
        switch (c) {
        case '>': {
            data.push_back({
                .tag = Move,
                .move = 1,
            });
        } break;
        case '<': {
            data.push_back({
                .tag = Move,
                .move = -1,
            });
        } break;
        case '+': {
            data.push_back({
                .tag = Add,
                .add = 1,
            });
        } break;
        case '-': {
            data.push_back({
                .tag = Add,
                .add = static_cast<uint8_t>(-1),
            });
        } break;
        case '[': {
            data.push_back({
                .tag = JumpL,
            });
        } break;
        case ']': {
            data.push_back({
                .tag = JumpR,
            });
        } break;
        case '.': {
            data.push_back({
                .tag = Write,
            });
        } break;
        case ',': {
            data.push_back({
                .tag = Read,
            });
        } break;
        default: {
            // Do nothing
        } break;
        }
    }
}

void textinst_out(std::ostream& out, const std::vector<Inst>& data, int indent, bool add_sign) {
    int cur_indent = 0;
    for (const auto& inst : data) {
        if (inst.tag == JumpR) {
            cur_indent -= indent;
        }
        out << std::string(cur_indent, ' ');
        if (inst.tag == JumpL) {
            cur_indent += indent;
        }

        switch (inst.tag) {
        case Move: {
            out << "mv " << static_cast<int>(inst.move);
        } break;
        case Add: {
            out << "add ";
            if (add_sign) {
                if (inst.add <= 127) {
                    out << static_cast<int>(inst.add);
                } else {
                    out << static_cast<int>(inst.add) - 256;
                }
            } else {
                out << static_cast<int>(inst.add);
            }
        } break;
        case JumpL: {
            out << '[';
        } break;
        case JumpR: {
            out << ']';
        } break;
        case Write: {
            out << "write";
        } break;
        case Read: {
            out << "read";
        } break;
        case ZeroAdd: {
            out << "zero ";
            for (int i = 0; i < inst.za.cnt; ++i) {
                out << "(mv " << static_cast<int>(inst.za.shift[i]) << ", * ";
                if (add_sign) {
                    if (inst.za.mul[i] <= 127) {
                        out << static_cast<int>(inst.za.mul[i]);
                    } else {
                        out << static_cast<int>(inst.za.mul[i]) - 256;
                    }
                } else {
                    out << static_cast<int>(inst.za.mul[i]);
                }
                out << ") ";
            }
        } break;
        default: {
            panic("not implemented, textinst_out");
        } break;
        }
        out << '\n';
    }
}

// Adopted from https://github.com/itchyny/llvm-brainfuck
// TODO: use bitcode, text format have not guarantees to be backward compatible between llvm versions
void textllvm_out(std::ostream& out, const std::vector<Inst>& data) {
    int idx = 1;
    int loop_idx = 0;
    std::vector<int> loop_stack;
    
    // header
    out << "define i32 @main() {\n";
    out << "  %data = alloca i8*, align 8\n";
    out << "  %ptr = alloca i8*, align 8\n";
    out << "  %data_ptr = call i8* @calloc(i64 30000, i64 1)\n";
    out << "  store i8* %data_ptr, i8** %data, align 8\n";
    out << "  store i8* %data_ptr, i8** %ptr, align 8\n";

    for (const auto& inst : data) {
        switch (inst.tag) {
        case Move: {
            out << "  %" << idx << " = load i8*, i8** %ptr, align 8\n";
            out << "  %" << idx + 1 << " = getelementptr inbounds i8, i8* %" << idx << ", i32 " << static_cast<int>(inst.move) << "\n";
            out << "  store i8* %" << idx + 1 << ", i8** %ptr, align 8\n";
            idx += 2;
        } break;
        case Add: {
            out << "  %" << idx << " = load i8*, i8** %ptr, align 8\n";
            out << "  %" << idx + 1 << " = load i8, i8* %" << idx << ", align 1\n";
            out << "  %" << idx + 2 << " = add i8 %" << idx + 1 << ", " << static_cast<int>(inst.add) << "\n";
            out << "  store i8 %" << idx + 2 << ", i8* %" << idx << ", align 1\n";
            idx += 3;
        } break;
        case JumpL: {
            int while_idx = loop_idx;
            out << "  br label %while_cond" << while_idx << "\n";
            out << "while_cond" << while_idx << ":\n";
            out << "  %" << idx << " = load i8*, i8** %ptr, align 8\n";
            out << "  %" << idx + 1 << " = load i8, i8* %" << idx << ", align 1\n";
            out << "  %" << idx + 2 << " = icmp ne i8 %" << idx + 1 << ", 0\n";
            out << "  br i1 %" << idx + 2 << ", label %while_body" << while_idx << ", label %while_end" << while_idx << "\n";
            out << "while_body" << while_idx << ":\n";
            idx += 3;

            loop_stack.push_back(loop_idx);
            loop_idx++;
        } break;
        case JumpR: {
            int while_idx = loop_stack.back();
            out << "  br label %while_cond" << while_idx << "\n";
            out << "while_end" << while_idx << ":\n";

            loop_stack.pop_back();
        } break;
        case Write: {
            out << "  %" << idx << " = load i8*, i8** %ptr, align 8\n";
            out << "  %" << idx + 1 << " = load i8, i8* %" << idx << ", align 1\n";
            out << "  %" << idx + 2 << " = sext i8 %" << idx + 1 << " to i32\n";
            out << "  %" << idx + 3 << " = call i32 @putchar(i32 %" << idx + 2 << ")\n";
            idx += 4;
        } break;
        case Read: {
            out << "  %" << idx << " = call i32 @getchar()\n";
            out << "  %" << idx + 1 << " = trunc i32 %" << idx << " to i8\n";
            out << "  %" << idx + 2 << " = load i8*, i8** %ptr, align 8\n";
            out << "  store i8 %" << idx + 1 << ", i8* %" << idx + 2 << ", align 1\n";
            idx += 3;
        } break;
        case ZeroAdd: {
            out << "  %" << idx << " = load i8*, i8** %ptr, align 8\n";
            out << "  %" << idx + 1 << " = load i8, i8* %" << idx << ", align 1\n";
            out << "  store i8 0, i8* %" << idx << ", align 1\n";

            for (int i = 0; i < inst.za.cnt; ++i) {
                int loop_idx = idx + 2 + i * 4;

                out << "  %" << loop_idx << " = getelementptr inbounds i8, i8* %" << idx << ", i32 " << static_cast<int>(inst.za.shift[i]) << "\n";
                out << "  %" << loop_idx + 1 << " = load i8, i8* %" << loop_idx << ", align 1\n";

                out << "  %" << loop_idx + 2 << " = mul i8 %" << idx + 1 << ", " << static_cast<int>(inst.za.mul[i]) << "\n";

                out << "  %" << loop_idx + 3 << " = add i8 %" << loop_idx + 1 << ", %" << loop_idx + 2 << "\n";

                out << "  store i8 %" << loop_idx + 3 << ", i8* %" << loop_idx << ", align 1\n";
            }

            idx += 2 + inst.za.cnt * 4;
        } break;
        default: {
            panic("not implemented, textllvm_out");
        } break;
        }
    }

    // footer
    out << "  %" << idx << " = load i8*, i8** %data, align 8\n";
    out << "  call void @free(i8* %" << idx << ")\n";
    out << "  ret i32 0\n";
    out << "}\n\n";

    out << "declare i8* @calloc(i64, i64)\n\n";
    out << "declare void @free(i8*)\n\n";
    out << "declare i32 @putchar(i32)\n\n";
    out << "declare i32 @getchar()\n";
}

void concat_pass(const std::vector<Inst>& in, std::vector<Inst>& out) {
    std::optional<Inst> prev;

    const auto no_op = [](const Inst& inst) {
        switch (inst.tag) {
        case Move:
            return inst.move == 0;
        case Add:
            return inst.add == 0;
        default:
            return false;
        }
    };

    for (const auto& inst : in) {
        switch (inst.tag) {
        case Move: {
            if (!prev.has_value()) {
                prev = inst;
                continue;
            }
            if ((*prev).tag != Move) {
                if (!no_op(*prev)) out.push_back(*prev);
                prev = inst;
                continue;
            }

            (*prev).move += inst.move;
        } break;
        case Add: {
            if (!prev.has_value()) {
                prev = inst;
                continue;
            }
            if ((*prev).tag != Add) {
                if (!no_op(*prev)) out.push_back(*prev);
                prev = inst;
                continue;
            }

            (*prev).add += inst.add;
        } break;
        default: {
            if (prev.has_value()) {
                if (!no_op(*prev)) out.push_back(*prev);
                prev.reset();
            }

            out.push_back(inst);
        };
        }
    }

    if (prev.has_value()) {
        if (!no_op(*prev)) out.push_back(*prev);
    }
}


// Handle such code:
// [-], [>>>++<<<-], [->+>-<<], etc
void zeroadd_pass(const std::vector<Inst>& in, std::vector<Inst>& out) {
    int written_idx = 0;

    std::optional<int> start_loop_cand_idx;

    std::unordered_map<int16_t, uint8_t> shift_mul;
    int16_t cur_shift = 0;

    const auto reset_and_catchup = [&](int idx) {
        start_loop_cand_idx.reset();
        shift_mul.clear();
        cur_shift = 0;

        for (; written_idx <= idx; ++written_idx) {
            out.push_back(in[written_idx]);
        }
    };
    
    int inst_idx = 0;
    // Searching for loop without loops
    for (const auto& inst : in) {
        switch (inst.tag) {
        case JumpL: {
            if (start_loop_cand_idx.has_value()) {
                reset_and_catchup(inst_idx - 1);
            }
            start_loop_cand_idx = inst_idx;
        } break;
        case JumpR: {
            if (start_loop_cand_idx.has_value()) {
                if (shift_mul.contains(0) && shift_mul[0] == (uint8_t)(-1) && cur_shift == 0) {
                    // All conditions met
                    
                    if (shift_mul.size() > (ZA_ARR_SIZE + 1)) {
                        panic("not implemented, zeroadd_pass");
                    }

                    Inst za_inst = {
                        .tag = ZeroAdd,
                        .za = {},
                    };
                    for (auto [shift, mul] : shift_mul) {
                        if (shift != 0 && mul != 0) {
                            za_inst.za.shift[za_inst.za.cnt] = shift;
                            za_inst.za.mul[za_inst.za.cnt] = mul;
                            ++za_inst.za.cnt;
                        }
                    }

                    out.push_back(za_inst);
                    written_idx = inst_idx + 1;
                    start_loop_cand_idx.reset();
                    shift_mul.clear();
                    cur_shift = 0;
                } else {
                    reset_and_catchup(inst_idx);
                }
            } else {
                ++written_idx;
                out.push_back(inst);
            }
        } break;
        case Move: {
            if (start_loop_cand_idx.has_value()) {
                cur_shift += inst.move;
            } else {
                ++written_idx;
                out.push_back(inst);
            }
        } break;
        case Add: {
            if (start_loop_cand_idx.has_value()) {
                shift_mul[cur_shift] += inst.add;
            } else {
                ++written_idx;
                out.push_back(inst);
            }
        } break;
        default: {
            if (start_loop_cand_idx.has_value()) {
                reset_and_catchup(inst_idx);
            } else {
                ++written_idx;
                out.push_back(inst);
            }
        };
        }

        ++inst_idx;
    }
}