//
// Created by Xiaolin Zheng on 2022/2/9.
//

#include "assembler_rvc_histogram.hpp"
#include "runtime/atomic.hpp"
#include "utilities/pair.hpp"
#include "memory/resourceArea.hpp"
#include "memory/resourceArea.inline.hpp"

const char *RVCCalculator::names[final+1] = {
// 00
"c_halt",
"c_addi4spn",
"c_fld",
"c_lw",
"c_ld",
"c_fsd",
"c_sw",
"c_sd",
// 01
"c_nop",
"c_addi",
"c_addiw",
"c_li",
"c_addi16sp",
"c_lui",
"c_srli",
"c_srai",
"c_andi",
"c_sub",
"c_xor",
"c_or",
"c_and",
"c_subw",
"c_addw",
"c_j",
"c_beqz",
"c_bnez",
// 10
"c_slli",
"c_fldsp",
"c_lwsp",
"c_ldsp",
"c_jr",
"c_mv",
"c_ebreak",
"c_jalr",
"c_add",
"c_fsdsp",
"c_swsp",
"c_sdsp",

"add","sub","subw","andr","orr","xorr","addw","lw","ld","fld","sd","sw","fsd","beq","bne","jal","jalr","ebreak",
"halt","li(*)" /* multi insts... */,"addi","addiw","andi","slli","srai","srli","lui",

"total_insts",
};
unsigned long RVCCalculator::times[RVCCalculator::final+1];

void RVCCalculator::atomic_add_one(Inst inst, bool scratch) {
  if (!scratch) {
    Atomic::fetch_and_add(&times[inst], 1u);
  }
}

void RVCCalculator::total_atomic_add_one(bool scratch) {
  atomic_add_one(Inst::final, scratch);
}

#define YELLOW_PRINT       "\033[33m"
#define GREY_PRINT         "\033[36m"
#define RESET_COLOR        "\033[0m"

#define RAW_FRACT          "(%.2lf%%)"
#define FRACT              GREY_PRINT   RAW_FRACT RESET_COLOR
#define SP_FRACT           YELLOW_PRINT RAW_FRACT RESET_COLOR

#define INST_FAMILY        "[%s family]"
#define DATA               "[%ld" FRACT    "]"
#define SP_DATA            "[%ld" SP_FRACT "]"
#define INST_DATA          "%-11s:" DATA
#define SP_INST_DATA       "%-11s:" SP_DATA

#define SPLIT_SYMBOL_SIZE  3
#define DATA_SIZE          45

void RVCCalculator::split() {
  tty->print_cr("---------------------------------------------------------------------------------------------------------------------");
}

void RVCCalculator::print_header(stringStream &ss, Inst normal_inst) {
  ss.print(INST_FAMILY, names[normal_inst]);
}

void RVCCalculator::print_col1(stringStream &ss, Inst c_inst, int c_inst_num, int total) {
  const int col = 20;
  ss.fill_to(col - SPLIT_SYMBOL_SIZE);
  ss.print("|");
  ss.fill_to(col);
  ss.print(SP_INST_DATA,
           names[c_inst], times[c_inst], percent_of(c_inst_num, total));
}

void RVCCalculator::print_col2(stringStream &ss, Inst normal_inst, int normal_inst_num, int total) {
  print_col2_empty(ss);
  ss.print(INST_DATA,
           names[normal_inst], times[normal_inst], percent_of(normal_inst_num, total));
}

void RVCCalculator::print_col2_empty(stringStream &ss) {
  const int col = 20 + DATA_SIZE;
  ss.fill_to(col - SPLIT_SYMBOL_SIZE);
  ss.print("|");
  ss.fill_to(col);
}

void RVCCalculator::print_col3(stringStream &ss, int total) {
  print_col3_empty(ss, false);
  ss.print("%10d", total);
}

void RVCCalculator::print_col3_empty(stringStream &ss, bool sp) {
  int col = 20 + DATA_SIZE * 2 - (sp ? 9 /* workaround: RED / YELLOW things */ : 0);
  ss.fill_to(col - SPLIT_SYMBOL_SIZE);
  ss.print("|");
  ss.fill_to(col);
}

void RVCCalculator::print_two_compressions(Inst c_inst, Inst normal_inst) {
  unsigned long c_inst_num = times[c_inst];
  unsigned long normal_inst_num = times[normal_inst];
  unsigned long total = c_inst_num + normal_inst_num;
  ResourceMark rm;
  stringStream ss;

  print_header(ss, normal_inst);
  print_col1(ss, c_inst, c_inst_num, total);
  print_col2(ss, normal_inst, normal_inst_num, total);
  print_col3(ss, total);

  tty->print_cr("%s", ss.as_string());
  split();
}

void RVCCalculator::print_three_compressions(Inst c_inst1, Inst c_inst2, Inst normal_inst) {
  unsigned long c_inst1_num = times[c_inst1];
  unsigned long c_inst2_num = times[c_inst2];
  unsigned long normal_inst_num = times[normal_inst];
  unsigned long total = c_inst1_num + c_inst2_num + normal_inst_num;
  ResourceMark rm;
  stringStream ss;

  print_header(ss, normal_inst);
  print_col1(ss, c_inst1, c_inst1_num, total);
  print_col2(ss, normal_inst, normal_inst_num, total);
  print_col3(ss, total);
  ss.cr();

  print_col1(ss, c_inst2, c_inst2_num, total);
  print_col2_empty(ss);
  print_col3_empty(ss);

  tty->print_cr("%s", ss.as_string());
  split();
}

void RVCCalculator::print_six_compressions(Inst c_inst1, Inst c_inst2, Inst c_inst3, Inst c_inst4, Inst c_inst5, Inst normal_inst) {
  unsigned long c_inst1_num = times[c_inst1];
  unsigned long c_inst2_num = times[c_inst2];
  unsigned long c_inst3_num = times[c_inst3];
  unsigned long c_inst4_num = times[c_inst4];
  unsigned long c_inst5_num = times[c_inst5];
  unsigned long normal_inst_num = times[normal_inst];
  unsigned long total = c_inst1_num + c_inst2_num + c_inst3_num + c_inst4_num + c_inst5_num + normal_inst_num;
  ResourceMark rm;
  stringStream ss;

  print_header(ss, normal_inst);
  print_col1(ss, c_inst1, c_inst1_num, total);
  print_col2(ss, normal_inst, normal_inst_num, total);
  print_col3(ss, total);
  ss.cr();

  print_col1(ss, c_inst2, c_inst2_num, total);
  print_col2_empty(ss);
  print_col3_empty(ss);
  ss.cr();

  print_col1(ss, c_inst3, c_inst3_num, total);
  print_col2_empty(ss);
  print_col3_empty(ss);
  ss.cr();

  print_col1(ss, c_inst4, c_inst4_num, total);
  print_col2_empty(ss);
  print_col3_empty(ss);
  ss.cr();

  print_col1(ss, c_inst5, c_inst5_num, total);
  print_col2_empty(ss);
  print_col3_empty(ss);

  tty->print_cr("%s", ss.as_string());
  split();
}

static int comparator(Pair<void(*)(), unsigned long> *p1, Pair<void(*)(), unsigned long> *p2) {
  return p2->second - p1->second;
};

void RVCCalculator::print() {
  ResourceMark rm;
  tty->cr();
  tty->print_cr("=================================  H I S T O G R A M  (for generated instructions) ==================================");
  split();
  tty->print_cr("Family           |   RVC Instruction [num(percent)]  | Normal Instruction [num(percent)] |  Total Instruction num  ");
  split();
  // calculate other insts
  unsigned long total_rvc_16bit_insts = 0;           // RVC insts   e.g. c_add
  unsigned long total_rvc_related_32bit_insts = 0;   // RVC insts' normal versions  e.g. _add
  unsigned long other_32bit_insts = 0;               // normal insts excluding RVC insts' normal versions e.g. vsetvli
  unsigned long all_insts = 0;                       // ALL insts
  for (int i = 0; i < RVCCalculator::final_rvc; i++) {
    total_rvc_16bit_insts += times[i];
  }
  for (int i = RVCCalculator::add; i < RVCCalculator::final; i++) {
    total_rvc_related_32bit_insts += times[i];
  }
  other_32bit_insts = times[RVCCalculator::final] /* ALL emit_int32 */ - total_rvc_related_32bit_insts /* _add _xorr ... */;
  all_insts = total_rvc_16bit_insts /* ALL emit_int16 */ + times[RVCCalculator::final] /* ALL emit_int32 */;

  // sort: key - an inst family's total instruction count. e.g. family 'add' includes: c_add + add. family 'lw' includes: c_lwsp + c_lw + lw.
  Pair<void(*)(), unsigned long> family[final];   // <print_func, total_insts_count>

  family[add] = {[]() {
    print_two_compressions(c_add, add);
  }, family_inst_count(c_add, add)};
  family[sub] = {[]() {
    print_two_compressions(c_sub, sub);
  }, family_inst_count(c_sub, sub)};
  family[subw] = {[]() {
    print_two_compressions(c_subw, subw);
  }, family_inst_count(c_subw, subw)};
  family[andr] = {[]() {
    print_two_compressions(c_and, andr);
  }, family_inst_count(c_and, andr)};
  family[orr] = {[]() {
    print_two_compressions(c_or, orr);
  }, family_inst_count(c_or, orr)};
  family[xorr] = {[]() {
    print_two_compressions(c_xor, xorr);
  }, family_inst_count(c_xor, xorr)};
  family[addw] = {[]() {
    print_two_compressions(c_addw, addw);
  }, family_inst_count(c_addw, addw)};
  family[lw] = {[]() {
    print_three_compressions(c_lwsp, c_lw, lw);
  }, family_inst_count(c_lwsp, c_lw, lw)};
  family[ld] = {[]() {
    print_three_compressions(c_ldsp, c_ld, ld);
  }, family_inst_count(c_ldsp, c_ld, ld)};
  family[fld] = {[]() {
    print_three_compressions(c_fldsp, c_fld, fld);
  }, family_inst_count(c_fldsp, c_fld, fld)};
  family[sw] = {[]() {
    print_three_compressions(c_swsp, c_sw, sw);
  }, family_inst_count(c_swsp, c_sw, sw)};
  family[sd] = {[]() {
    print_three_compressions(c_sdsp, c_sd, sd);
  }, family_inst_count(c_sdsp, c_sd, sd)};
  family[fsd] = {[]() {
    print_three_compressions(c_fsdsp, c_fsd, fsd);
  }, family_inst_count(c_fsdsp, c_fsd, fsd)};
  family[beq] = {[]() {
    print_two_compressions(c_beqz, beq);
  }, family_inst_count(c_beqz, beq)};
  family[bne] = {[]() {
    print_two_compressions(c_bnez, bne);
  }, family_inst_count(c_bnez, bne)};
  family[jal] = {[]() {
    print_two_compressions(c_j, jal);
  }, family_inst_count(c_j, jal)};
  family[jalr] = {[]() {
    print_three_compressions(c_jalr, c_jr, jalr);
  }, family_inst_count(c_jalr, c_jr, jalr)};
  family[ebreak] = {[]() {
    print_two_compressions(c_ebreak, ebreak);
  }, family_inst_count(c_ebreak, ebreak)};
  family[halt] = {[]() {
    print_two_compressions(c_halt, halt);
  }, family_inst_count(c_halt, halt)};
  family[li] = {[]() {
    print_two_compressions(c_li, li);
  }, family_inst_count(c_li, li)};
  family[addi]  = {[]() {
    print_six_compressions(c_nop, c_addi, c_mv, c_addi16sp, c_addi4spn, addi);
  }, family_inst_count(c_nop, c_addi, c_mv, c_addi16sp, c_addi4spn, addi)};
  family[addiw]  = {[]() {
    print_two_compressions(c_addiw, addiw);
  }, family_inst_count(c_addiw, addiw)};
  family[and_imm12]  = {[]() {
    print_two_compressions(c_andi, and_imm12);
  }, family_inst_count(c_andi, and_imm12)};
  family[slli]  = {[]() {
    print_two_compressions(c_slli, slli);
  }, family_inst_count(c_slli, slli)};
  family[srai]  = {[]() {
    print_two_compressions(c_srai, srai);
  }, family_inst_count(c_srai, srai)};
  family[srli]  = {[]() {
    print_two_compressions(c_srli, srli);
  }, family_inst_count(c_srli, srli)};
  family[lui]  = {[]() {
    print_two_compressions(c_lui, lui);
  }, family_inst_count(c_lui, lui)};

  qsort(family + final_rvc, final - final_rvc, sizeof(family[0]), (_sort_Fn)comparator);

  // print compression rate
  for (Inst inst = final_rvc; inst < final; inst = static_cast<Inst>(static_cast<int>(inst)+1)) {
    family[inst].first();
  }

  tty->print_cr("===============================================================================");
  tty->print_cr("[1] RVC 16-bit Insts                          " SP_DATA, total_rvc_16bit_insts, percent_of(total_rvc_16bit_insts, all_insts));
  tty->print_cr("[2] RVC Original 32-bit Insts                 " DATA, total_rvc_related_32bit_insts, percent_of(total_rvc_related_32bit_insts, all_insts));
  tty->print_cr("[3] Other 32-bit Insts                        " DATA, other_32bit_insts, percent_of(other_32bit_insts, all_insts));
  tty->print_cr("Total 16-bit && 32-bit Insts [1]+[2]+[3]      " DATA, all_insts, percent_of(all_insts, all_insts));
  int total_32bit_insts = times[RVCCalculator::final];
  tty->print_cr("Ideally Code Size Could Shrink to             " SP_FRACT,
          /*
           * IdealMemorySavedRate% = (([1] * 2) + (([2]+[3]) * 4)) / (([1]+[2]+[3]) * 4)
           *                       = ([1] + ([2]+[3])*2) / (([1]+[2]+[3])*2)
           *
           * Note: Interp + C1 + C2 + StubCode
           */
          percent_of(total_rvc_16bit_insts + total_32bit_insts * 2, all_insts * 2)
  );
  tty->print_cr("===============================================================================");
}
