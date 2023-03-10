#include "SM83.h"
#include "Bus.h"

SM83::SM83() {
    // to clean up the opcode table
    using op = SM83;

    opcode_lookup =
    {
            {"NOP", &op::nop, 4, 1}, {"LD BC,d16", &op::ld_bc_d16, 12, 3}, {"LD (BC),A", &op::ld_abs_bc_a, 8, 1}, {"INC BC", &op::inc_bc, 8, 1}, {"INC B", &op::inc_b, 4, 1}, {"DEC B", &op::dec_b, 4, 1}, {"LD B,d8", &op::ld_b_d8, 8, 2}, {"RLCA", &op::rlca, 4, 1}, {"LD (a16),SP", &op::ld_abs_a16_sp, 20, 3}, {"ADD HL,BC", &op::add_hl_bc, 8, 1}, {"LD A,(BC)", &op::ld_a_abs_bc, 8, 1}, {"DEC BC", &op::dec_bc, 8, 1}, {"INC C", &op::inc_c, 4, 1}, {"DEC C", &op::dec_c, 4, 1}, {"LD C,d8", &op::ld_c_d8, 8, 2}, {"RRCA", &op::rrca, 4, 1},
            {"STOP d8", &op::stop_d8, 4, 2}, {"LD DE,d16", &op::ld_de_d16, 12, 3}, {"LD (DE),A", &op::ld_abs_de_a, 8, 1}, {"INC DE", &op::inc_de, 8, 1}, {"INC D", &op::inc_d, 4, 1}, {"DEC D", &op::dec_d, 4, 1}, {"LD D,d8", &op::ld_d_d8, 8, 2}, {"RLA", &op::rla, 4, 1}, {"JR", &op::jr_r8, 12, 2}, {"ADD HL,DE", &op::add_hl_de, 8, 1}, {"LD A,(DE)", &op::ld_a_abs_de, 8 ,1}, {"DEC DE", &op::dec_de, 8, 1}, {"INC E", &op::inc_e, 4, 1}, {"DEC E", &op::dec_e, 4, 1}, {"LD E,d8", &op::ld_e_d8, 8, 2}, {"RRA", &op::rra, 4, 1},
            {"JR NZ,r8", &op::jr_nz_r8, 8, 2}, {"LD HL,d16", &op::ld_hl_d16, 12, 3}, {"LD (HL+),A", &op::ld_abs_hli_a, 8, 1}, {"INC HL", &op::inc_hl, 8, 1}, {"INC H", &op::inc_h, 4, 1}, {"DEC H", &op::dec_h, 4, 1}, {"LD H,d8", &op::ld_h_d8, 8, 2}, {"DAA", &op::daa, 4, 1}, {"JR Z,r8", &op::jr_z_r8, 8, 2}, {"ADD HL,HL", &op::add_hl_hl, 8, 1}, {"LD A,(HL+)", &op::ld_a_abs_hli, 8, 1}, {"DEC HL", &op::dec_hl, 8, 1}, {"INC L", &op::inc_l, 4, 1}, {"DEC L", &op::dec_l, 4, 1}, {"LD L,d8", &op::ld_l_d8, 8, 2}, {"CPL", &op::cpl, 4, 1},
            {"JR NC,r8", &op::jr_nc_r8, 8, 2}, {"LD SP,d16", &op::ld_sp_d16, 12, 3}, {"LD (HL-),A", &op::ld_abs_hld_a, 8, 1}, {"INC SP", &op::inc_sp, 8, 1}, {"INC (HL)", &op::inc_abs_hl, 12, 1}, {"DEC (HL)", &op::dec_abs_hl, 12, 1}, {"LD (HL),d8", &op::ld_abs_hl_d8, 12, 2}, {"SCF", &op::scf, 4, 1}, {"JR C,r8", &op::jr_c_r8, 8, 2}, {"ADD HL,SP", &op::add_hl_sp, 8, 1}, {"LD A, (HL-)", &op::ld_a_abs_hld, 8, 1}, {"DEC SP", &op::dec_sp, 8, 1}, {"INC A", &op::inc_a, 4, 1}, {"DEC A", &op::dec_a, 4, 1}, {"LD A,d8", &op::ld_a_d8, 8, 2}, {"CCF", &op::ccf, 4, 1},
            {"LD B,B", &op::ld_b_b, 4, 1}, {"LD B,C", &op::ld_b_c, 4, 1}, {"LD B,D", &op::ld_b_d, 4, 1}, {"LD B,E", &op::ld_b_e, 4, 1}, {"LD B,H", &op::ld_b_h, 4, 1}, {"LD B,L", &op::ld_b_l, 4, 1}, {"LD B,(HL)", &op::ld_b_abs_hl, 8, 1}, {"LD B,A", &op::ld_b_a, 4, 1}, {"LD C,B", &op::ld_c_b, 4, 1}, {"LD C,C", &op::ld_c_c, 4, 1}, {"LD C,D", &op::ld_c_d, 4, 1}, {"LD C,E", &op::ld_c_e, 4, 1}, {"LD C,E", &op::ld_c_h, 4, 1}, {"LD C,L", &op::ld_c_l, 4, 1}, {"LD C,(HL)", &op::ld_c_abs_hl, 8, 1}, {"LD C,A", &op::ld_c_a, 4, 1},
            {"LD D,B", &op::ld_d_b, 4, 1}, {"LD D,C", &op::ld_d_c, 4, 1}, {"LD D,D", &op::ld_d_d, 4, 1}, {"LD D,E", &op::ld_d_e, 4, 1}, {"LD D,H", &op::ld_d_h, 4, 1}, {"LD D,L", &op::ld_d_l, 4, 1}, {"LD D,(HL)", &op::ld_d_abs_hl, 8, 1}, {"LD D,A", &op::ld_d_a, 4, 1}, {"LD E,B", &op::ld_e_b, 4, 1}, {"LD E,C", &op::ld_e_c, 4, 1}, {"LD E,D", &op::ld_e_d, 4, 1}, {"LD E,E", &op::ld_e_e, 4, 1}, {"LD E,H", &op::ld_e_h, 4, 1}, {"LD E,L", &op::ld_e_l, 4, 1}, {"LD E,(HL)", &op::ld_e_abs_hl, 8 ,1}, {"LD E,A", &op::ld_e_a, 4, 1},
            {"LD H,B", &op::ld_h_b, 4 ,1}, {"LD H,C", &op::ld_h_c, 4, 1}, {"LD H,D", &op::ld_h_d, 4, 1}, {"LD H,E", &op::ld_h_e, 4, 1}, {"LD H,H", &op::ld_h_h, 4, 1}, {"LD H,L", &op::ld_h_l, 4, 1}, {"LD H,(HL)", &op::ld_h_abs_hl, 8, 1}, {"LD H,A", &op::ld_h_a, 4, 1}, {"LD L,B", &op::ld_l_b, 4, 1}, {"LD L,C", &op::ld_l_c, 4, 1}, {"LD L,D", &op::ld_l_d, 4, 1}, {"LD L,E", &op::ld_l_e, 4, 1}, {"LD L,H", &op::ld_l_h, 4, 1}, {"LD L,L", &op::ld_l_l, 4, 1}, {"LD L,(HL)", &op::ld_l_abs_hl, 8, 1}, {"LD L,A", &op::ld_l_a, 4, 1},
            {"LD (HL),B", &op::ld_abs_hl_b, 8, 1}, {"LD (HL),C", &op::ld_abs_hl_c, 8, 1}, {"LD (HL),D", &op::ld_abs_hl_d, 8 ,1}, {"LD (HL),E", &op::ld_abs_hl_e, 8, 1}, {"LD (HL),H", &op::ld_abs_hl_h, 8 ,1}, {"LD (HL),L", &op::ld_abs_hl_l, 8, 1}, {"HALT", &op::halt, 4, 1}, {"LD (HL),A", &op::ld_abs_hl_a, 8, 1}, {"LD A,B", &op::ld_a_b, 4 ,1}, {"LD A,C", &op::ld_a_c, 4, 1}, {"LD A,D", &op::ld_a_d, 4 ,1}, {"LD A,E", &op::ld_a_e, 4, 1}, {"LD A,H", &op::ld_a_h, 4, 1}, {"LD A,L", &op::ld_a_l, 4 ,1}, {"LD A,(HL)", &op::ld_a_abs_hl, 8, 1}, {"LD A,A", &op::ld_a_a, 4 ,1},
            {"ADD A,B", &op::add_a_b, 4, 1}, {"ADD A,C", &op::add_a_c, 4, 1}, {"ADD A,D", &op::add_a_d, 4, 1}, {"ADD A,E", &op::add_a_e, 4, 1}, {"ADD A,H", &op::add_a_h, 4, 1}, {"ADD A,L", &op::add_a_l, 4, 1}, {"ADD A,(HL)", &op::add_a_abs_hl, 8, 1}, {"ADD A,A", &op::add_a_a, 4, 1}, {"ADC A,B", &op::adc_a_b, 4, 1}, {"ADC A,C", &op::adc_a_c, 4, 1}, {"ADC A,D", &op::adc_a_d, 4, 1}, {"ADC A,E", &op::adc_a_e, 4, 1}, {"ADC A,H", &op::adc_a_h, 4, 1}, {"ADC A,L", &op::adc_a_l, 4, 1}, {"ADC A,(HL)", &op::adc_a_abs_hl, 8, 1}, {"ADC A,A", &op::adc_a_a, 4, 1},
            {"SUB B", &op::sub_b, 4, 1}, {"SUB C", &op::sub_c, 4, 1}, {"SUB D", &op::sub_d, 4, 1}, {"SUB E", &op::sub_e, 4, 1}, {"SUB H", &op::sub_h, 4, 1}, {"SUB L", &op::sub_l, 4, 1}, {"SUB (HL)", &op::sub_abs_hl, 8, 1}, {"SUB A", &op::sub_a, 4, 1}, {"SBC A,B", &op::sbc_a_b, 4, 1},{"SBC A,C", &op::sbc_a_c, 4, 1}, {"SBC A,D", &op::sbc_a_d, 4, 1}, {"SBC A,E", &op::sbc_a_e, 4, 1}, {"SBC A,H", &op::sbc_a_h, 4, 1}, {"SBC A,L", &op::sbc_a_l, 4, 1}, {"SBC A,(HL)", &op::sbc_a_abs_hl, 8, 1}, {"SBC A,A", &op::sbc_a_a, 4, 1},
            {"AND B", &op::and_b, 4, 1}, {"AND C", &op::and_c, 4, 1}, {"AND D", &op::and_d, 4, 1}, {"AND E", &op::and_e, 4, 1}, {"AND H", &op::and_h, 4, 1}, {"AND L", &op::and_l, 4, 1}, {"AND (HL)", &op::and_abs_hl, 8 ,1}, {"AND A", &op::and_a, 4, 1}, {"XOR B", &op::xor_b, 4, 1}, {"XOR C", &op::xor_c, 4, 1}, {"XOR D", &op::xor_d, 4, 1}, {"XOR E", &op::xor_e, 4, 1}, {"XOR H", &op::xor_h, 4, 1}, {"XOR L", &op::xor_l, 4, 1}, {"XOR (HL)", &op::xor_abs_hl, 8 ,1}, {"XOR A", &op::xor_a, 4, 1},
            {"OR B", &op::or_b, 4, 1}, {"OR C", &op::or_c, 4, 1}, {"OR D", &op::or_d, 4, 1}, {"OR E", &op::or_e, 4, 1}, {"OR H", &op::or_h, 4, 1}, {"OR L", &op::or_l, 4, 1}, {"OR (HL)", &op::or_abs_hl, 8, 1}, {"OR A", &op::or_a, 4, 1}, {"CP B", &op::cp_b, 4, 1}, {"CP C", &op::cp_c, 4, 1}, {"CP D", &op::cp_d, 4, 1}, {"CP E", &op::cp_e, 4, 1}, {"CP H", &op::cp_h, 4, 1}, {"CP L", &op::cp_l, 4, 1}, {"CP (HL)", &op::cp_abs_hl, 8, 1}, {"CP A", &op::cp_a, 4, 1},
            {"RET NZ", &op::ret_nz, 8, 1}, {"POP BC", &op::pop_bc, 12, 1}, {"JP NZ,a16", &op::jp_nz_a16, 12, 3}, {"JP a16", &op::jp_a16, 16, 3}, {"CALL NZ,a16", &op::call_nz_a16, 12, 3}, {"PUSH BC", &op::push_bc, 16, 1}, {"ADD A,d8", &op::add_a_d8, 8, 2}, {"RST 00H", &op::rst_00h, 16, 1}, {"RET Z", &op::ret_z, 8, 1}, {"RET", &op::ret, 16, 1}, {"JP Z,a16", &op::jp_z_a16, 12, 3}, {"PREFIX", &op::prefix, 4, 1}, {"CALL Z,a16", &op::call_z_16, 12, 3}, {"CALL a16", &op::call_a16, 24, 3}, {"ADC A,d8", &op::adc_a_d8, 8, 2}, {"RST 08H", &op::rst_08h, 16, 1},
            {"RET NC", &op::ret_nc, 8, 1}, {"POP DE", &op::pop_de, 12, 1}, {"JP NC,a16", &op::jp_nc_a16, 12, 3}, {"XXX", &op::xxx, 4, 1}, {"CALL NC,a16", &op::call_nc_a16, 12, 3}, {"PUSH DE", &op::push_de, 16, 1}, {"SUB d8", &op::sub_d8, 8, 2}, {"RST 10H", &op::rst_10h, 16, 1}, {"RET C", &op::ret_c, 8, 1}, {"RETI", &op::reti, 16, 1}, {"JP C,a16", &op::jp_c_a16, 12, 3}, {"XXX", &op::xxx, 4, 1}, {"CALL C,a16", &op::call_c_a16, 12, 3}, {"XXX", &op::xxx, 4, 1}, {"SBC A,d8", &op::sbc_a_d8, 8, 2}, {"RST 18H", &op::rst_18h, 16, 1},
            {"LDH (a8),A", &op::ldh_abs_a8_a, 12, 2}, {"POP HL", &op::pop_hl, 12, 1}, {"LDH (C),A", &op::ldh_abs_c_a, 8, 1}, {"XXX", &op::xxx, 4, 1}, {"XXX", &op::xxx, 4, 1}, {"PUSH HL", &op::push_hl, 16, 1}, {"AND d8", &op::and_d8, 8, 2}, {"RST 20H", &op::rst_20h, 16, 1}, {"ADD SP,r8", &op::add_sp_r8, 16, 2}, {"JP (HL)", &op::jp_abs_hl, 4, 1}, {"LD (a16),A", &op::ld_abs_a16_a, 16, 3}, {"XXX", &op::xxx, 4, 1}, {"XXX", &op::xxx, 4, 1}, {"XXX", &op::xxx, 4, 1}, {"XOR d8", &op::xor_d8, 8, 2}, {"RST 28H", &op::rst_28h, 16, 1},
            {"LDH A,(a8)", &op::ldh_a_abs_a8, 12, 2}, {"POP AF", &op::pop_af, 12, 1}, {"LDH A,(C)", &op::ldh_a_abs_c, 8, 1}, {"DI", &op::di, 4, 1}, {"XXX", &op::xxx, 4, 1}, {"PUSH AF", &op::push_af, 16, 1}, {"OR d8", &op::or_d8, 8, 2}, {"RST 30H", &op::rst_30h, 16, 1}, {"LD HL,SP + r8", &op::ld_hl_sp_r8, 12, 2}, {"LD SP,HL", &op::ld_sp_hl, 8, 1}, {"LD A,(a16)", &op::ld_a_abs_a16, 16, 3}, {"EI", &op::ei, 4, 1}, {"XXX", &op::xxx, 4, 1}, {"XXX", &op::xxx, 4, 1}, {"CP d8", &op::cp_d8, 8, 2}, {"RST 38H", &op::rst_38h, 16, 1}

    };
    // Note, the cycle count for these instructions are added to the cycle count of the PREFIX opcode, which is 4 cycles.
    prefix_lookup =
    {
            {"RLC B", &op::rlc_b, 4, 2}, {"RLC C", &op::rlc_c, 4, 2}, {"RLC D", &op::rlc_d, 4, 2}, {"RLC E", &op::rlc_e, 4, 2}, {"RLC H", &op::rlc_h, 4, 2}, {"RLC L", &op::rlc_l, 4, 2}, {"RLC (HL)", &op::rlc_abs_hl, 12, 2}, {"RLC A", &op::rlc_a, 4, 2}, {"RRC B", &op::rrc_b, 4, 2}, {"RRC C", &op::rrc_c, 4, 2}, {"RRC D", &op::rrc_d, 4 ,2}, {"RRC E", &op::rrc_e, 4, 2}, {"RRC H", &op::rrc_h, 4, 2}, {"RRC L", &op::rrc_l, 4, 2}, {"RRC (HL)", &op::rrc_abs_hl, 12, 2}, {"RRC A", &op::rrc_a, 4, 2},
            {"RL B", &op::rl_b, 4, 2}, {"RL C", &op::rl_c, 4, 2}, {"RL D", &op::rl_d, 4, 2}, {"RL E", &op::rl_e, 4, 2}, {"RL H", &op::rl_h, 4, 2}, {"RL L", &op::rl_l, 4, 2}, {"RL (HL)", &op::rl_abs_hl, 12, 2}, {"RL A", &op::rl_a, 4, 2}, {"RR B", &op::rr_b, 4, 2}, {"RR C", &op::rr_c, 4, 2}, {"RR D", &op::rr_d, 4, 2}, {"RR E", &op::rr_e, 4, 2}, {"RR H", &op::rr_h, 4, 2}, {"RR L", &op::rr_l, 4, 2}, {"RR (HL)", &op::rr_abs_hl, 12, 2}, {"RR A", &op::rr_a, 4, 2},
            {"SLA B", &op::sla_b, 4, 2}, {"SLA C", &op::sla_c, 4, 2}, {"SLA D", &op::sla_d, 4, 2}, {"SLA E", &op::sla_e, 4, 2}, {"SLA H", &op::sla_h, 4, 2}, {"SLA L", &op::sla_l, 4, 2}, {"SLA (HL)", &op::sla_abs_hl, 12, 4}, {"SLA A", &op::sla_a, 4, 2}, {"SRA B", &op::sra_b, 4, 2}, {"SRA C", &op::sra_c, 4, 2}, {"SRA D", &op::sra_d, 4, 2}, {"SRA E", &op::sra_e, 4, 2}, {"SRA H", &op::sra_h, 4, 2}, {"SRA L", &op::sra_l,4, 2}, {"SRA (HL)", &op::sra_abs_hl, 12, 2}, {"SRA A", &op::sra_a, 4, 2},
            {"SWAP B", &op::swap_b, 4, 2}, {"SWAP C", &op::swap_c, 4, 2}, {"SWAP D", &op::swap_d, 4, 2}, {"SWAP E", &op::swap_e, 4, 2}, {"SWAP H", &op::swap_h, 4, 2}, {"SWAP L", &op::swap_l, 4, 2}, {"SWAP (HL)", &op::swap_abs_hl, 12, 2}, {"SWAP A", &op::swap_a, 4, 2}, {"SRL B", &op::srl_b, 4, 2}, {"SRL C", &op::srl_c, 4, 2}, {"SRL D", &op::srl_d, 4, 2}, {"SRL E", &op::srl_e, 4, 2}, {"SRL H", &op::srl_h, 4, 2}, {"SRL L", &op::srl_l, 4, 2}, {"SRL (HL)", &op::srl_abs_hl, 12, 2}, {"SRL A", &op::srl_a, 4, 2},
            {"BIT 0,B", &op::bit_0_b, 4, 2}, {"BIT 0,C", &op::bit_0_c, 4, 2}, {"BIT 0,D", &op::bit_0_d, 4, 2}, {"BIT 0,E", &op::bit_0_e, 4, 2}, {"BIT 0,H", &op:: bit_0_h, 4, 2},{"BIT 0,L", &op::bit_0_l, 4, 2}, {"BIT 0,(HL)", &op::bit_0_abs_hl, 12, 2}, {"BIT 0,A", &op::bit_0_a, 4, 2}, {"BIT 1,B", &op::bit_1_b, 4, 2}, {"BIT 1,C", &op::bit_1_c, 4, 2}, {"BIT 1,D", &op::bit_1_d, 4, 2}, {"BIT 1,E", &op::bit_1_e, 4, 2}, {"BIT 1,H", &op::bit_1_h, 4, 2}, {"BIT 1,L", &op::bit_1_l, 4, 2}, {"BIT 1,(HL)", &op::bit_1_abs_hl, 12, 2}, {"BIT 1,A", &op::bit_1_a, 4, 2},
            {"BIT 2,B", &op::bit_2_b, 4, 2}, {"BIT 2,C", &op::bit_2_c, 4, 2}, {"BIT 2,D", &op::bit_2_d, 4, 2}, {"BIT 2,E", &op::bit_2_e, 4, 2}, {"BIT 2,H", &op::bit_2_h, 4, 2}, {"BIT 2,L", &op::bit_2_l, 4, 2}, {"BIT 2,(HL)", &op::bit_2_abs_hl, 12, 2}, {"BIT 2,A", &op::bit_2_a, 4, 2}, {"BIT 3,B", &op::bit_3_b, 4, 2}, {"BIT 3,C", &op::bit_3_c, 4, 2}, {"BIT 3,D", &op::bit_3_d, 4, 2}, {"BIT 3,E", &op::bit_3_e, 4, 2}, {"BIT 3,H", &op::bit_3_h,4, 2}, {"BIT 3,L", &op::bit_3_l, 4, 2}, {"BIT 3,(HL)", &op::bit_3_abs_hl, 12, 2}, {"BIT 3,A", &op::bit_3_a, 4, 2},
            {"BIT 4,B", &op::bit_4_b, 4, 2}, {"BIT 4,C", &op::bit_4_c, 4, 2}, {"BIT 4,D", &op::bit_4_d, 4, 2}, {"BIT 4,E", &op::bit_4_e, 4, 2}, {"BIT 4,H", &op::bit_4_h, 4, 2}, {"BIT 4,L", &op::bit_4_l, 4, 2}, {"BIT 4,(HL)", &op::bit_4_abs_hl, 12, 2}, {"BIT 4,A", &op::bit_4_a, 4, 2}, {"BIT 5,B", &op::bit_5_b, 4, 2}, {"BIT 5,C", &op::bit_5_c, 4, 2}, {"BIT 5,D", &op::bit_5_d, 4, 2}, {"BIT 5,E", &op::bit_5_e, 4, 2}, {"BIT 5,H", &op::bit_5_h, 4, 2}, {"BIT 5,L", &op::bit_5_l, 4, 2}, {"BIT 5,(HL)", &op::bit_5_abs_hl, 12, 2}, {"BIT 5,A", &op::bit_5_a, 4, 2},
            {"BIT 6,B", &op::bit_6_b, 4, 2}, {"BIT 6,C", &op::bit_6_c, 4, 2}, {"BIT 6,D", &op::bit_6_d, 4, 2}, {"BIT 6,E", &op::bit_6_e, 4, 2}, {"BIT 6,H", &op::bit_6_h, 4, 2}, {"BIT 6,L", &op::bit_6_l, 4, 2}, {"BIT 6,(HL)", &op::bit_6_abs_hl, 12, 2}, {"BIT 6,A", &op::bit_6_a, 4, 2}, {"BIT 7,B", &op::bit_7_b, 4, 2}, {"BIT 7,C", &op::bit_7_c, 4, 2}, {"BIT 7,D", &op::bit_7_d, 4, 2}, {"BIT 7,E", &op::bit_7_e, 4, 2}, {"BIT 7,H", &op::bit_7_h, 4, 2}, {"BIT 7,L", &op::bit_7_l, 4, 2}, {"BIT 7,(HL)", &op::bit_7_abs_hl, 4, 2}, {"BIT 7,A", &op::bit_7_a, 4, 2},
            {"RES 0,B", &op::res_n_r8, 4, 2, {&b_reg}, 0}, {"RES 0,C", &op::res_n_r8, 4, 2, {&c_reg}, 0}, {"RES 0,D", &op::res_n_r8, 4, 2, {&d_reg}, 0}, {"RES 0,E", &op::res_n_r8, 4, 2, {&e_reg}, 0}, {"RES 0,H", &op::res_n_r8, 4, 2, {&h_reg}, 0}, {"RES 0,L", &op::res_n_r8, 4, 2, {&l_reg}, 0}, {"RES 0,(HL)", &op::res_n_r8, 12, 2, {&l_reg, &h_reg}, 0}, {"RES 0,A", &op::res_n_r8, 4, 2, {&a_reg}, 0}, {"RES 1,B", &op::res_n_r8, 4, 2, {&b_reg}, 1}, {"RES 1,C", &op::res_n_r8, 4, 2, {&c_reg}, 1}, {"RES 1,D", &op::res_n_r8, 4, 2, {&d_reg}, 1}, {"RES 1,E", &op::res_n_r8, 4, 2, {&e_reg}, 1}, {"RES 1,H", &op::res_n_r8, 4, 2, {&h_reg}, 1}, {"RES 1,L", &op::res_n_r8, 4, 2, {&l_reg}, 1}, {"RES 1,(HL)", &op::res_n_r8, 12, 2, {&l_reg, &h_reg}, 1}, {"RES 1,A", &op::res_n_r8, 4, 2, {&a_reg}, 1},
            {"RES 2,B", &op::res_n_r8, 4, 2, {&b_reg}, 2}, {"RES 2,C", &op::res_n_r8, 4, 2, {&c_reg}, 2}, {"RES 2,D", &op::res_n_r8, 4, 2, {&d_reg}, 2}, {"RES 2,E", &op::res_n_r8, 4, 2, {&e_reg}, 2}, {"RES 2,H", &op::res_n_r8, 4, 2, {&h_reg}, 2}, {"RES 2,L", &op::res_n_r8, 4, 2, {&l_reg}, 2}, {"RES 2,(HL)", &op::res_n_r8, 12, 2, {&l_reg, &h_reg}, 2}, {"RES 2,A", &op::res_n_r8, 4, 2, {&a_reg}, 2}, {"RES 3,B", &op::res_n_r8, 4, 2, {&b_reg}, 3}, {"RES 3,C", &op::res_n_r8, 4, 2, {&c_reg}, 3}, {"RES 3,D", &op::res_n_r8, 4, 2, {&d_reg}, 3}, {"RES 3,E", &op::res_n_r8, 4, 2, {&e_reg}, 3}, {"RES 3,H", &op::res_n_r8, 4, 2, {&h_reg}, 3}, {"RES 3,L", &op::res_n_r8, 4, 2, {&l_reg}, 3}, {"RES 3,(HL)", &op::res_n_r8, 12, 2, {&l_reg, &h_reg}, 3}, {"RES 3,A", &op::res_n_r8, 4, 2, {&a_reg}, 3},
            {"RES 4,B", &op::res_n_r8, 4, 2, {&b_reg}, 4}, {"RES 4,C", &op::res_n_r8, 4, 2, {&c_reg}, 4}, {"RES 4,D", &op::res_n_r8, 4, 2, {&d_reg}, 4}, {"RES 4,E", &op::res_n_r8, 4, 2, {&e_reg}, 4}, {"RES 4,H", &op::res_n_r8, 4, 2, {&h_reg}, 4}, {"RES 4,L", &op::res_n_r8, 4, 2, {&l_reg}, 4}, {"RES 4,(HL)", &op::res_n_r8, 12, 2, {&l_reg, &h_reg}, 4}, {"RES 4,A", &op::res_n_r8, 4, 2, {&a_reg}, 4}, {"RES 5,B", &op::res_n_r8, 4, 2, {&b_reg}, 5}, {"RES 5,C", &op::res_n_r8, 4, 2, {&c_reg}, 5}, {"RES 5,D", &op::res_n_r8, 4, 2, {&d_reg}, 5}, {"RES 5,E", &op::res_n_r8, 4, 2, {&e_reg}, 5}, {"RES 5,H", &op::res_n_r8, 4, 2, {&h_reg}, 5}, {"RES 5,L", &op::res_n_r8, 4, 2, {&l_reg}, 5}, {"RES 5,(HL)", &op::res_n_r8, 12, 2, {&l_reg, &h_reg}, 5}, {"RES 5,A", &op::res_n_r8, 4, 2, {&a_reg}, 5},
            {"RES 6,B", &op::res_n_r8, 4, 2, {&b_reg}, 6}, {"RES 6,C", &op::res_n_r8, 4, 2, {&c_reg}, 6}, {"RES 6,D", &op::res_n_r8, 4, 2, {&d_reg}, 6}, {"RES 6,E", &op::res_n_r8, 4, 2, {&e_reg}, 6}, {"RES 6,H", &op::res_n_r8, 4, 2, {&h_reg}, 6}, {"RES 6,L", &op::res_n_r8, 4, 2, {&l_reg}, 6}, {"RES 6,(HL)", &op::res_n_r8, 12, 2, {&l_reg, &h_reg}, 6}, {"RES 6,A", &op::res_n_r8, 4, 2, {&a_reg}, 6}, {"RES 7,B", &op::res_n_r8, 4, 2, {&b_reg}, 7}, {"RES 7,C", &op::res_n_r8, 4, 2, {&c_reg}, 7}, {"RES 7,D", &op::res_n_r8, 4, 2, {&d_reg}, 7}, {"RES 7,E", &op::res_n_r8, 4, 2, {&e_reg}, 7}, {"RES 7,H", &op::res_n_r8, 4, 2, {&h_reg}, 7}, {"RES 7,L", &op::res_n_r8, 4, 2, {&l_reg}, 7}, {"RES 7,(HL)", &op::res_n_r8, 12, 2, {&l_reg, &h_reg}, 7}, {"RES 7,A", &op::res_n_r8, 4, 2, {&a_reg}, 7},
            {"SET 0,B", &op::set_n_r8, 4, 2, {&b_reg}, 0}, {"SET 0,C", &op::set_n_r8, 4, 2, {&c_reg}, 0}, {"SET 0,D", &op::set_n_r8, 4, 2, {&d_reg}, 0}, {"SET 0,E", &op::set_n_r8, 4, 2, {&e_reg}, 0}, {"SET 0,H", &op::set_n_r8, 4, 2, {&h_reg}, 0}, {"SET 0,L", &op::set_n_r8, 4, 2, {&l_reg}, 0}, {"SET 0,(HL)", &op::set_n_r8, 12, 2, {&l_reg, &h_reg}, 0}, {"SET 0,A", &op::set_n_r8, 4, 2, {&a_reg}, 0}, {"SET 1,B", &op::set_n_r8, 4, 2, {&b_reg}, 1}, {"SET 1,C", &op::set_n_r8, 4, 2, {&c_reg}, 1}, {"SET 1,D", &op::set_n_r8, 4, 2, {&d_reg}, 1}, {"SET 1,E", &op::set_n_r8, 4, 2, {&e_reg}, 1}, {"SET 1,H", &op::set_n_r8, 4, 2, {&h_reg}, 1}, {"SET 1,L", &op::set_n_r8, 4, 2, {&l_reg}, 1}, {"SET 1,(HL)", &op::set_n_r8, 12, 2, {&l_reg, &h_reg}, 1}, {"SET 1,A", &op::set_n_r8, 4, 2, {&a_reg}, 1},
            {"SET 2,B", &op::set_n_r8, 4, 2, {&b_reg}, 2}, {"SET 2,C", &op::set_n_r8, 4, 2, {&c_reg}, 2}, {"SET 2,D", &op::set_n_r8, 4, 2, {&d_reg}, 2}, {"SET 2,E", &op::set_n_r8, 4, 2, {&e_reg}, 2}, {"SET 2,H", &op::set_n_r8, 4, 2, {&h_reg}, 2}, {"SET 2,L", &op::set_n_r8, 4, 2, {&l_reg}, 2}, {"SET 2,(HL)", &op::set_n_r8, 12, 2, {&l_reg, &h_reg}, 2}, {"SET 2,A", &op::set_n_r8, 4, 2, {&a_reg}, 2}, {"SET 3,B", &op::set_n_r8, 4, 2, {&b_reg}, 3}, {"SET 3,C", &op::set_n_r8, 4, 2, {&c_reg}, 3}, {"SET 3,D", &op::set_n_r8, 4, 2, {&d_reg}, 3}, {"SET 3,E", &op::set_n_r8, 4, 2, {&e_reg}, 3}, {"SET 3,H", &op::set_n_r8, 4, 2, {&h_reg}, 3}, {"SET 3,L", &op::set_n_r8, 4, 2, {&l_reg}, 3}, {"SET 3,(HL)", &op::set_n_r8, 12, 2, {&l_reg, &h_reg}, 3}, {"SET 3,A", &op::set_n_r8, 4, 2, {&a_reg}, 3},
            {"SET 4,B", &op::set_n_r8, 4, 2, {&b_reg}, 4}, {"SET 4,C", &op::set_n_r8, 4, 2, {&c_reg}, 4}, {"SET 4,D", &op::set_n_r8, 4, 2, {&d_reg}, 4}, {"SET 4,E", &op::set_n_r8, 4, 2, {&e_reg}, 4}, {"SET 4,H", &op::set_n_r8, 4, 2, {&h_reg}, 4}, {"SET 4,L", &op::set_n_r8, 4, 2, {&l_reg}, 4}, {"SET 4,(HL)", &op::set_n_r8, 12, 2, {&l_reg, &h_reg}, 4}, {"SET 4,A", &op::set_n_r8, 4, 2, {&a_reg}, 4}, {"SET 5,B", &op::set_n_r8, 4, 2, {&b_reg}, 5}, {"SET 5,C", &op::set_n_r8, 4, 2, {&c_reg}, 5}, {"SET 5,D", &op::set_n_r8, 4, 2, {&d_reg}, 5}, {"SET 5,E", &op::set_n_r8, 4, 2, {&e_reg}, 5}, {"SET 5,H", &op::set_n_r8, 4, 2, {&h_reg}, 5}, {"SET 5,L", &op::set_n_r8, 4, 2, {&l_reg}, 5}, {"SET 5,(HL)", &op::set_n_r8, 12, 2, {&l_reg, &h_reg}, 5}, {"SET 5,A", &op::set_n_r8, 4, 2, {&a_reg}, 5},
            {"SET 6,B", &op::set_n_r8, 4, 2, {&b_reg}, 6}, {"SET 6,C", &op::set_n_r8, 4, 2, {&c_reg}, 6}, {"SET 6,D", &op::set_n_r8, 4, 2, {&d_reg}, 6}, {"SET 6,E", &op::set_n_r8, 4, 2, {&e_reg}, 6}, {"SET 6,H", &op::set_n_r8, 4, 2, {&h_reg}, 6}, {"SET 6,L", &op::set_n_r8, 4, 2, {&l_reg}, 6}, {"SET 6,(HL)", &op::set_n_r8, 12, 2, {&l_reg, &h_reg}, 6}, {"SET 6,A", &op::set_n_r8, 4, 2, {&a_reg}, 6}, {"SET 7,B", &op::set_n_r8, 4, 2, {&b_reg}, 7}, {"SET 7,C", &op::set_n_r8, 4, 2, {&c_reg}, 7}, {"SET 7,D", &op::set_n_r8, 4, 2, {&d_reg}, 7}, {"SET 7,E", &op::set_n_r8, 4, 2, {&e_reg}, 7}, {"SET 7,H", &op::set_n_r8, 4, 2, {&h_reg}, 7}, {"SET 7,L", &op::set_n_r8, 4, 2, {&l_reg}, 7}, {"SET 7,(HL)", &op::set_n_r8, 12, 2, {&l_reg, &h_reg}, 7}, {"SET 7,A", &op::set_n_r8, 4, 2, {&a_reg}, 7}
    };
}

SM83::~SM83() {

}

uint8_t SM83::read(uint16_t addr, bool bReadOnly) {
    return bus->read(addr, bReadOnly);
}

void SM83::write(uint16_t addr, uint8_t data) {
    bus->write(addr, data);
}

void SM83::clock() {
    // Only execute when the internal cycles count is 0
    if(cycles == 0) {
        // Read opcode - PC will be pointing to it
        opcode = read(pc);
        // Inc the PC to point to the next byte of the instruction
        pc++;
        // get the number of cycles for the instruction
        cycles = opcode_lookup[opcode].cycles;
        // Call the function pointer of the current opcode - Function returns the number of additional cycles req
        uint8_t additional_cycle = (this->*opcode_lookup[opcode].operate)();
        cycles += additional_cycle;
    }
    cycles--;
}
bool SM83::complete() {
    return cycles == 0;
}
// Returns 1 if the flag bit is set and 0 if reset
uint8_t SM83::getFlag(SM83_FLAGS f) {
    // If the specified bit is set return 1 else return 0
    return ((f_reg & f) > 0 ? 1 : 0);
}
// Sets or resets a specific flag bit in f_reg
void SM83::setFlag(SM83_FLAGS f, bool v) {
    if(v)
        f_reg |= f;     // Set the bit
    else
        f_reg &= ~f;    // Reset the bit
}

// Fetch the data located at addr_abs.
uint8_t SM83::fetch() {
    fetched = read(addr_abs);
    return fetched;
}
// Instructions in alphabetical order



// Add the A register with itself.
// Flags:
//  -Z: Set if the result of the addition is 0
//  -N: Reset to 0
//  -H: Set if the result of the addition enables bit 4
//  -C: Set if A overflows past 0xff
uint8_t SM83::add_a_a() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (a_reg & 0xf);
    // Overflow check
    uint16_t a_overflow = a_reg + a_reg;
    a_reg += a_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_overflow > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the data stored at the absolute address in HL with the A register and store in A.
// Flags:
//  -Z: Set if the result of the addition is 0
//  -N: Reset to 0
//  -H: Set if the result of the addition enables bit 4
//  -C: Set if A overflows past 0xff
uint8_t SM83::add_a_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (data & 0xf);
    // Overflow check
    uint16_t a_overflow = a_reg + data;
    a_reg += data;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_overflow > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the B register with the A register and store in A.
// Flags:
//  -Z: Set if the result of the addition is 0
//  -N: Reset to 0
//  -H: Set if the result of the addition enables bit 4
//  -C: Set if A overflows past 0xff
uint8_t SM83::add_a_b() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (b_reg & 0xf);
    // Overflow check
    uint16_t a_overflow = a_reg + b_reg;
    a_reg += b_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_overflow > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the C register with the A register and store in A.
// Flags:
//  -Z: Set if the result of the addition is 0
//  -N: Reset to 0
//  -H: Set if the result of the addition enables bit 4
//  -C: Set if A overflows past 0xff
uint8_t SM83::add_a_c() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (c_reg & 0xf);
    // Overflow check
    uint16_t a_overflow = a_reg + c_reg;
    a_reg += c_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_overflow > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the immediate 8-bit decimal value to A.
// Flags:
//  -Z: Set if the result of the addition is 0
//  -N: Reset to 0
//  -H: Set if the result of the addition enables bit 4
//  -C: Set if A overflows past 0xff
uint8_t SM83::add_a_d8() {
    // Get the data
    uint8_t data = read(pc++);
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (data & 0xf);
    // Overflow check
    uint16_t a_overflow = a_reg + data;
    a_reg += data;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_overflow > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the D register with the A register and store in A.
// Flags:
//  -Z: Set if the result of the addition is 0
//  -N: Reset to 0
//  -H: Set if the result of the addition enables bit 4
//  -C: Set if A overflows past 0xff
uint8_t SM83::add_a_d() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (d_reg & 0xf);
    // Overflow check
    uint16_t a_overflow = a_reg + d_reg;
    a_reg += d_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_overflow > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the E register with the A register and store in A.
// Flags:
//  -Z: Set if the result of the addition is 0
//  -N: Reset to 0
//  -H: Set if the result of the addition enables bit 4
//  -C: Set if A overflows past 0xff
uint8_t SM83::add_a_e() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (e_reg & 0xf);
    // Overflow check
    uint16_t a_overflow = a_reg + e_reg;
    a_reg += e_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_overflow > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the H register with the A register and store in A.
// Flags:
//  -Z: Set if the result of the addition is 0
//  -N: Reset to 0
//  -H: Set if the result of the addition enables bit 4
//  -C: Set if A overflows past 0xff
uint8_t SM83::add_a_h() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (h_reg & 0xf);
    // Overflow check
    uint16_t a_overflow = a_reg + h_reg;
    a_reg += h_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_overflow > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the L register with the A register and store in A.
// Flags:
//  -Z: Set if the result of the addition is 0
//  -N: Reset to 0
//  -H: Set if the result of the addition enables bit 4
//  -C: Set if A overflows past 0xff
uint8_t SM83::add_a_l() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (l_reg & 0xf);
    // Overflow check
    uint16_t a_overflow = a_reg + l_reg;
    a_reg += l_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_overflow > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the BC register pair into the HL register pair.
// Flags:
//  -N: Reset to 0
//  -H: Set if overflow from bit 11, if H overflows, or if H = 0x10 (This was pretty hard to figure out)
//  -C: Set to 1 if overflow from bit 15
uint8_t SM83::add_hl_bc() {
    // Create a 16-bit copy of the H register
    uint16_t h_16 = h_reg;
    // Create copy of H before addition - Need for when H overflows from L overflow i.e H = 0xff
    // before carry from L.
    uint8_t h_old = h_reg;
    // Calculate the overflow and half carry flags
    uint16_t l_overflow = l_reg + c_reg;
    l_reg += c_reg;
    if(l_overflow > 0xff) {
        // Increment both h_reg and the 16-bit version
        h_reg++;
        h_16++;
    }
    // Check for half carry flag
    uint8_t h_check = (h_reg & 0xf) + (b_reg & 0xf);
    uint8_t h_old_check = (h_old & 0xf) + (b_reg & 0xf);
    // Add the two versions of H with B
    h_16 += b_reg;
    h_reg += b_reg;
    // Now run the check for the half carry flag

    // To break down this check:
    // First we need to have the state of the H register before we have modified it for this
    // function. This is due to the way uint8_t variables work. When their value goes over 0xff,
    // they will wrap around to 0. This is a problem when checking for the half carry if the result
    // of the addition should cause the half carry to trigger after it has wrapped around, like when adding
    // 0xff + 0xff. We then do a check on overflow from bit 3 to 4 on the new and old h_reg (before and after overflow
    // from L). We also check if h_reg == 0 as this covers the case where overflow from L causes h_reg == 0x10.
    //-----------------------------------------------------------------------------------------------------------
    // Note: h_reg != h_old ONLY when l_reg has overflow. This means h_old_check will result the same as h_check
    // if there is no L overflow, and that means there should not be any unexpected results.
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || h_reg == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Check overflow of HL pair
    if(h_16 > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the DE register pair into the HL register pair.
// Flags:
//  -N: Reset to 0
//  -H: Set if overflow from bit 11, if H overflows, or if H = 0x10 (This was pretty hard to figure out)
//  -C: Set to 1 if overflow from bit 15
uint8_t SM83::add_hl_de() {
    // Create a 16-bit copy of the H register
    uint16_t h_16 = h_reg;
    // Create copy of H before addition - Need for when H overflows from L overflow i.e H = 0xff
    // before carry from L.
    uint8_t h_old = h_reg;
    // Calculate the overflow and half carry flags
    uint16_t l_overflow = l_reg + e_reg;
    l_reg += e_reg;
    if(l_overflow > 0xff) {
        // Increment both h_reg and the 16-bit version
        h_reg++;
        h_16++;
    }
    // Check for half carry flag
    uint8_t h_check = (h_reg & 0xf) + (d_reg & 0xf);
    uint8_t h_old_check = (h_old & 0xf) + (d_reg & 0xf);
    // Add the two versions of H with D
    h_16 += d_reg;
    h_reg += d_reg;
    // Now run the check for the half carry flag
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || h_reg == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Check overflow of HL pair
    if(h_16 > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the HL register pair into itself.
// Flags:
//  - N: Reset to 0
//  - H: Set to 1 if overflow from bit 11
//  - C: Set to 1 if overflow from bit 15
uint8_t SM83::add_hl_hl() {
    // Need to create copies of each of these before the increments.
    uint8_t l_old = l_reg;
    uint8_t h_old = h_reg;
    // Copy of H register
    uint16_t h_16 = h_reg;
    // Check for L register overflow
    uint16_t l_overflow = l_reg + l_old;
    l_reg += l_old;
    if(l_overflow > 0xff) {
        h_reg++;
        h_16++;
    }

    // Used to check for half carry
    uint8_t h_check = ((h_reg & 0xf) + (h_old & 0xf));
    uint8_t h_old_check = (h_old & 0xf) + (h_old & 0xf);
    // Used to check for carry
    h_16 += h_old;
    h_reg += h_old;
    // Check if half carry needs to be enabled
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || h_reg == 0x10 )
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Check if carry needs to be enabled
    if(h_16 > 0xFF)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Add the 16 bit SP to the 16-bit register pair HL.
// Note: Probably need to do more testing, but I am pretty
// sure that this is a pretty good setup.

// Flags:
//  -N: Reset to 0
//  -H: Set if overflow from bit 11, if H overflows, or if H = 0x10 (This was pretty hard to figure out)
//  -C: Set of H overflows from bit 15
uint8_t SM83::add_hl_sp() {
    // Create temporary variables for the H register
    uint16_t h_16 = h_reg;
    // Create copy of H before addition - Need for when H overflows from L overflow i.e H = 0xff
    // before carry from L.
    uint8_t h_old = h_reg;
    // Read in the value from the SP
    uint8_t lowByte = sp;
    uint8_t highByte = (sp >> 8);

    // Check for L overflow
    uint16_t l_overflow = l_reg + lowByte;
    l_reg += lowByte;
    if(l_overflow > 0xff) {
        h_16++;
        h_reg++;
    }
    // Check for half carry
    uint8_t h_check = (h_reg &0xf) + (highByte & 0xf);
    uint8_t h_old_check = (h_old & 0xf) + (highByte & 0xf);
    h_16 += highByte;
    h_reg += highByte;
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || h_reg == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Check for overflow
    if(h_16 > 0xff) {
        setFlag(C, 1);
    }
    else
        setFlag(C, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Add immediate signed 8-bit data to SP.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Set if bit 3 overflows
//  -C: Set if bit 7 overflows
uint8_t SM83::add_sp_r8() {
    // SP is an uint16_t
    // Need to read in the value as a signed int
    uint8_t data = read(pc++);
    // Need to have a signed version of the data
    int8_t sData = data;
    uint8_t h_check = (sp & 0x000f) + (data & 0xf);
    uint16_t overflow = (sp & 0x00ff) + data;
    // Add the signed data
    sp += sData;
    // Check for half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Check for carry flag
    if(overflow > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the zero and sign flags
    setFlag(Z, 0);
    setFlag(N, 0);
    return 0;
}

// Add the A register to itself with carry.
// Flags:
//  -Z: If the result is 0
//  -N: Reset to 0
//  -H: Set if the addition sets bit 4 or if the carry sets bit 4
//  -C: Set if A overflows
uint8_t SM83::adc_a_a() {
    // Create a 16-bit copy of A
    uint16_t a_16 = a_reg;
    uint8_t a_old = a_reg;
    // Need to see if the carry flag is enabled and inc A if so
    if(getFlag(C) == 1) {
        a_reg++;
        a_16++;
    }
    // Used in half carry check to see if the carry being added to A causes it to be 0x10
    uint8_t a_h10 = a_reg;
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (a_old & 0xf);
    uint8_t h_old_check = (a_old & 0xf) + (a_old & 0xf);
    a_reg += a_old;
    a_16 += a_old;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || a_h10 == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_16 > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the data stored at the absolute address in HL to the A register with carry.
// Flags:
//  -Z: If the result is 0
//  -N: Reset to 0
//  -H: Set if the addition sets bit 4 or if the carry sets bit 4
//  -C: Set if A overflows
uint8_t SM83::adc_a_abs_hl() {
    // Load the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch the data
    uint8_t data = fetch();

    // Create a 16-bit copy of A
    uint16_t a_16 = a_reg;
    uint8_t a_old = a_reg;
    // Need to see if the carry flag is enabled and inc A if so
    if(getFlag(C) == 1) {
        a_reg++;
        a_16++;
    }
    // Used in half carry check to see if the carry being added to A causes it to be 0x10
    uint8_t a_h10 = a_reg;
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (data & 0xf);
    uint8_t h_old_check = (a_old & 0xf) + (data & 0xf);
    a_reg += data;
    a_16 += data;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || a_h10 == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_16 > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the B register with the A register plus the carry flag.
// Flags:
//  -Z: If the result is 0
//  -N: Reset to 0
//  -H: Set if the addition sets bit 4 or if the carry sets bit 4
//  -C: Set if A overflows
uint8_t SM83::adc_a_b() {
    // Create a 16-bit copy of A
    uint16_t a_16 = a_reg;
    uint8_t a_old = a_reg;
    // Need to see if the carry flag is enabled and inc A if so
    if(getFlag(C) == 1) {
        a_reg++;
        a_16++;
    }
    // Used in half carry check to see if the carry being added to A causes it to be 0x10
    uint8_t a_h10 = a_reg;
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (b_reg & 0xf);
    uint8_t h_old_check = (a_old & 0xf) + (b_reg & 0xf);
    a_reg += b_reg;
    a_16 += b_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || a_h10 == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_16 > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the C register with the A register plus the carry flag.
// Flags:
//  -Z: If the result is 0
//  -N: Reset to 0
//  -H: Set if the addition sets bit 4 or if the carry sets bit 4
//  -C: Set if A overflows
uint8_t SM83::adc_a_c() {
    // Create a 16-bit copy of A
    uint16_t a_16 = a_reg;
    uint8_t a_old = a_reg;
    // Need to see if the carry flag is enabled and inc A if so
    if(getFlag(C) == 1) {
        a_reg++;
        a_16++;
    }
    // Used in half carry check to see if the carry being added to A causes it to be 0x10
    uint8_t a_h10 = a_reg;
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (c_reg & 0xf);
    uint8_t h_old_check = (a_old & 0xf) + (c_reg & 0xf);
    a_reg += c_reg;
    a_16 += c_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || a_h10 == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_16 > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the D register with the A register plus the carry flag.
// Flags:
//  -Z: If the result is 0
//  -N: Reset to 0
//  -H: Set if the addition sets bit 4 or if the carry sets bit 4
//  -C: Set if A overflows
uint8_t SM83::adc_a_d() {
    // Create a 16-bit copy of A
    uint16_t a_16 = a_reg;
    uint8_t a_old = a_reg;
    // Need to see if the carry flag is enabled and inc A if so
    if(getFlag(C) == 1) {
        a_reg++;
        a_16++;
    }
    // Used in half carry check to see if the carry being added to A causes it to be 0x10
    uint8_t a_h10 = a_reg;
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (d_reg & 0xf);
    uint8_t h_old_check = (a_old & 0xf) + (d_reg & 0xf);
    a_reg += d_reg;
    a_16 += d_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || a_h10 == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_16 > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the immediate 8-bit data with the A register with carry.
// Flags:
//  -Z: If the result is 0
//  -N: Reset to 0
//  -H: Set if the addition sets bit 4 or if the carry sets bit 4
//  -C: Set if A overflows
uint8_t SM83::adc_a_d8() {
    // Get the data
    uint8_t data = read(pc++);
    // Create a 16-bit copy of A
    uint16_t a_16 = a_reg;
    uint8_t a_old = a_reg;
    // Need to see if the carry flag is enabled and inc A if so
    if(getFlag(C) == 1) {
        a_reg++;
        a_16++;
    }
    // Used in half carry check to see if the carry being added to A causes it to be 0x10
    uint8_t a_h10 = a_reg;
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (data & 0xf);
    uint8_t h_old_check = (a_old & 0xf) + (data & 0xf);
    a_reg += data;
    a_16 += data;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || a_h10 == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_16 > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the E register with the A register plus the carry flag.
// Flags:
//  -Z: If the result is 0
//  -N: Reset to 0
//  -H: Set if the addition sets bit 4 or if the carry sets bit 4
//  -C: Set if A overflows
uint8_t SM83::adc_a_e() {
    // Create a 16-bit copy of A
    uint16_t a_16 = a_reg;
    uint8_t a_old = a_reg;
    // Need to see if the carry flag is enabled and inc A if so
    if(getFlag(C) == 1) {
        a_reg++;
        a_16++;
    }
    // Used in half carry check to see if the carry being added to A causes it to be 0x10
    uint8_t a_h10 = a_reg;
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (e_reg & 0xf);
    uint8_t h_old_check = (a_old & 0xf) + (e_reg & 0xf);
    a_reg += e_reg;
    a_16 += e_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || a_h10 == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_16 > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the H register with the A register plus the carry flag.
// Flags:
//  -Z: If the result is 0
//  -N: Reset to 0
//  -H: Set if the addition sets bit 4 or if the carry sets bit 4
//  -C: Set if A overflows
uint8_t SM83::adc_a_h() {
    // Create a 16-bit copy of A
    uint16_t a_16 = a_reg;
    uint8_t a_old = a_reg;
    // Need to see if the carry flag is enabled and inc A if so
    if(getFlag(C) == 1) {
        a_reg++;
        a_16++;
    }
    // Used in half carry check to see if the carry being added to A causes it to be 0x10
    uint8_t a_h10 = a_reg;
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (h_reg & 0xf);
    uint8_t h_old_check = (a_old & 0xf) + (h_reg & 0xf);
    a_reg += h_reg;
    a_16 += h_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || a_h10 == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_16 > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Add the contents of the L register with the A register plus the carry flag.
// Flags:
//  -Z: If the result is 0
//  -N: Reset to 0
//  -H: Set if the addition sets bit 4 or if the carry sets bit 4
//  -C: Set if A overflows
uint8_t SM83::adc_a_l() {
    // Create a 16-bit copy of A
    uint16_t a_16 = a_reg;
    uint8_t a_old = a_reg;
    // Need to see if the carry flag is enabled and inc A if so
    if(getFlag(C) == 1) {
        a_reg++;
        a_16++;
    }
    // Used in half carry check to see if the carry being added to A causes it to be 0x10
    uint8_t a_h10 = a_reg;
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) + (l_reg & 0xf);
    uint8_t h_old_check = (a_old & 0xf) + (l_reg & 0xf);
    a_reg += l_reg;
    a_16 += l_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (h_old_check & 0x10) == 0x10 || a_h10 == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Carry check
    if(a_16 > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// And the A register with itself.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Set to 1
//  -C: Reset to 0
uint8_t SM83::and_a() {
    a_reg &= a_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set the half carry flag
    setFlag(H, 1);
    // Reset the sign and carry flag
    setFlag(N, 0);
    setFlag(C, 0);
    return 0;
}

// And the contents of the A register with the data stored in the absolute address in HL. Store the result in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Set to 1
//  -C: Reset to 0
uint8_t SM83::and_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store the data
    uint8_t data = fetch();
    a_reg &= data;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set the half carry flag
    setFlag(H, 1);
    // Reset the sign and carry flag
    setFlag(N, 0);
    setFlag(C, 0);
    return 0;
}

// And the contents of the A and B registers. Store the result in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Set to 1
//  -C: Reset to 0
uint8_t SM83::and_b() {
    a_reg &= b_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set the half carry flag
    setFlag(H, 1);
    // Reset the sign and carry flag
    setFlag(N, 0);
    setFlag(C, 0);
    return 0;
}

// And the contents of the A and C registers. Store the result in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Set to 1
//  -C: Reset to 0
uint8_t SM83::and_c() {
    a_reg &= c_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set the half carry flag
    setFlag(H, 1);
    // Reset the sign and carry flag
    setFlag(N, 0);
    setFlag(C, 0);
    return 0;
}

// And the contents of the A and D registers. Store the result in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Set to 1
//  -C: Reset to 0
uint8_t SM83::and_d() {
    a_reg &= d_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set the half carry flag
    setFlag(H, 1);
    // Reset the sign and carry flag
    setFlag(N, 0);
    setFlag(C, 0);
    return 0;
}

// And the contents of the A register with the immediate 8-bit data. Store the result in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Set to 1
//  -C: Reset to 0
uint8_t SM83::and_d8() {
    // Need to get the data
    uint8_t data = read(pc++);
    a_reg &= data;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set the half carry flag
    setFlag(H, 1);
    // Reset the sign and carry flag
    setFlag(N, 0);
    setFlag(C, 0);
    return 0;
}

// And the contents of the A and E registers. Store the result in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Set to 1
//  -C: Reset to 0
uint8_t SM83::and_e() {
    a_reg &= e_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set the half carry flag
    setFlag(H, 1);
    // Reset the sign and carry flag
    setFlag(N, 0);
    setFlag(C, 0);
    return 0;
}

// And the contents of the A and H registers. Store the result in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Set to 1
//  -C: Reset to 0
uint8_t SM83::and_h() {
    a_reg &= h_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set the half carry flag
    setFlag(H, 1);
    // Reset the sign and carry flag
    setFlag(N, 0);
    setFlag(C, 0);
    return 0;
}

// And the contents of the A and L registers. Store the result in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Set to 1
//  -C: Reset to 0
uint8_t SM83::and_l() {
    a_reg &= l_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set the half carry flag
    setFlag(H, 1);
    // Reset the sign and carry flag
    setFlag(N, 0);
    setFlag(C, 0);
    return 0;
}

// Check bit 0 in register A.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_0_a() {
    // Check for zero flag
    if((a_reg & 0x01) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 0 in data stored at the absolute address in HL.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_0_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Check for zero flag
    if((data & 0x01) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 0 in register B.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_0_b() {
    // Check for zero flag
    if((b_reg & 0x01) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 0 in register C.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_0_c() {
    // Check for zero flag
    if((c_reg & 0x01) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 0 in register D.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_0_d() {
    // Check for zero flag
    if((d_reg & 0x01) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 0 in register E.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_0_e() {
    // Check for zero flag
    if((e_reg & 0x01) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 0 in register H.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_0_h() {
    // Check for zero flag
    if((h_reg & 0x01) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 0 in register L.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_0_l() {
    // Check for zero flag
    if((l_reg & 0x01) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 1 in register A.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_1_a() {
    // Check for zero flag
    if((a_reg & (1 << 1)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 1 in data stored at the absolute address in HL.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_1_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Check for zero flag
    if((data & (1 << 1)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 1 in register B.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_1_b() {
    // Check for zero flag
    if((b_reg & (1 << 1)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 1 in register C.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_1_c() {
    // Check for zero flag
    if((c_reg & (1 << 1)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 1 in register D.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_1_d() {
    // Check for zero flag
    if((d_reg & (1 << 1)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 1 in register E.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_1_e() {
    // Check for zero flag
    if((e_reg & (1 << 1)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 1 in register H.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_1_h() {
    // Check for zero flag
    if((h_reg & (1 << 1)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 1 in register L.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_1_l() {
    // Check for zero flag
    if((l_reg & (1 << 1)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 2 in register A.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_2_a() {
    // Check for zero flag
    if((a_reg & (1 << 2)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 2 in the data stored at the absolute address in HL.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_2_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Check for zero flag
    if((data & (1 << 2)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 2 in register B.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_2_b() {
    // Check for zero flag
    if((b_reg & (1 << 2)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 2 in register C.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_2_c() {
    // Check for zero flag
    if((c_reg & (1 << 2)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 2 in register D.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_2_d() {
    // Check for zero flag
    if((d_reg & (1 << 2)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 2 in register E.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_2_e() {
    // Check for zero flag
    if((e_reg & (1 << 2)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 2 in register H.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_2_h() {
    // Check for zero flag
    if((h_reg & (1 << 2)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 2 in register L.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_2_l() {
    // Check for zero flag
    if((l_reg & (1 << 2)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 3 in register A.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_3_a() {
    // Check for zero flag
    if((a_reg & (1 << 3)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 3 in the data stored at the absolute address in HL.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_3_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Check for zero flag
    if((data & (1 << 3)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 3 in register B.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_3_b() {
    // Check for zero flag
    if((b_reg & (1 << 3)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 3 in register C.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_3_c() {
    // Check for zero flag
    if((c_reg & (1 << 3)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 3 in register D.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_3_d() {
    // Check for zero flag
    if((d_reg & (1 << 3)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 3 in register E.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_3_e() {
    // Check for zero flag
    if((e_reg & (1 << 3)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 3 in register H.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_3_h() {
    // Check for zero flag
    if((h_reg & (1 << 3)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 3 in register L.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_3_l() {
    // Check for zero flag
    if((l_reg & (1 << 3)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 4 in register A.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_4_a() {
    // Check for zero flag
    if((a_reg & (1 << 4)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 4 in the data stored at the absolute address in HL.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_4_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Check for zero flag
    if((data & (1 << 4)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 4 in register B.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_4_b() {
    // Check for zero flag
    if((b_reg & (1 << 4)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 4 in register C.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_4_c() {
    // Check for zero flag
    if((c_reg & (1 << 4)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 4 in register D.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_4_d() {
    // Check for zero flag
    if((d_reg & (1 << 4)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 4 in register E.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_4_e() {
    // Check for zero flag
    if((e_reg & (1 << 4)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 4 in register H.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_4_h() {
    // Check for zero flag
    if((h_reg & (1 << 4)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 4 in register L.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_4_l() {
    // Check for zero flag
    if((l_reg & (1 << 4)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 5 in register A.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_5_a() {
    // Check for zero flag
    if((a_reg & (1 << 5)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 5 in the data stored ath the absolute address in HL.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_5_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Check for zero flag
    if((data & (1 << 5)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 5 in register B.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_5_b() {
    // Check for zero flag
    if((b_reg & (1 << 5)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 5 in register C.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_5_c() {
    // Check for zero flag
    if((c_reg & (1 << 5)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 5 in register D.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_5_d() {
    // Check for zero flag
    if((d_reg & (1 << 5)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 5 in register E.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_5_e() {
    // Check for zero flag
    if((e_reg & (1 << 5)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 5 in register H.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_5_h() {
    // Check for zero flag
    if((h_reg & (1 << 5)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 5 in register L.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_5_l() {
    // Check for zero flag
    if((l_reg & (1 << 5)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 6 in register A.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_6_a() {
    // Check for zero flag
    if((a_reg & (1 << 6)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 6 in the data stored at the absolute address in HL.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_6_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte  << 8) | lowByte;
    uint8_t data = fetch();
    // Check for zero flag
    if((data & (1 << 6)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 6 in register B.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_6_b() {
    // Check for zero flag
    if((b_reg & (1 << 6)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 6 in register C.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_6_c() {
    // Check for zero flag
    if((c_reg & (1 << 6)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 6 in register D.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_6_d() {
    // Check for zero flag
    if((d_reg & (1 << 6)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 6 in register E.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_6_e() {
    // Check for zero flag
    if((e_reg & (1 << 6)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 6 in register H.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_6_h() {
    // Check for zero flag
    if((h_reg & (1 << 6)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 6 in register L.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_6_l() {
    // Check for zero flag
    if((l_reg & (1 << 6)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 7 in register A.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_7_a() {
    // Check for zero flag
    if((a_reg & (1 << 7)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 7 in the data stored in the absolute address in HL.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_7_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Check for zero flag
    if((data & (1 << 7)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 7 in register B.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_7_b() {
    // Check for zero flag
    if((b_reg & (1 << 7)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 7 in register C.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_7_c() {
    // Check for zero flag
    if((c_reg & (1 << 7)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 7 in register D.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_7_d() {
    // Check for zero flag
    if((d_reg & (1 << 7)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 7 in register E.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_7_e() {
    // Check for zero flag
    if((e_reg & (1 << 7)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 7 in register H.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_7_h() {
    // Check for zero flag
    if((h_reg & (1 << 7)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Check bit 7 in register L.
// Flags:
//  -Z: Set if bit 0 is not set.
//  -N: Reset to 0
//  -H: Set to 1
uint8_t SM83::bit_7_l() {
    // Check for zero flag
    if((l_reg & (1 << 7)) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Set half carry flag
    setFlag(H, 1);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Push the address of the next instruction to the SP, then update the PC
// with the 16-bit absolute address.
uint8_t SM83::call_a16() {
    // Need to store the address
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    addr_abs = (highByte << 8) | lowByte;
    // Push the current PC to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Update the PC
    pc = addr_abs;
    return 0;
}

// Push the address of the next instruction to the SP, then update the PC
// with the 16-bit absolute address only if the carry flag is set.
uint8_t SM83::call_c_a16() {
    // Need to store the address
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    // Check if the carry flag is not set
    if(!getFlag(C))
        return 0;
    addr_abs = (highByte << 8) | lowByte;
    // Push the current PC to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Update the PC
    pc = addr_abs;
    return 12;
}

// Push the address of the next instruction to the SP, then update the PC
// with the 16-bit absolute address only if the carry flag is not set.
uint8_t SM83::call_nc_a16() {
    // Need to store the address
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    // Check if the carry flag is set
    if(getFlag(C))
        return 0;
    addr_abs = (highByte << 8) | lowByte;
    // Push the current PC to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Update the PC
    pc = addr_abs;
    return 12;
}

// Push the address of the next instruction to the SP, then update the PC
// with the 16-bit absolute address only if the zero flag is not set.
uint8_t SM83::call_nz_a16() {
    // Need to store the address
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    // Check if the zero flag is set
    if(getFlag(Z))
        return 0;
    addr_abs = (highByte << 8) | lowByte;
    // Push the current PC to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Update the PC
    pc = addr_abs;
    return 12;
}

// Push the address of the next instruction to the SP, then update the PC
// with the 16-bit absolute address only if the zero flag is set.
uint8_t SM83::call_z_16() {
    // Need to store the address
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    // Check if the zero flag is not set
    if(!getFlag(Z))
        return 0;
    addr_abs = (highByte << 8) | lowByte;
    // Push the current PC to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Update the PC
    pc = addr_abs;
    return 12;
}

// Complement (invert) the carry flag.
uint8_t SM83::ccf() {
    if(getFlag(C) == 1)
        setFlag(C, 0);
    else
        setFlag(C, 1);
    return 0;
}

// CP commands are the same as sub, they just do not store the result in A.

// Subtract the A register from itself, but do not store the value.
// Flags:
//  -Z: Set to 1
//  -N: Set to 1
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::cp_a() {
    // Set the zero and sign flag
    setFlag(Z, 1);
    setFlag(N, 1);
    // Reset the half carry and carry flags
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Subtract the value of the data stored at the absolute address in HL from the A register, but do not store the result.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set after the subtraction
//  -C: Set if B > A
uint8_t SM83::cp_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store
    uint8_t data = fetch();
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (data & 0xf);
    // Check for carry flag
    if(data > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Zero flag check
    if((a_reg - data) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the B register from the A register, but do not store the result.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set after the subtraction
//  -C: Set if B > A
uint8_t SM83::cp_b() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (b_reg & 0xf);
    // Check for carry flag
    if(b_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Zero flag check
    if((a_reg - b_reg) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the C register from the A register, but do not store the result.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set after the subtraction
//  -C: Set if C > A
uint8_t SM83::cp_c() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (c_reg & 0xf);
    // Check for carry flag
    if(c_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Zero flag check
    if((a_reg - c_reg) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the D register from the A register, but do not store the result.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set after the subtraction
//  -C: Set if D > A
uint8_t SM83::cp_d() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (d_reg & 0xf);
    // Check for carry flag
    if(d_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Zero flag check
    if((a_reg - d_reg) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}
// Subtract th immediate 8-bit data from the A register, but do not store the result.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set after the subtraction
//  -C: Set if d8 > A
uint8_t SM83::cp_d8() {
    // Need to get the data
    uint8_t data = read(pc++);
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (data & 0xf);
    // Check for carry flag
    if(data > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Zero flag check
    if((a_reg - data) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the E register from the A register, but do not store the result.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set after the subtraction
//  -C: Set if E > A
uint8_t SM83::cp_e() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (e_reg & 0xf);
    // Check for carry flag
    if(e_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Zero flag check
    if((a_reg - e_reg) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the H register from the A register, but do not store the result.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set after the subtraction
//  -C: Set if H > A
uint8_t SM83::cp_h() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (h_reg & 0xf);
    // Check for carry flag
    if(h_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Zero flag check
    if((a_reg - h_reg) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the L register from the A register, but do not store the result.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set after the subtraction
//  -C: Set if L > A
uint8_t SM83::cp_l() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (l_reg & 0xf);
    // Check for carry flag
    if(l_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Zero flag check
    if((a_reg - l_reg) == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Get the complement of the A register, then save that into A.
// The complement of A is A with all of its bits flipped.
// Flag:
//  -N: Set to 1
//  -H: Set to 1
uint8_t SM83::cpl() {
    a_reg = ~a_reg;
    // Set sign and half carry flags
    setFlag(N, 1);
    setFlag(H, 1);
    return 0;
}

// This instruction is meant to run after an addition or subtraction on the A register.
// It is meant to adjust the value in A to conform with BCD rules.
// This article online was a great help with this particular instruction
// as it simplified BCD for me, and you can see how its influence is in my code.
// The code provided was written in Rust (I think) and I have changed it to work in C++,
// and adjusted some other things like setting/resetting flags.
// Link: https://ehaskins.com/2018-01-30%20Z80%20DAA/
// Flags:
//  -Z: Set if the value in A is 0
//  -H: Reset to 0
//  -C: Set if the value of A > 0x99 and subtraction is not being performed for adjustment
uint8_t SM83::daa() {
    // Need to add or subtract 6 when the value in each nibble is > 0x9 for least
    // significant, and 0x9 for the most significant
    int8_t correction = 0;      // Note this is a signed int
    // Check to see if the half carry flag is enabled, or the sign flag is disabled
    // and the value of the lower nibble exceeds 0x9 in register A
    if(getFlag(H) || (!getFlag(N) && (a_reg & 0xf) > 0x9))
        correction |= 0x6;
    // Check to see if the carry flag is enabled, or the sign flag is disabled
    // and the value of a_reg is greater than 0x99 (this means the high nibble is > 0x9
    if(getFlag(C) || (!getFlag(N) && a_reg > 0x99)) {
        correction |= 0x60;
        setFlag(C, 1);
    }
    else
        setFlag(C, 0);
    // Add or subtract based on sign flag
    a_reg += getFlag(N) ? -correction : correction;
    // Check zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset half carry
    setFlag(H, 0);
    return 0;
}
// Decrement the A register. Set according flags.
// Flags:
//  - Z: If result is 0
//  - N: Gets set to 1
//  - H: If bit 4 is set after the subtraction
uint8_t SM83::dec_a() {

    // Used to check half carry flag
    uint8_t h_check = ((a_reg & 0xf) - (1 & 0xf));
    a_reg--;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check for half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    return 0;
}

// Using HL as an absolute address, decrement the data at that address.
// Flag:
//  -Z: Set if result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set after decrementing the data
uint8_t SM83::dec_abs_hl() {
    // Load the data that HL is pointing to
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch data
    uint8_t data = fetch();
    // Used to check half carry flag
    uint8_t h_check = (data & 0xf) - (1 &0xf);
    data--;
    // Check zero flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry flag
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    write(addr_abs, data);
    return 0;
}

// Decrement the B register. Set according flags.
// Flags:
//  - Z: If result is 0
//  - N: Gets set to 1
//  - H: If bit 4 is set after the subtraction
uint8_t SM83::dec_b() {
    // Used to check half carry flag
    uint8_t h_check = ((b_reg & 0xf) - (1 & 0xf));
    b_reg--;
    if(b_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check to see if bit 4 was set due to our subtraction
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);

    setFlag(N, 1);
    return 0;
}

// Decrement the C register. Set according flags.
// Flags:
//  - Z: If result is 0
//  - N: Gets set to 1
//  - H: If bit 4 is set after the subtraction
uint8_t SM83::dec_c() {
    // Used to check half carry flag
    uint8_t h_check = ((c_reg & 0xf) - (1 & 0xf));
    c_reg--;
    // Check zero flag
    if(c_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry flag
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    return 0;
}

// Decrement the D register. Set according flags.
// Flags:
//  - Z: If result is 0
//  - N: Gets set to 1
//  - H: If bit 4 is set after the subtraction
uint8_t SM83::dec_d() {
    // Used to check half carry flag
    uint8_t h_check = ((d_reg & 0xf) - (1 & 0xf));
    d_reg--;
    // Check for zero flag
    if(d_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check for half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    return 0;
}

// Decrement the BC register pair.
uint8_t SM83::dec_bc() {
    if(c_reg == 0x00)
        b_reg--;
    c_reg--;
    return 0;
}

// Decrement the DE register pair.
uint8_t SM83::dec_de() {
    if(e_reg == 0x00)
        d_reg--;
    e_reg--;
    return 0;
}

// Decrement the E register.
// Flag:
//  -Z: Set if result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set to 1 after decrement

uint8_t SM83::dec_e() {
    // Used to check half carry
    uint8_t h_check = ((e_reg & 0xf) - (1 & 0xf));
    e_reg--;
    // Check zero flag
    if(e_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry flag
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    return 0;
}

// Decrement the H register.
// Flag:
//  -Z: Set if result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set to 1 after decrement
uint8_t SM83::dec_h() {
// Used to check half carry
    uint8_t h_check = ((h_reg & 0xf) - (1 & 0xf));
    h_reg--;
    // Check zero flag
    if(h_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry flag
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    return 0;
}

// Decrement the HL register pair.
uint8_t SM83::dec_hl() {
    if(l_reg == 0x00)
        h_reg--;
    l_reg--;
    return 0;
}

// Decrement the L register.
// Flag:
//  -Z: Set if result is 0
//  -N: Set to 1
//  -H: Set if bit 4 is set to 1 after decrement
uint8_t SM83::dec_l() {
// Used to check half carry
    uint8_t h_check = ((l_reg & 0xf) - (1 & 0xf));
    l_reg--;
    // Check zero flag
    if(l_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry flag
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set sign flag
    setFlag(N, 1);
    return 0;
}

// Decrement the SP.
uint8_t SM83::dec_sp() {
    // Since the SP is an uint16_t, there is no need to check for overflow, there is no need to check for overflow.
    sp--;
    return 0;
}

// TODO: Need to implement it after finished with interrupts
// Disable interrupts
uint8_t SM83::di() {

    return 0;
}

// TODO: Need to implement it after finished with interrupts
// Enable interrupts
uint8_t SM83::ei() {

    return 0;
}

// TODO: Need to finish this after interrupts have been implemented.
// Halt the system clock until an interrupt occurs.
uint8_t SM83::halt() {

    return 0;
}

// Increment the A register.
// Flags:
//  -Z: Set this flag to 1 if result is 0
//  -N: Reset this flag to 0
//  -H: Set this flag to 1 if bit 4 is set after the increment
uint8_t SM83::inc_a() {
    // Used for the half carry bit
    // By &ing the A register with 0xf, we reset the high nibble. Same for 1, but the high nibble
    // for 1 is never set. It is written out here for clarity.
    // When we add these two numbers together, we can check bit 4.
    // If bit 4 is enabled, that means this addition should set the half carry bit.
    uint8_t h_check = (a_reg & 0xf) + (1 & 0xf);
    a_reg++;
    // Check for zero
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check for half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset sign
    setFlag(N, 0);
    return 0;
}

// Using HL as an absolute address, increment the data that HL points to.
// Flag:
// -Z: Set if the result is 0
// -N: Reset to 0
// -H: Set if bit 4 after incrementing the data
uint8_t SM83::inc_abs_hl() {
    // Need to load the data that HL is pointing to.
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch the data
    uint8_t data = fetch();
    // Used to check half carry
    uint8_t h_check = (data & 0xf) + (1 & 0xf);
    data++;     // Increment the data
    // Check Z flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    write(addr_abs, data);
    return 0;
}

// Increment the B register.
// Flags:
//  -Z: Set this flag to 1 if result is 0
//  -N: Reset this flag to 0
//  -H: Set this flag to 1 if bit 4 is set after the increment

uint8_t SM83::inc_b() {
    uint8_t h_check= ((b_reg & 0xf) + (1 & 0xf));
    b_reg++;
    if(b_reg == 0)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check if bit 4 in b_reg is set due to our addition
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);      // If so, set the half-carry flag
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the BC register pair. If C overflows, increment the B register.
uint8_t SM83::inc_bc() {
    c_reg++;
    // Check to see if C wrapped back around to 0x00
    if (c_reg == 0x00)
        b_reg++;        // If so, increase B
    return 0;
}

// Increment the C register.
// Flags:
//  -Z: Set this flag to 1 if result is 0
//  -N: Reset this flag to 0
//  -H: Set this flag to 1 if bit 4 is set after the increment
uint8_t SM83::inc_c() {
    // Used for checking half carry flag
    uint8_t h_check = ((c_reg & 0xf) + (1 &0xf));
    c_reg++;
    // Check zero
    if(c_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the D register.
// Flags:
//  -Z: Set this flag to 1 if result is 0
//  -N: Reset this flag to 0
//  -H: Set this flag to 1 if bit 4 is set after the increment
uint8_t SM83::inc_d() {
    // Used to check half carry flag
    uint8_t h_check = ((d_reg & 0xf) + (1 &0xf));
    d_reg++;
    // Check zero
    if(d_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset the sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the DE register pair. If E overflows increment the D register.
uint8_t SM83::inc_de() {
    e_reg++;
    // Check to see if E register wrapped back around to 0x00
    if(e_reg == 0x00)
        d_reg++;
    return 0;
}

// Increment the E register.
// Flags:
//  -Z: Set if result is 0
//  -N: Reset this flag to 0
//  -H: Set if bit 4 is set after increment
uint8_t SM83::inc_e() {

    // Used to check for half carry flag
    uint8_t h_check = ((e_reg & 0xf) + (1 & 0xf));
    e_reg++;
    // Check for zero flag
    if(e_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check for half carry
    if((h_check &0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the H register.
// Flags:
//  -Z: Set if result is 0
//  -N: Reset this flag to 0
//  -H: Set if bit 4 is set after increment
uint8_t SM83::inc_h() {
    // Used to check for half carry flag
    uint8_t h_check = ((h_reg & 0xf) + (1 & 0xf));
    h_reg++;
    // Check for zero flag
    if(h_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check for half carry
    if((h_check &0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the HL register pair. If L overflows increment the H register.
uint8_t SM83::inc_hl() {
    l_reg++;
    if(l_reg == 0x00)
        h_reg++;

    return 0;
}

// Increment the L register.
// Flags:
//  -Z: Set if result is 0
//  -N: Reset this flag to 0
//  -H: Set if bit 4 is set after increment
uint8_t SM83::inc_l() {
    // Used to check for half carry flag
    uint8_t h_check = ((l_reg & 0xf) + (1 & 0xf));
    l_reg++;
    // Check for zero flag
    if(l_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Check for half carry
    if((h_check &0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Reset sign flag
    setFlag(N, 0);
    return 0;
}

// Increment the SP.
uint8_t SM83::inc_sp() {
    // Since the SP is an uint16_t, there is nothing really to do here.
    sp++;
    return 0;
}

// Load the program counter with the address stored in HL.
uint8_t SM83::jp_abs_hl() {
    // Load the address
    addr_abs = (h_reg << 8) | l_reg;
    // Load the address into the PC
    pc = addr_abs;
    return 0;
}

// Jump to the absolute 16-bit address.
uint8_t SM83::jp_a16() {
    // Load the address from PC
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    // Update the PC
    pc = (highByte << 8) | lowByte;
    return 0;
}

// Jump to the absolute 16-bit address if the carry flag is set.
uint8_t SM83::jp_c_a16() {
    // Load the address from the PC
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    // Check if carry flag is not enabled
    if(!getFlag(C))
        return 0;
    // Update the PC to point to the new address
    pc = (highByte << 8) | lowByte;
    return 4;
}

// Jump to the absolute 16-bit address if the carry flag is not set.
uint8_t SM83::jp_nc_a16() {
    // Load the address from the PC
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    // Check if carry flag is enabled
    if(getFlag(C))
        return 0;
    // Update the PC to point to the new address
    pc = (highByte << 8) | lowByte;
    return 4;
}

// Jump to the absolute 16-bit address if the zero flag is not set.
uint8_t SM83::jp_nz_a16() {
    // Load the address from the PC
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    // Check if zero flag is enabled
    if(getFlag(Z))
        return 0;
    // Update the PC to point to the new address
    pc = (highByte << 8) | lowByte;
    return 4;
}

// Jump to the absolute 16-bit address if the zero flag is set.
uint8_t SM83::jp_z_a16() {
    // Load the address from the PC
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    // Check if zero flag is not enabled
    if(!getFlag(Z))
        return 0;
    // Update the PC to point to the new address
    pc = (highByte << 8) | lowByte;
    return 4;
}

// Jump to an address -128 - +127 memory addresses relative to the current position
// of the PC.
// Note: If you are trying to jump to a memory address relative to the address of
// the JR opcode, then you will need to subtract 2 from the offset.
// This is because JR is a 2 byte opcode, and we increment the PC after we read
// in the offset.
uint8_t SM83::jr_r8() {
    // Use a signed 8-bit int
    int8_t offset = read(pc++);
    pc += offset;
    return 0;
}

// Jump to an address -128 - +127 memory address relative to the current position
// of the PC only if the carry flag is set. Add 4 clock cycles if the condition is
// met.
uint8_t SM83::jr_c_r8() {
    // Use a signed 8-bit int
    int8_t offset = read(pc++);
    if(!getFlag(C))
        return 0;
    pc += offset;
    return 4;
}

// Jump to an address -128 - +127 memory address relative to the current position
// of the PC only if the carry flag is not set. Add 4 clock cycles if the condition is
// met.
uint8_t SM83::jr_nc_r8() {
    // Use a signed 8-bit int
    int8_t offset = read(pc++);
    if(getFlag(C))
        return 0;
    pc += offset;
    return 4;
}

// Jump to an address -128 - +127 memory address relative to the current position
// of the PC only if the zero flag is not set. Add 4 clock cycles if the condition is
// met.
uint8_t SM83::jr_nz_r8() {
    // Used a signed 8-bit int
    int8_t offset = read(pc++);
    // Check if zero flag is set
    if(getFlag(Z))
        return 0;
    pc += offset;
    return 4;
}

// Jump to an address -128 - +127 memory address relative to the current position
// of the PC only if the zero flag is set. Add 4 clock cycles if the condition is
// met.
uint8_t SM83::jr_z_r8() {
    int8_t offset = read(pc++);
    // Check if the Z flag is not set
    if(!getFlag(Z))
        return 0;
    pc +=offset;
    return 4;
}

// Load the A register with itself.
uint8_t SM83::ld_a_a() {
    a_reg = a_reg;
    return 0;
}

// Load the A register using from the data stored at the immediate absolute address.
uint8_t SM83::ld_a_abs_a16() {
    // Need to load the address
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store in A
    a_reg = fetch();
    return 0;
}

// Load the A register with the data stored at the absolute address stored in HL.
uint8_t SM83::ld_a_abs_hl() {
    // Need to read in the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store the data into A
    a_reg = fetch();
    return 0;
}

// Load the A register with the contents of the B register.
uint8_t SM83::ld_a_b() {
    a_reg = b_reg;
    return 0;
}

// Load the A register with the contents of the C register.
uint8_t SM83::ld_a_c() {
    a_reg = c_reg;
    return 0;
}

// Load the A register with the contents of the D register.
uint8_t SM83::ld_a_d() {
    a_reg = d_reg;
    return 0;
}

// Load A with the immediate 8-bit data.
uint8_t SM83::ld_a_d8() {
    a_reg = read(pc++);
    return 0;
}

// Load the A register with the contents of the E register.
uint8_t SM83::ld_a_e() {
    a_reg = e_reg;
    return 0;
}

// Load the A register with the contents of the H register.
uint8_t SM83::ld_a_h() {
    a_reg = h_reg;
    return 0;
}

// Load the A register with the contents of the L register.
uint8_t SM83::ld_a_l() {
    a_reg = l_reg;
    return 0;
}

// Load the 8-bit data value from the absolute address in the register pair BC into register A.
uint8_t SM83::ld_a_abs_bc() {
    uint16_t lowByte = c_reg;
    uint16_t highByte = b_reg;
    // Load the absolute address into addr_abs
    addr_abs = (highByte << 8) | lowByte;
    // Read in the 8-bit data value at the address in addr_abs

    a_reg = fetch();
    return 0;
}
// Load the 8-bit data value from the absolute address in the register pair DE into register A.
uint8_t SM83::ld_a_abs_de() {
    uint16_t lowByte = e_reg;
    uint16_t highByte = d_reg;
    // Load the absolute address into addr_abs
    addr_abs = (highByte << 8) | lowByte;
    a_reg = fetch();
    return 0;
}

// Load the 8-bit data value from the absolute address in the register pair HL into register A
// then, decrement the HL register pair.
uint8_t SM83::ld_a_abs_hld() {
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    // Load the absolute address into addr_abs
    addr_abs = (highByte << 8) | lowByte;
    a_reg = fetch();
    dec_hl();
    return 0;
}

// Load the 8-bit data value from the absolute address in the register pair HL into register A
// then, increment the HL register pair.
uint8_t SM83::ld_a_abs_hli() {
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    // Load the absolute address into addr_abs
    addr_abs = (highByte << 8) | lowByte;
    a_reg = fetch();
    inc_hl();
    return 0;
}

// Load the A register into the 16-bit absolute address.
uint8_t SM83::ld_abs_a16_a() {
    // Load the address
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    addr_abs = (highByte << 8) | lowByte;
    // Write the data
    write(addr_abs, a_reg);
    return 0;
}

// Load the 16-bit value in the SP into the 16-bit little endian absolute address.
uint8_t SM83::ld_abs_a16_sp() {
    // Load the absolute address into addr_abs
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    addr_abs = (highByte << 8) | lowByte;

    // Write the SP to the absolute address in little-endian
    write(addr_abs++, sp);
    write(addr_abs, (sp >> 8));
    return 0;
}

// Using BC as an absolute address, load value in A into that address.
uint8_t SM83::ld_abs_bc_a() {
    uint16_t lowByte = c_reg;
    uint16_t highByte = b_reg;
    addr_abs = (highByte << 8) | lowByte;
    write(addr_abs, a_reg);
    return 0;
}

//  Using DE as an absolute address, load value in A into that address.
uint8_t SM83::ld_abs_de_a() {
    uint16_t lowByte = e_reg;
    uint16_t  highByte = d_reg;
    addr_abs = (highByte << 8) | lowByte;
    write(addr_abs, a_reg);
    return 0;
}

// Load the data stored at the absolute address in HL with the contents of the A register.
uint8_t SM83::ld_abs_hl_a() {
    // Need to load the address in HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Write the data in B into the address stored in HL
    write(addr_abs, a_reg);
    return 0;
    return 0;
}

// Load the data stored at the absolute address in HL with the contents of the B register.
uint8_t SM83::ld_abs_hl_b() {
    // Need to load the address in HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Write the data in B into the address stored in HL
    write(addr_abs, b_reg);
    return 0;
}

// Load the data stored at the absolute address in HL with the contents of the C register.
uint8_t SM83::ld_abs_hl_c() {
    // Need to load the address in HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Write the data in C into the address stored in HL
    write(addr_abs, c_reg);

    return 0;
}


// Load the data stored at the absolute address in HL with the contents of the D register.
uint8_t SM83::ld_abs_hl_d() {
    // Need to load the address in HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Write the data in C into the address stored in HL
    write(addr_abs, d_reg);

    return 0;
}

// Using HL as an absolute address, load the immediate 8-bit data into that address.
uint8_t SM83::ld_abs_hl_d8() {
    // Need to load the address in HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = read(pc++);
    write(addr_abs, data);
    return 0;
}

// Load the data stored at the absolute address in HL with the contents of the E register.
uint8_t SM83::ld_abs_hl_e() {
    // Need to load the address in HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Write the data in C into the address stored in HL
    write(addr_abs, e_reg);

    return 0;
}

// Load the data stored at the absolute address in HL with the contents of the H register.
uint8_t SM83::ld_abs_hl_h() {
    // Need to load the address in HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Write the data in C into the address stored in HL
    write(addr_abs, h_reg);

    return 0;
}

// Load the data stored at the absolute address in HL with the contents of the L register.
uint8_t SM83::ld_abs_hl_l() {
    // Need to load the address in HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Write the data in C into the address stored in HL
    write(addr_abs, l_reg);

    return 0;
}

// Using HL as an absolute address, load the value in A into that address, then increment the HL register pair.
uint8_t SM83::ld_abs_hli_a() {
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    write(addr_abs, a_reg);
    inc_hl();
    return 0;
}

// Using HL as an absolute address, load the value in A into that address, then decrement the HL register pair.
uint8_t SM83::ld_abs_hld_a() {
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    write(addr_abs, a_reg);
    dec_hl();
    return 0;
}

// Load the B register with the contents of the A register.
uint8_t SM83::ld_b_a() {
    b_reg = a_reg;
    return 0;
}

// Load the B register with the data located at the absolute address stored in HL.
uint8_t SM83::ld_b_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Save the fetched data into B
    b_reg = fetch();
    return 0;
}

// Load the B register with itself.
// Not sure why this opcode exists
uint8_t SM83::ld_b_b() {
    b_reg = b_reg;
    return 0;
}

// Load the B register with the contents of the C register.
uint8_t SM83::ld_b_c() {
    b_reg = c_reg;
    return 0;
}

// Load the B register with the contents of the D register.
uint8_t SM83::ld_b_d() {
    b_reg = d_reg;
    return 0;
}

// Load the B register with an immediate 8-bit data value.
uint8_t SM83::ld_b_d8() {
    b_reg = read(pc++);
    return 0;
}

// Load the B register with the contents of the E register.
uint8_t SM83::ld_b_e() {
    b_reg = e_reg;
    return 0;
}

// Load the B register with the contents of the H register.
uint8_t SM83::ld_b_h() {
    b_reg = h_reg;
    return 0;
}

// Load the B register with the contents of the L register.
uint8_t SM83::ld_b_l() {
    b_reg = l_reg;
    return 0;
}

// Load register pair BC with an immediate little-endian 16-bit data value.
uint8_t SM83::ld_bc_d16() {
    c_reg = read(pc++);
    b_reg = read(pc++);
    // No additional clocks, return 0
    return 0;
}

// Load the C register with the contents of the A register.
uint8_t SM83::ld_c_a() {
    c_reg = a_reg;
    return 0;
}

// Load the C register with the data stored at the absolute address in HL.
uint8_t SM83::ld_c_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store the data in C
    c_reg = fetch();
    return 0;
}

// Load the C register with the contents of the B register.
uint8_t SM83::ld_c_b() {
    c_reg = b_reg;
    return 0;
}

// Load the contents of the C register with itself.
uint8_t SM83::ld_c_c() {
    c_reg = c_reg;
    return 0;
}

// Load the C register with the contents of the D register.
uint8_t SM83::ld_c_d() {
    c_reg = d_reg;
    return 0;
}

// Load the C register with the immediate 8-bit data value.
uint8_t SM83::ld_c_d8() {
    c_reg = read(pc++);
    return 0;
}

// Load the C register with the contents of the E register.
uint8_t SM83::ld_c_e() {
    c_reg = e_reg;
    return 0;
}

// Load the C register with the contents of the H register.
uint8_t SM83::ld_c_h() {
    c_reg = h_reg;
    return 0;
}


// Load the C register with the contents of the L register.
uint8_t SM83::ld_c_l() {
    c_reg = l_reg;
    return 0;
}

// Load the D register with the contents of the A register.
uint8_t SM83::ld_d_a() {
    d_reg = a_reg;
    return 0;
}

// Load the D register with the data located at the absolute address in HL.
uint8_t SM83::ld_d_abs_hl() {
    // Get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store the data into D
    d_reg = fetch();
    return 0;
}

// Load the D register with the contents of the B register.
uint8_t SM83::ld_d_b() {
    d_reg = b_reg;
    return 0;
}


// Load the D register with the contents of the C register.
uint8_t SM83::ld_d_c() {
    d_reg = c_reg;
    return 0;
}

// Load the D register with the contents of itself.
uint8_t SM83::ld_d_d() {
    d_reg = d_reg;
    return 0;
}

// Load the D register with the contents of the E register.
uint8_t SM83::ld_d_e() {
    d_reg = e_reg;
    return 0;
}

// Load the D register with the contents of the H register.
uint8_t SM83::ld_d_h() {
    d_reg = h_reg;
    return 0;
}

// Load the D register with the contents of the L register.
uint8_t SM83::ld_d_l() {
    d_reg = l_reg;
    return 0;
}

// Load the D register with the immediate 8-bit data value.
uint8_t SM83::ld_d_d8() {
    d_reg = read(pc++);
    return 0;
}

// Load the DE register pair with the immediate 16-bit data value.
uint8_t SM83::ld_de_d16() {
    e_reg = read(pc++);
    d_reg = read(pc++);
    return 0;
}

// Load the E register with the contents of the B register.
uint8_t SM83::ld_e_b() {
    e_reg = b_reg;
    return 0;
}

// Load the E register with the contents of the C register.
uint8_t SM83::ld_e_c() {
    e_reg = c_reg;
    return 0;
}

// Load the E register with the contents of the D register.
uint8_t SM83::ld_e_d() {
    e_reg = d_reg;
    return 0;
}

// Load E register with the immediate 8-bit data value.
uint8_t SM83::ld_e_d8() {
    e_reg = read(pc++);
    return 0;
}

// Load E register with itself.
uint8_t SM83::ld_e_e() {
    e_reg = e_reg;
    return 0;
}

// Load the E register with the contents of the H register.
uint8_t SM83::ld_e_h(){
    e_reg = h_reg;
    return 0;
}

// Load the E register with the contents of the L register.
uint8_t SM83::ld_e_l() {
    e_reg = l_reg;
    return 0;
}

// Load the E register with the contents of the A register.
uint8_t SM83::ld_e_a() {
    e_reg = a_reg;
    return 0;
}

// Load the E register with the data located at the absolute address in HL.
uint8_t SM83::ld_e_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store data in E
    e_reg = fetch();
    return 0;
}

// Load the H register with the contents of the A register.
uint8_t SM83::ld_h_a() {
    h_reg = a_reg;
    return 0;
}

// Load the H register from the absolute address stored in HL.
uint8_t SM83::ld_h_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8 ) | lowByte;
    // Fetch and store data in H
    h_reg = fetch();
    return 0;
}

// Load the H register with the contents of the B register.
uint8_t SM83::ld_h_b() {
    h_reg = b_reg;
    return 0;
}

// Load the H with the contents of the C register.
uint8_t SM83::ld_h_c() {
    h_reg = c_reg;
    return 0;
}

// Load the H register with the contents of the D register.
uint8_t SM83::ld_h_d() {
    h_reg = d_reg;
    return 0;
}

// Load H register with the immediate 8-bit data value.
uint8_t SM83::ld_h_d8() {
    h_reg = read(pc++);
    return 0;
}

// Load the H register with the contents of the E register.
uint8_t SM83::ld_h_e() {
    h_reg = e_reg;
    return 0;
}

// Load the H register with itself.
uint8_t SM83::ld_h_h() {
    h_reg = h_reg;
    return 0;
}

// Load the H register with the contents of the L register.
uint8_t SM83::ld_h_l() {
    h_reg = l_reg;
    return 0;
}

// Load the HL register pair with the immediate 16-bit value.
uint8_t SM83::ld_hl_d16() {
    l_reg = read(pc++);
    h_reg = read(pc++);
    return 0;
}
// Add immediate signed 8-bit data to SP and load into HL. Do not update SP.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Set if bit 3 overflows
//  -C: Set if bit 7 overflows
uint8_t SM83::ld_hl_sp_r8() {
    // SP is an uint16_t
    // Make a copy of SP
    uint16_t sp_cp = sp;
    // Need to read in the value as a signed int
    uint8_t data = read(pc++);
    // Need to have a signed version of the data
    int8_t sData = data;
    uint8_t h_check = (sp_cp & 0x000f) + (data & 0xf);
    uint16_t overflow = (sp_cp & 0x00ff) + data;
    // Add the signed data
    sp_cp += sData;
    // Check for half carry
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Check for carry flag
    if(overflow > 0xff)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset the zero and sign flags
    setFlag(Z, 0);
    setFlag(N, 0);
    // Load the SP into HL
    l_reg = (0x00ff & sp_cp);
    // Shift the high byte of sp_cp into the low byte
    h_reg = (sp_cp >> 8);
    return 0;
}

// Load the L register with the contents of the A register.
uint8_t SM83::ld_l_a() {
    l_reg = a_reg;
    return 0;
}

// Load the L register with the data stored in HL.
uint8_t SM83::ld_l_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store the data into L
    l_reg = fetch();
    return 0;
}

// Load the L register with the contents of the B register.
uint8_t SM83::ld_l_b() {
    l_reg = b_reg;
    return 0;
}

// Load the L register with the contents of the C register.
uint8_t SM83::ld_l_c() {
    l_reg = c_reg;
    return 0;
}

// Load the L register with the contents of the D register.
uint8_t SM83::ld_l_d() {
    l_reg = d_reg;
    return 0;
}


// Load L register with the immediate 8-bit data value.
uint8_t SM83::ld_l_d8() {
    l_reg = read(pc++);
    return 0;
}

// Load the L register with the contents of the E register.
uint8_t SM83::ld_l_e() {
    l_reg = e_reg;
    return 0;
}

// Load the L register with the contents of the H register.
uint8_t SM83::ld_l_h() {
    l_reg = h_reg;
    return 0;
}

// Load the L register with itself.
uint8_t SM83::ld_l_l() {
    l_reg = l_reg;
    return 0;
}

// Load the SP with the immediate 16-bit value.
uint8_t SM83::ld_sp_d16() {
    uint16_t lowByte = read(pc++);
    uint16_t highByte = read(pc++);
    // Create 16-bit data from low and high bytes read in from PC
    uint16_t data = (highByte << 8) | lowByte;
    sp = data;
    return 0;
}

// Load the SP with the HL register pair.
uint8_t SM83::ld_sp_hl() {
    // Create a 16 bit version of HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    // Store in SP
    sp = (highByte << 8) | lowByte;
    return 0;
}

// Using a8 as an offset from address 0xff00, load the contents at the absolute address into the A register.
uint8_t SM83::ldh_a_abs_a8() {
    // Load the address
    uint16_t lowByte = read(pc++);
    addr_abs = (0xff00 | lowByte);
    // Fetch and store in A
    a_reg = fetch();
    return 0;
}

// Using the C register as an offset from 0xff00, load the A register with the data stored at that address.
uint8_t SM83::ldh_a_abs_c() {
    // Load the data
    addr_abs = (0xff00 | c_reg);
    // Fetch and store the data in A
    a_reg = fetch();
    return 0;
}

// Using a8 as an offset from address 0xFF00, load the A register into that address.
uint8_t SM83::ldh_abs_a8_a() {
    // Load the address
    uint16_t lowByte = read(pc++);
    addr_abs = (0xff00 | lowByte);
    // Write the data to the address
    write(addr_abs, a_reg);
    return 0;
}

// Using the C register as an offset from 0xff00, load the A register into that address.
uint8_t SM83::ldh_abs_c_a() {
    // Load the address
    addr_abs = (0xff00 | c_reg);
    // Write the data to the address
    write(addr_abs, a_reg);
    return 0;
}

// Or the A register with itself. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::or_a() {
    a_reg |= a_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Or the A register with the data stored at the absolute address in HL. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::or_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store
    uint8_t data = fetch();
    a_reg |= data;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Or the A and B registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::or_b() {
    a_reg |= b_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Or the A and C registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::or_c() {
    a_reg |= c_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Or the A and D registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::or_d() {
    a_reg |= d_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Or the A register with the immediate 8-bit data. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::or_d8() {
    // Need to get the data
    uint8_t data = read(pc++);
    a_reg |= data;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Or the A and E registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::or_e() {
    a_reg |= e_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Or the A and H registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::or_h() {
    a_reg |= h_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Or the A and L registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::or_l() {
    a_reg |= l_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Pop 2 bytes off of the SP and load them into AF.
// Flags:
//  - Z: Set according to the first byte popped off of the stack
//  - N: Set according to the first byte popped off of the stack
//  - H: Set according to the first byte popped off of the stack
//  - C: Set according to the first byte popped off of the stack
uint8_t SM83::pop_af() {
    // Load the address that SP points to into addr_abs
    addr_abs = sp;
    // Fetch the data stored at where the SP is pointing at and at (SP + 1)
    f_reg = (0xf0 & fetch());   // Need to reset the lower nibble
    addr_abs++;
    a_reg = fetch();
    // Point the SP to the correct byte
    sp++;
    sp++;
    return 0;
}

// Pop 2 bytes off of the SP and load them into BC.
uint8_t SM83::pop_bc() {
    // Load the address that SP points to into addr_abs
    addr_abs = sp;
    // Fetch the data stored at where the SP is pointing at and at (SP + 1)
    c_reg = fetch();
    addr_abs++;
    b_reg = fetch();
    // Point the SP to the correct byte
    sp++;
    sp++;
    return 0;
}

// Pop 2 bytes off of the SP and load them into DE.
uint8_t SM83::pop_de() {
    // Load the address that SP points to into addr_abs
    addr_abs = sp;
    // Fetch the data stored at where the SP is pointing at and at (SP + 1)
    e_reg = fetch();
    addr_abs++;
    d_reg = fetch();
    // Point the SP to the correct byte
    sp++;
    sp++;
    return 0;
}

// Pop 2 bytes off of the SP and load them into HL.
uint8_t SM83::pop_hl() {
    // Load the address that SP points to into addr_abs
    addr_abs = sp;
    // Fetch the data stored at where the SP is pointing at and at (SP + 1)
    l_reg = fetch();
    addr_abs++;
    h_reg = fetch();
    // Point the SP to the correct byte
    sp++;
    sp++;
    return 0;
}

// This function is used to access the prefix opcode table.
// Note: None of the prefixed opcodes are more than 2 bytes (1 after we remove the prefix)
uint8_t SM83::prefix() {
    // Read the opcode for the prefix function
    uint8_t prefix_op = read(pc++);
    // Add the cycles to the current cycle count
    cycles += prefix_lookup[prefix_op].cycles;
    // Call the function pointer - will return additional clock cycles
    cycles += (this->*prefix_lookup[prefix_op].operate)();
    return 0;
}

// Push the AF register pair onto the stack.
uint8_t SM83::push_af() {
    sp--;
    write(sp, a_reg);
    sp--;
    write(sp, f_reg);
    return 0;
}

// Push the BC register pair onto the stack.
uint8_t SM83::push_bc() {
    sp--;
    write(sp, b_reg);
    sp--;
    write(sp, c_reg);
    return 0;
}

// Push the DE register pair onto the stack.
uint8_t SM83::push_de() {
    sp--;
    write(sp, d_reg);
    sp--;
    write(sp, e_reg);
    return 0;
}

// Push the HL register pair onto the stack.
uint8_t SM83::push_hl() {
    sp--;
    write(sp, h_reg);
    sp--;
    write(sp, l_reg);
    return 0;
}

// Generic function to reset bit n in register r8.
uint8_t SM83::res_n_r8() {
    // Get the opcode of the instruction that is to be executed
    uint8_t prev_opcode = read(pc - 1);
    // Access the prefix opcode table as this function is for a prefixed opcode
    uint8_t n = prefix_lookup[prev_opcode].n;
    // Need to check the size of the operand vector
    // The only way the size of the vector will be 2 is if we are working with
    // the HL register pair, and need to get the absolute address stored in it.
    if(prefix_lookup[prev_opcode].operands.size() == 2) {
        // Grab the data from the H & L pointers from the operands vector
        // Operands are little endian
        uint8_t lowByte = *prefix_lookup[prev_opcode].operands[0];
        uint8_t highByte = *prefix_lookup[prev_opcode].operands[1];
        addr_abs = (highByte << 8) | lowByte;
        uint8_t data = fetch();
        data &= ~(1 << n);
        // Write the data
        write(addr_abs, data);
        return 0;
    }
    // If we are here, that means only 1 operand
    uint8_t *r8 = prefix_lookup[prev_opcode].operands[0];
    // Reset bit n in register r8
    *r8 &= ~(1 << n);
    return 0;
}

// Pop 2 bytes off of the stack and load them into the PC.
uint8_t SM83::ret() {
    // Load the address that the SP points to into addr_abs
    addr_abs = sp;
    uint16_t pc_l = fetch();
    addr_abs++;
    uint16_t pc_h = fetch();
    // Load the address stored in the SP into PC
    pc = (pc_h << 8) | pc_l;
    // Point the SP to the correct byte
    sp++;
    sp++;
    return 0;
}

// Pop 2 bytes off of the stack and load them into the PC, only when the carry flag is set.
uint8_t SM83::ret_c() {
    // Check to see if the carry flag is not set
    if(!getFlag(C))
        return 0;
    // Load the address that the SP points to into addr_abs
    addr_abs = sp;
    uint16_t pc_l = fetch();
    addr_abs++;
    uint16_t pc_h = fetch();
    // Load the address stored in the SP into PC
    pc = (pc_h << 8) | pc_l;
    // Point the SP to the correct byte
    sp++;
    sp++;
    return 12;
}

// Pop 2 bytes off of the stack and load them into the PC, only when the carry flag is not set.
uint8_t SM83::ret_nc() {
    // Check to see if the carry flag is set
    if(getFlag(C))
        return 0;
    // Load the address that the SP points to into addr_abs
    addr_abs = sp;
    uint16_t pc_l = fetch();
    addr_abs++;
    uint16_t pc_h = fetch();
    // Load the address stored in the SP into PC
    pc = (pc_h << 8) | pc_l;
    // Point the SP to the correct byte
    sp++;
    sp++;
    return 12;
}

// Pop 2 bytes off of the stack and load them into the PC, only when the zero flag is not set.
uint8_t SM83::ret_nz() {
    // Check to see if the zero flag is set
    if(getFlag(Z))
        return 0;
    // Load the address that the SP points to into addr_abs
    addr_abs = sp;
    uint16_t pc_l = fetch();
    addr_abs++;
    uint16_t pc_h = fetch();
    // Load the address stored in the SP into PC
    pc = (pc_h << 8) | pc_l;
    // Point the SP to the correct byte
    sp++;
    sp++;
    return 12;
}

// Pop 2 bytes off of the stack and load them into the PC, only when the zero flag is set.
uint8_t SM83::ret_z() {
    // Check to see if the zero flag is not set
    if(!getFlag(Z))
        return 0;
    // Load the address that the SP points to into addr_abs
    addr_abs = sp;
    uint16_t pc_l = fetch();
    addr_abs++;
    uint16_t pc_h = fetch();
    // Load the address stored in the SP into PC
    pc = (pc_h << 8) | pc_l;
    // Point the SP to the correct byte
    sp++;
    sp++;
    return 12;
}

// TODO: Enable interrupts after return. Still need to implement interrupts
uint8_t SM83::reti() {
    // Load the address that the SP points to into addr_abs
    addr_abs = sp;
    uint16_t pc_l = fetch();
    addr_abs++;
    uint16_t pc_h = fetch();
    // Load the address stored in the SP into PC
    pc = (pc_h << 8) | pc_l;
    // Point the SP to the correct byte
    sp++;
    sp++;

    // Need to enable interrupts after return

    return 0;
}

// Rotates the bits in A register left. If the carry is enabled, that bit is fed into
// bit 0.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 7 is enabled before rotating
uint8_t SM83::rla() {
    // Check if bit 7 is enabled before rotating
    uint8_t c_check = a_reg & (1 << 7);
    // Check to see if the carry bit is set
    if(getFlag(C) == 1) {
        a_reg = (a_reg << 1);           // Shift left 1
        a_reg |= 0x01;                  // Set bit 0
    }
    else
        a_reg = (a_reg << 1);
    // Need to check for carry
    // Check to see if bit 7 was enabled before rotate
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset rest of flags
    setFlag(Z, 0);
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotates the bits in A register left. If the carry is enabled, that bit is fed into
// bit 0.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 7 is enabled before rotating
uint8_t SM83::rl_a() {
    // Check if bit 7 is enabled before rotating
    uint8_t c_check = a_reg & (1 << 7);
    // Check to see if the carry bit is set
    if(getFlag(C) == 1) {
        a_reg = (a_reg << 1);           // Shift left 1
        a_reg |= 0x01;                  // Set bit 0
    }
    else
        a_reg = (a_reg << 1);
    // Need to check for carry
    // Check to see if bit 7 was enabled before rotate
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(a_reg == 0x00)
        setFlag(Z , 1);
    else
        setFlag(Z, 0);
    // Reset rest of flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotates the bits stored at the absolute address in HL. If the carry is enabled, that bit is fed into
// bit 0.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 7 is enabled before rotating
uint8_t SM83::rl_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store
    uint8_t data = fetch();
    // Check if bit 7 is enabled before rotating
    uint8_t c_check = data & (1 << 7);
    // Check to see if the carry bit is set
    if(getFlag(C) == 1) {
        data = (data  << 1);           // Shift left 1
        data |= 0x01;                  // Set bit 0
    }
    else
        data = (data << 1);
    // Write the data back into the address
    write(addr_abs, data);
    // Need to check for carry
    // Check to see if bit 7 was enabled before rotate
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for zero flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotates the bits in B register left. If the carry is enabled, that bit is fed into
// bit 0.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 7 is enabled before rotating
uint8_t SM83::rl_b() {
    // Check if bit 7 is enabled before rotating
    uint8_t c_check = b_reg & (1 << 7);
    // Check to see if the carry bit is set
    if(getFlag(C) == 1) {
        b_reg = (b_reg << 1);           // Shift left 1
        b_reg |= 0x01;                  // Set bit 0
    }
    else
        b_reg = (b_reg << 1);
    // Need to check for carry
    // Check to see if bit 7 was enabled before rotate
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(b_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotates the bits in C register left. If the carry is enabled, that bit is fed into
// bit 0.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 7 is enabled before rotating
uint8_t SM83::rl_c() {
    // Check if bit 7 is enabled before rotating
    uint8_t c_check = c_reg & (1 << 7);
    // Check to see if the carry bit is set
    if(getFlag(C) == 1) {
        c_reg = (c_reg << 1);           // Shift left 1
        c_reg |= 0x01;                  // Set bit 0
    }
    else
        c_reg = (c_reg << 1);
    // Need to check for carry
    // Check to see if bit 7 was enabled before rotate
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check zero flag
    if(c_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotates the bits in D register left. If the carry is enabled, that bit is fed into
// bit 0.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 7 is enabled before rotating
uint8_t SM83::rl_d() {
    // Check if bit 7 is enabled before rotating
    uint8_t c_check = d_reg & (1 << 7);
    // Check to see if the carry bit is set
    if(getFlag(C) == 1) {
        d_reg = (d_reg << 1);           // Shift left 1
        d_reg |= 0x01;                  // Set bit 0
    }
    else
        d_reg = (d_reg << 1);
    // Need to check for carry
    // Check to see if bit 7 was enabled before rotate
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(d_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotates the bits in E register left. If the carry is enabled, that bit is fed into
// bit 0.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 7 is enabled before rotating
uint8_t SM83::rl_e() {
    // Check if bit 7 is enabled before rotating
    uint8_t c_check = e_reg & (1 << 7);
    // Check to see if the carry bit is set
    if(getFlag(C) == 1) {
        e_reg = (e_reg << 1);           // Shift left 1
        e_reg |= 0x01;                  // Set bit 0
    }
    else
        e_reg = (e_reg << 1);
    // Need to check for carry
    // Check to see if bit 7 was enabled before rotate
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(e_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotates the bits in H register left. If the carry is enabled, that bit is fed into
// bit 0.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 7 is enabled before rotating
uint8_t SM83::rl_h() {
    // Check if bit 7 is enabled before rotating
    uint8_t c_check = h_reg & (1 << 7);
    // Check to see if the carry bit is set
    if(getFlag(C) == 1) {
        h_reg = (h_reg << 1);           // Shift left 1
        h_reg |= 0x01;                  // Set bit 0
    }
    else
        h_reg = (h_reg << 1);
    // Need to check for carry
    // Check to see if bit 7 was enabled before rotate
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(h_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotates the bits in L register left. If the carry is enabled, that bit is fed into
// bit 0.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 7 is enabled before rotating
uint8_t SM83::rl_l() {
    // Check if bit 7 is enabled before rotating
    uint8_t c_check = l_reg & (1 << 7);
    // Check to see if the carry bit is set
    if(getFlag(C) == 1) {
        l_reg = (l_reg << 1);           // Shift left 1
        l_reg |= 0x01;                  // Set bit 0
    }
    else
        l_reg = (l_reg << 1);
    // Need to check for carry
    // Check to see if bit 7 was enabled before rotate
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(l_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotates the bits in the A register left.
// Flags:
//  - Z: Reset to 0
//  - N: Reset to 0
//  - H: Reset to 0
//  - C: When the last bit is enabled, enable the carry bit
uint8_t SM83::rlca() {
    // If bit 7 in B is set, set the carry bit
    if(a_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits left 1
    // First shift all bits left one, then or with all bits shifted right 7.
    a_reg = (a_reg << 1) | (a_reg >> 7);

    // Reset the rest of the flags
    setFlag(Z, 0);
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in the A register left.
// Flags:
//  - Z: Set if the result is 1
//  - N: Reset to 0
//  - H: Reset to 0
//  - C: When the last bit is enabled, enable the carry bit
uint8_t SM83::rlc_a() {
    // If bit 7 in B is set, set the carry bit
    if(a_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits left 1
    // First shift all bits left one, then or with all bits shifted right 7.
    a_reg = (a_reg << 1) | (a_reg >> 7);
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in the byte stored at the absolute address in HL, left.
// Flags:
//  - Z: Set to 1 if the result is 0
//  - N: Reset to 0
//  - H: Reset to 0
//  - C: When the last bit is enabled, enable the carry bit
uint8_t SM83::rlc_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // If bit 7 is set, set the carry bit
    if(data & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits left 1
    // First shift all bits left one, then or with all bits shifted right 7.
    data = (data << 1) | (data >> 7);
    // Write the data to the address
    write(addr_abs, data);
    // Check for zero flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in the B register left.
// Flags:
//  - Z: Reset to 0
//  - N: Reset to 0
//  - H: Reset to 0
//  - C: When the last bit is enabled, enable the carry bit
uint8_t SM83::rlc_b() {
    // If bit 7 is set, set the carry bit
    if(b_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits left 1
    // First shift all bits left one, then or with all bits shifted right 7.
    b_reg = (b_reg << 1) | (b_reg >> 7);
    // Check for zero flag
    if(b_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);

    return 0;
}
// Rotates the bits in the C register left.
// Flags:
//  - Z: Reset to 0
//  - N: Reset to 0
//  - H: Reset to 0
//  - C: When the last bit is enabled, enable the carry bit
uint8_t SM83::rlc_c() {
    // If bit 7 is set, set the carry bit
    if(c_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits left 1
    // First shift all bits left one, then or with all bits shifted right 7.
    c_reg = (c_reg << 1) | (c_reg >> 7);
    // Check the zero flag
    if(c_reg == 0x00)
        setFlag(Z ,1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}
// Rotates the bits in the D register left.
// Flags:
//  - Z: Reset to 0
//  - N: Reset to 0
//  - H: Reset to 0
//  - C: When the last bit is enabled, enable the carry bit
uint8_t SM83::rlc_d() {
    // If bit 7 is set, set the carry bit
    if(d_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits left 1
    // First shift all bits left one, then or with all bits shifted right 7.
    d_reg = (d_reg << 1) | (d_reg >> 7);
    // Check the zero flag
    if(d_reg == 0x00)
        setFlag(Z ,1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in the E register left.
// Flags:
//  - Z: Reset to 0
//  - N: Reset to 0
//  - H: Reset to 0
//  - C: When the last bit is enabled, enable the carry bit
uint8_t SM83::rlc_e() {
    // If bit 7 is set, set the carry bit
    if(e_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits left 1
    // First shift all bits left one, then or with all bits shifted right 7.
    e_reg = (e_reg << 1) | (e_reg >> 7);
    // Check the zero flag
    if(e_reg == 0x00)
        setFlag(Z ,1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in the H register left.
// Flags:
//  - Z: Reset to 0
//  - N: Reset to 0
//  - H: Reset to 0
//  - C: When the last bit is enabled, enable the carry bit
uint8_t SM83::rlc_h() {
    // If bit 7 is set, set the carry bit
    if(h_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits left 1
    // First shift all bits left one, then or with all bits shifted right 7.
    h_reg = (h_reg << 1) | (h_reg >> 7);
    // Check the zero flag
    if(h_reg == 0x00)
        setFlag(Z ,1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in the L register left.
// Flags:
//  - Z: Reset to 0
//  - N: Reset to 0
//  - H: Reset to 0
//  - C: When the last bit is enabled, enable the carry bit
uint8_t SM83::rlc_l() {
    // If bit 7 is set, set the carry bit
    if(l_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits left 1
    // First shift all bits left one, then or with all bits shifted right 7.
    l_reg = (l_reg << 1) | (l_reg >> 7);
    // Check for the zero flag
    if(l_reg == 0x00)
        setFlag(Z ,1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in A register right. If the carry is enabled, that bit is fed into
// bit 7.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is enabled before rotating
uint8_t SM83::rra() {
    // Check if bit 0 is enabled before rotating
    uint8_t c_check = a_reg & 0x01;
    // Check if the carry bit is enabled
    if(getFlag(C) == 1) {
        a_reg = (a_reg >> 1);       // Shift right one
        a_reg |= (1 << 7);              // Enable bit 7
    }
    else
        a_reg = (a_reg >> 1);
    // Check if the carry flag needs to be set
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Reset other flags
    setFlag(Z, 0);
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in A register right. If the carry is enabled, that bit is fed into
// bit 7.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is enabled before rotating
uint8_t SM83::rr_a() {
    // Check if bit 0 is enabled before rotating
    uint8_t c_check = a_reg & 0x01;
    // Check if the carry bit is enabled
    if(getFlag(C) == 1) {
        a_reg = (a_reg >> 1);       // Shift right one
        a_reg |= (1 << 7);              // Enable bit 7
    }
    else
        a_reg = (a_reg >> 1);
    // Check if the carry flag needs to be set
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits at the address stored in HL right. If the carry is enabled, that bit is fed into
// bit 7.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is enabled before rotating
uint8_t SM83::rr_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Check if bit 0 is enabled before rotating
    uint8_t c_check = data & 0x01;
    // Check if the carry bit is enabled
    if(getFlag(C) == 1) {
        data = (data >> 1);       // Shift right one
        data |= (1 << 7);              // Enable bit 7
    }
    else
        data = (data >> 1);
    // Write the data at the address
    write(addr_abs, data);
    // Check if the carry flag needs to be set
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in B register right. If the carry is enabled, that bit is fed into
// bit 7.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is enabled before rotating
uint8_t SM83::rr_b() {
    // Check if bit 0 is enabled before rotating
    uint8_t c_check = b_reg & 0x01;
    // Check if the carry bit is enabled
    if(getFlag(C) == 1) {
        b_reg = (b_reg >> 1);       // Shift right one
        b_reg |= (1 << 7);              // Enable bit 7
    }
    else
        b_reg = (b_reg >> 1);
    // Check if the carry flag needs to be set
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(b_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset other flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in C register right. If the carry is enabled, that bit is fed into
// bit 7.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is enabled before rotating
uint8_t SM83::rr_c() {
    // Check if bit 0 is enabled before rotating
    uint8_t c_check = c_reg & 0x01;
    // Check if the carry bit is enabled
    if(getFlag(C) == 1) {
        c_reg = (c_reg >> 1);       // Shift right one
        c_reg |= (1 << 7);              // Enable bit 7
    }
    else
        c_reg = (c_reg >> 1);
    // Check if the carry flag needs to be set
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(c_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset other flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in D register right. If the carry is enabled, that bit is fed into
// bit 7.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is enabled before rotating
uint8_t SM83::rr_d() {
    // Check if bit 0 is enabled before rotating
    uint8_t c_check = d_reg & 0x01;
    // Check if the carry bit is enabled
    if(getFlag(C) == 1) {
        d_reg = (d_reg >> 1);       // Shift right one
        d_reg |= (1 << 7);              // Enable bit 7
    }
    else
        d_reg = (d_reg >> 1);
    // Check if the carry flag needs to be set
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(d_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset other flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in E register right. If the carry is enabled, that bit is fed into
// bit 7.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is enabled before rotating
uint8_t SM83::rr_e() {
    // Check if bit 0 is enabled before rotating
    uint8_t c_check = e_reg & 0x01;
    // Check if the carry bit is enabled
    if(getFlag(C) == 1) {
        e_reg = (e_reg >> 1);       // Shift right one
        e_reg |= (1 << 7);              // Enable bit 7
    }
    else
        e_reg = (e_reg >> 1);
    // Check if the carry flag needs to be set
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(e_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset other flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in H register right. If the carry is enabled, that bit is fed into
// bit 7.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is enabled before rotating
uint8_t SM83::rr_h() {
    // Check if bit 0 is enabled before rotating
    uint8_t c_check = h_reg & 0x01;
    // Check if the carry bit is enabled
    if(getFlag(C) == 1) {
        h_reg = (h_reg >> 1);       // Shift right one
        h_reg |= (1 << 7);              // Enable bit 7
    }
    else
        h_reg = (h_reg >> 1);
    // Check if the carry flag needs to be set
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(h_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset other flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Rotates the bits in L register right. If the carry is enabled, that bit is fed into
// bit 7.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is enabled before rotating
uint8_t SM83::rr_l() {
    // Check if bit 0 is enabled before rotating
    uint8_t c_check = l_reg & 0x01;
    // Check if the carry bit is enabled
    if(getFlag(C) == 1) {
        l_reg = (l_reg >> 1);       // Shift right one
        l_reg |= (1 << 7);              // Enable bit 7
    }
    else
        l_reg = (l_reg >> 1);
    // Check if the carry flag needs to be set
    if(c_check)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check for the zero flag
    if(l_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset other flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Push the current PC to the stack and jump to 0x0000.
uint8_t SM83::rst_00h() {
    // Push the current address to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Jump to address 0x0000
    pc = 0x0000;
    return 0;
}

// Push the current PC to the stack and jump to 0x0008.
uint8_t SM83::rst_08h() {
    // Push the current address to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Jump to address 0x0008
    pc = 0x0008;
    return 0;
}

// Push the current PC to the stack and jump to 0x0010.
uint8_t SM83::rst_10h() {
    // Push the current address to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Jump to address 0x0010
    pc = 0x0010;
    return 0;
}

// Push the current PC to the stack and jump to 0x0018.
uint8_t SM83::rst_18h() {
    // Push the current address to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Jump to address 0x0018
    pc = 0x0018;
    return 0;
}

// Push the current PC to the stack and jump to 0x0020.
uint8_t SM83::rst_20h() {
    // Push the current address to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Jump to address 0x0020
    pc = 0x0020;
    return 0;
}

// Push the current PC to the stack and jump to 0x0028.
uint8_t SM83::rst_28h() {
    // Push the current address to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Jump to address 0x0028
    pc = 0x0028;
    return 0;
}

// Push the current PC to the stack and jump to 0x0030.
uint8_t SM83::rst_30h() {
    // Push the current address to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Jump to address 0x0030
    pc = 0x0030;
    return 0;
}

// Push the current PC to the stack and jump to 0x0038.
uint8_t SM83::rst_38h() {
    // Push the current address to the stack
    sp--;
    write(sp, (pc >> 8));
    sp--;
    write(sp, (pc & 0xff));
    // Jump to address 0x0038
    pc = 0x0038;
    return 0;
}

// Rotate the contents of register A right one bit.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is one before the rotation
uint8_t SM83::rrca() {
    // If bit 0 is set, set the carry bit
    if(a_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits right 1
    // First shift all bits right one, then or with all bits shifted right 7.
    a_reg = (a_reg >> 1) | (a_reg << 7);
    // Reset the rest of the flags
    setFlag(Z, 0);
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotate the contents of register A right one bit.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is one before the rotation
uint8_t SM83::rrc_a() {
    // If bit 0 is set, set the carry bit
    if(a_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits right 1
    // First shift all bits right one, then or with all bits shifted right 7.
    a_reg = (a_reg >> 1) | (a_reg << 7);
    // Check for zero flag
    if(a_reg == 0x000)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotate the data stored at the absolute address stored in HL right one.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is one before the rotation
uint8_t SM83::rrc_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store
    uint8_t data = fetch();
    // If bit 0 is set, set the carry bit
    if(data & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits right 1
    // First shift all bits right one, then or with all bits shifted right 7.
    data = (data >> 1) | (data << 7);
    // Write the data back to the address
    write(addr_abs, data);
    // Check for zero flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotate the contents of register B right one bit.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is one before the rotation
uint8_t SM83::rrc_b() {
    // If bit 0 is set, set the carry bit
    if(b_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits right 1
    // First shift all bits right one, then or with all bits shifted right 7.
    b_reg = (b_reg >> 1) | (b_reg << 7);
    // Check the zero flag
    if(b_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotate the contents of register C right one bit.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is one before the rotation
uint8_t SM83::rrc_c() {
    // If bit 0 is set, set the carry bit
    if(c_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits right 1
    // First shift all bits right one, then or with all bits shifted right 7.
    c_reg = (c_reg >> 1) | (c_reg << 7);
    // Check for the zero flag
    if(c_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotate the contents of register D right one bit.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is one before the rotation
uint8_t SM83::rrc_d() {
    // If bit 0 is set, set the carry bit
    if(d_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits right 1
    // First shift all bits right one, then or with all bits shifted right 7.
    d_reg = (d_reg >> 1) | (d_reg << 7);
    // Check the zero flag
    if(d_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotate the contents of register E right one bit.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is one before the rotation
uint8_t SM83::rrc_e() {
    // If bit 0 is set, set the carry bit
    if(e_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits right 1
    // First shift all bits right one, then or with all bits shifted right 7.
    e_reg = (e_reg >> 1) | (e_reg << 7);
    // Check for the zero flag
    if(e_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotate the contents of register H right one bit.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is one before the rotation
uint8_t SM83::rrc_h() {
    // If bit 0 is set, set the carry bit
    if(h_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits right 1
    // First shift all bits right one, then or with all bits shifted right 7.
    h_reg = (h_reg >> 1) | (h_reg << 7);
    // Check for zero flag
    if(h_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Rotate the contents of register L right one bit.
// Flags:
//  -Z: Reset to 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is one before the rotation
uint8_t SM83::rrc_l() {
    // If bit 0 is set, set the carry bit
    if(l_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Rotate bits right 1
    // First shift all bits right one, then or with all bits shifted right 7.
    l_reg = (l_reg >> 1) | (l_reg << 7);
    // Check for zero flag
    if(l_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(H, 0);
    setFlag(N, 0);
    return 0;
}

// Subtract the A register from itself with carry going into a copy of A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if the result resets bit 4, if (A & 0xf) = 0xf, or if the copy of A overflows after carry has been added.
//  -C: Set if (A_copy + carry) > A
uint8_t SM83::sbc_a_a() {
    // Create a copy of operand r8
    uint8_t cp_r8 = a_reg;
    // 16-bit copy for overflow check
    uint16_t cp_r8_16 = a_reg;
    // Need to see if the carry flag is enabled and inc r8
    if(getFlag(C) == 1) {
        cp_r8++;
        cp_r8_16++;
    }
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (cp_r8 & 0xf);
    a_reg -= cp_r8;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (a_reg & 0xf) == 0x0f || cp_r8_16 > 0xff)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value stored at the absolute address in HL from
// register A with the carry going into that 8-bit data value.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if the result resets bit 4, if (A & 0xf) = 0xf, or if d8 overflows after carry has been added.
//  -C: Set if (d8 + carry) > A
uint8_t SM83::sbc_a_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch the data and store
    uint8_t data = fetch();
    // Create a copy of operand d8
    uint8_t cp_d8 = data;
    // 16-bit copy for overflow check
    uint16_t cp_d8_16 = data;
    // Need to see if the carry flag is enabled and inc r8
    if(getFlag(C) == 1) {
        cp_d8++;
        cp_d8_16++;
    }
    // Carry check
    if(cp_d8_16 > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (cp_d8 & 0xf);
    a_reg -= cp_d8;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (a_reg & 0xf) == 0x0f || cp_d8_16 > 0xff)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value in register B from register A with the carry going into B.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if the result resets bit 4, if (A & 0xf) = 0xf, or if B overflows after carry has been added.
//  -C: Set if (B + carry) > A
uint8_t SM83::sbc_a_b() {
    // Create a copy of operand r8
    uint8_t cp_r8 = b_reg;
    // 16-bit copy for overflow check
    uint16_t cp_r8_16 = b_reg;
    // Need to see if the carry flag is enabled and inc r8
    if(getFlag(C) == 1) {
        cp_r8++;
        cp_r8_16++;
    }
    // Carry check
    if(cp_r8_16 > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (cp_r8 & 0xf);
    a_reg -= cp_r8;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (a_reg & 0xf) == 0x0f || cp_r8_16 > 0xff)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value in register C from register A with the carry going into C.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if the result resets bit 4, if (A & 0xf) = 0xf, or if C overflows after carry has been added.
//  -C: Set if (C + carry) > A
uint8_t SM83::sbc_a_c() {
    // Create a copy of operand r8
    uint8_t cp_r8 = c_reg;
    // 16-bit copy for overflow check
    uint16_t cp_r8_16 = c_reg;
    // Need to see if the carry flag is enabled and inc r8
    if(getFlag(C) == 1) {
        cp_r8++;
        cp_r8_16++;
    }
    // Carry check
    if(cp_r8_16 > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (cp_r8 & 0xf);
    a_reg -= cp_r8;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (a_reg & 0xf) == 0x0f || cp_r8_16 > 0xff)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value in register D from register A with the carry going into D.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if the result resets bit 4, if (A & 0xf) = 0xf, or if D overflows after carry has been added.
//  -C: Set if (D + carry) > A
uint8_t SM83::sbc_a_d() {
    // Create a copy of operand r8
    uint8_t cp_r8 = d_reg;
    // 16-bit copy for overflow check
    uint16_t cp_r8_16 = d_reg;
    // Need to see if the carry flag is enabled and inc r8
    if(getFlag(C) == 1) {
        cp_r8++;
        cp_r8_16++;
    }
    // Carry check
    if(cp_r8_16 > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (cp_r8 & 0xf);
    a_reg -= cp_r8;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (a_reg & 0xf) == 0x0f || cp_r8_16 > 0xff)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the immediate 8-bit data from register A with the carry going into d8.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if the result resets bit 4, if (A & 0xf) = 0xf, or if d8 overflows after carry has been added.
//  -C: Set if (d8 + carry) > A
uint8_t SM83::sbc_a_d8() {
    // Need to load data
    uint8_t data = read(pc++);
    // Create a copy of operand r8
    uint8_t cp_r8 = data;
    // 16-bit copy for overflow check
    uint16_t cp_r8_16 = data;
    // Need to see if the carry flag is enabled and inc r8
    if(getFlag(C) == 1) {
        cp_r8++;
        cp_r8_16++;
    }
    // Carry check
    if(cp_r8_16 > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (cp_r8 & 0xf);
    a_reg -= cp_r8;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (a_reg & 0xf) == 0x0f || cp_r8_16 > 0xff)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value in register E from register A with the carry going into E.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if the result resets bit 4, if (A & 0xf) = 0xf, or if E overflows after carry has been added.
//  -C: Set if (E + carry) > A
uint8_t SM83::sbc_a_e() {
    // Create a copy of operand r8
    uint8_t cp_r8 = e_reg;
    // 16-bit copy for overflow check
    uint16_t cp_r8_16 = e_reg;
    // Need to see if the carry flag is enabled and inc r8
    if(getFlag(C) == 1) {
        cp_r8++;
        cp_r8_16++;
    }
    // Carry check
    if(cp_r8_16 > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (cp_r8 & 0xf);
    a_reg -= cp_r8;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (a_reg & 0xf) == 0x0f || cp_r8_16 > 0xff)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value in register H from register A with the carry going into H.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if the result resets bit 4, if (A & 0xf) = 0xf, or if H overflows after carry has been added.
//  -C: Set if (H + carry) > A
uint8_t SM83::sbc_a_h() {
    // Create a copy of operand r8
    uint8_t cp_r8 = h_reg;
    // 16-bit copy for overflow check
    uint16_t cp_r8_16 = h_reg;
    // Need to see if the carry flag is enabled and inc r8
    if(getFlag(C) == 1) {
        cp_r8++;
        cp_r8_16++;
    }
    // Carry check
    if(cp_r8_16 > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (cp_r8 & 0xf);
    a_reg -= cp_r8;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (a_reg & 0xf) == 0x0f || cp_r8_16 > 0xff)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value in register L from register A with the carry going into L.
// Flags:
//  -Z: Set if the result is 0
//  -N: Set to 1
//  -H: Set if the result resets bit 4, if (A & 0xf) = 0xf, or if L overflows after carry has been added.
//  -C: Set if (L + carry) > A
uint8_t SM83::sbc_a_l() {
    // Create a copy of operand r8
    uint8_t cp_r8 = l_reg;
    // 16-bit copy for overflow check
    uint16_t cp_r8_16 = l_reg;
    // Need to see if the carry flag is enabled and inc r8
    if(getFlag(C) == 1) {
        cp_r8++;
        cp_r8_16++;
    }
    // Carry check
    if(cp_r8_16 > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (cp_r8 & 0xf);
    a_reg -= cp_r8;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check
    if((h_check & 0x10) == 0x10 || (a_reg & 0xf) == 0x0f || cp_r8_16 > 0xff)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Set the carry flag. Reset the sign and half carry flags.
uint8_t SM83::scf() {
    setFlag(C, 1);
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Generic function to set bit n in register r8.
uint8_t SM83::set_n_r8() {
    // Get the opcode of the instruction that is to be executed
    uint8_t prev_opcode = read(pc - 1);
    // Access the prefix opcode table as this function is for a prefixed opcode
    uint8_t n = prefix_lookup[prev_opcode].n;
    // Need to check the size of the operand vector
    // The only way the size of the vector will be 2 is if we are working with
    // the HL register pair, and need to get the absolute address stored in it.
    if(prefix_lookup[prev_opcode].operands.size() == 2) {
        // Grab the data from the H & L pointers from the operands vector
        // Operands are little endian
        uint8_t lowByte = *prefix_lookup[prev_opcode].operands[0];
        uint8_t highByte = *prefix_lookup[prev_opcode].operands[1];
        addr_abs = (highByte << 8) | lowByte;
        uint8_t data = fetch();
        data |= (1 << n);
        // Write the data
        write(addr_abs, data);
        return 0;
    }
    // If we are here, that means only 1 operand
    uint8_t *r8 = prefix_lookup[prev_opcode].operands[0];
    // Reset bit n in register r8
    *r8 |= (1 << n);
    return 0;
}

// Shift the bits in the A register left 1.
// Flags:
// -Z: Set if the result is 0
// -N: Reset to 0
// -H: Reset to 0
// -C: Set if bit 7 is set before the shift
uint8_t SM83::sla_a() {
    // Check if bit 7 is set
    // If so, set carry flag
    if(a_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift the register left 1
    a_reg = a_reg << 1;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the data stored at the absolute address in HL left 1.
// Flags:
// -Z: Set if the result is 0
// -N: Reset to 0
// -H: Reset to 0
// -C: Set if bit 7 is set before the shift
uint8_t SM83::sla_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Check if bit 7 is set
    // If so, set carry flag
    if(data & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift the register left 1
    data = data << 1;
    // Check for zero flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the B register left 1.
// Flags:
// -Z: Set if the result is 0
// -N: Reset to 0
// -H: Reset to 0
// -C: Set if bit 7 is set before the shift
uint8_t SM83::sla_b() {
    // Check if bit 7 is set
    // If so, set carry flag
    if(b_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift the register left 1
    b_reg = b_reg << 1;
    // Check for zero flag
    if(b_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the C register left 1.
// Flags:
// -Z: Set if the result is 0
// -N: Reset to 0
// -H: Reset to 0
// -C: Set if bit 7 is set before the shift
uint8_t SM83::sla_c() {
    // Check if bit 7 is set
    // If so, set carry flag
    if(c_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift the register left 1
    c_reg = c_reg << 1;
    // Check for zero flag
    if(c_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the D register left 1.
// Flags:
// -Z: Set if the result is 0
// -N: Reset to 0
// -H: Reset to 0
// -C: Set if bit 7 is set before the shift
uint8_t SM83::sla_d() {
    // Check if bit 7 is set
    // If so, set carry flag
    if(d_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift the register left 1
    d_reg = d_reg << 1;
    // Check for zero flag
    if(d_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the E register left 1.
// Flags:
// -Z: Set if the result is 0
// -N: Reset to 0
// -H: Reset to 0
// -C: Set if bit 7 is set before the shift
uint8_t SM83::sla_e() {
    // Check if bit 7 is set
    // If so, set carry flag
    if(e_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift the register left 1
    e_reg = e_reg << 1;
    // Check for zero flag
    if(e_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the H register left 1.
// Flags:
// -Z: Set if the result is 0
// -N: Reset to 0
// -H: Reset to 0
// -C: Set if bit 7 is set before the shift
uint8_t SM83::sla_h() {
    // Check if bit 7 is set
    // If so, set carry flag
    if(h_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift the register left 1
    h_reg = h_reg << 1;
    // Check for zero flag
    if(h_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the L register left 1.
// Flags:
// -Z: Set if the result is 0
// -N: Reset to 0
// -H: Reset to 0
// -C: Set if bit 7 is set before the shift
uint8_t SM83::sla_l() {
    // Check if bit 7 is set
    // If so, set carry flag
    if(l_reg & (1 << 7))
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift the register left 1
    l_reg = l_reg << 1;
    // Check for zero flag
    if(l_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the A register right 1. Bit 7 will remain unchanged.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::sra_a() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(a_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check to see if bit 7 is set
    if(a_reg & (1 << 7)) {
        // Shift right 1
        a_reg = a_reg >> 1;
        // Set bit 7
        a_reg |= (1 << 7);
    }
    else
        a_reg = a_reg >> 1;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the data at the absolute address in HL right 1. Bit 7 will remain unchanged.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::sra_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Check if bit 0 is set
    // If so, set carry flag
    if(data & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check to see if bit 7 is set
    if(data & (1 << 7)) {
        // Shift right 1
        data = data >> 1;
        // Set bit 7
        data |= (1 << 7);
    }
    else
        data = data>> 1;
    // Write the data
    write(addr_abs, data);
    // Check for zero flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the B register right 1. Bit 7 will remain unchanged.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::sra_b() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(b_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check to see if bit 7 is set
    if(b_reg & (1 << 7)) {
        // Shift right 1
        b_reg = b_reg >> 1;
        // Set bit 7
        b_reg |= (1 << 7);
    }
    else
        b_reg = b_reg >> 1;
    // Check for zero flag
    if(b_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the C register right 1. Bit 7 will remain unchanged.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::sra_c() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(c_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check to see if bit 7 is set
    if(c_reg & (1 << 7)) {
        // Shift right 1
        c_reg = c_reg >> 1;
        // Set bit 7
        c_reg |= (1 << 7);
    }
    else
        c_reg = c_reg >> 1;
    // Check for zero flag
    if(c_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the D register right 1. Bit 7 will remain unchanged.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::sra_d() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(d_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check to see if bit 7 is set
    if(d_reg & (1 << 7)) {
        // Shift right 1
        d_reg = d_reg >> 1;
        // Set bit 7
        d_reg |= (1 << 7);
    }
    else
        d_reg = d_reg >> 1;
    // Check for zero flag
    if(d_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the E register right 1. Bit 7 will remain unchanged.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::sra_e() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(e_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check to see if bit 7 is set
    if(e_reg & (1 << 7)) {
        // Shift right 1
        e_reg = e_reg >> 1;
        // Set bit 7
        e_reg |= (1 << 7);
    }
    else
        e_reg = e_reg >> 1;
    // Check for zero flag
    if(e_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the H register right 1. Bit 7 will remain unchanged.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::sra_h() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(h_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check to see if bit 7 is set
    if(h_reg & (1 << 7)) {
        // Shift right 1
        h_reg = h_reg >> 1;
        // Set bit 7
        h_reg |= (1 << 7);
    }
    else
        h_reg = h_reg >> 1;
    // Check for zero flag
    if(h_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the L register right 1. Bit 7 will remain unchanged.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::sra_l() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(l_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Check to see if bit 7 is set
    if(l_reg & (1 << 7)) {
        // Shift right 1
        l_reg = l_reg >> 1;
        // Set bit 7
        l_reg |= (1 << 7);
    }
    else
        l_reg = l_reg >> 1;
    // Check for zero flag
    if(l_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the A register right 1. Bit 7 resets to 0.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::srl_a() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(a_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift right one bit
    a_reg = a_reg >> 1;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the data stored at the absolute address in HL right 1. Bit 7 resets to 0.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::srl_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    // Check if bit 0 is set
    // If so, set carry flag
    if(data & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift right one bit
    data = data >> 1;
    // Write the data
    write(addr_abs, data);
    // Check for zero flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the B register right 1. Bit 7 resets to 0.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::srl_b() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(b_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift right one bit
    b_reg = b_reg >> 1;
    // Check for zero flag
    if(b_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the C register right 1. Bit 7 resets to 0.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::srl_c() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(c_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift right one bit
    c_reg = c_reg >> 1;
    // Check for zero flag
    if(c_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the D register right 1. Bit 7 resets to 0.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::srl_d() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(d_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift right one bit
    d_reg = d_reg >> 1;
    // Check for zero flag
    if(d_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the E register right 1. Bit 7 resets to 0.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::srl_e() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(e_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift right one bit
    e_reg = e_reg >> 1;
    // Check for zero flag
    if(e_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the H register right 1. Bit 7 resets to 0.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::srl_h() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(h_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift right one bit
    h_reg = h_reg >> 1;
    // Check for zero flag
    if(h_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

// Shift the bits in the L register right 1. Bit 7 resets to 0.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Set if bit 0 is set before the shift
uint8_t SM83::srl_l() {
    // Check if bit 0 is set
    // If so, set carry flag
    if(l_reg & 0x01)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    // Shift right one bit
    l_reg = l_reg >> 1;
    // Check for zero flag
    if(l_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset the rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    return 0;
}

uint8_t SM83::stop_d8() {
    // Need to pause the Game Boy when this opcode is run
    // To exit stop check link: https://gbdev.io/pandocs/Reducing_Power_Consumption.html?highlight=stop#using-the-stop-instruction
    return 0;
}

// Subtract the A register from itself.
// Flags:
//  -Z: Set to 1
//  -N: Set to 1
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::sub_a() {
    a_reg -= a_reg;
    // Set the Zero and Sign flags
    setFlag(Z, 1);
    setFlag(N, 1);
    // Reset the Half carry and Carry flags
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Subtract the value stored at the absolute address in HL from A.
// Flags:
//  -Z: If the result is 0
//  -N: Set to 1
//  -H: Set if the result of the subtraction resets bit 4
//  -C: Set if (HL) > A
uint8_t SM83::sub_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch the data
    uint8_t data = fetch();
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (data & 0xf);
    // Check for carry flag
    if(data > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    a_reg -= data;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);

    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value of B from A.
// Flags:
//  -Z: If the result is 0
//  -N: Set to 1
//  -H: Set if the result of the subtraction resets bit 4
//  -C: Set if B > A
uint8_t SM83::sub_b() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (b_reg & 0xf);
    // Check for carry flag
    if(b_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    a_reg -= b_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);
    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value of C from A.
// Flags:
//  -Z: If the result is 0
//  -N: Set to 1
//  -H: Set if the result of the subtraction resets bit 4
//  -C: Set if C > A
uint8_t SM83::sub_c() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (c_reg & 0xf);
    // Check for carry flag
    if(c_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    a_reg -= c_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);

    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value of D from A.
// Flags:
//  -Z: If the result is 0
//  -N: Set to 1
//  -H: Set if the result of the subtraction resets bit 4
//  -C: Set if D > A
uint8_t SM83::sub_d() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (d_reg & 0xf);
    // Check for carry flag
    if(d_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    a_reg -= d_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);

    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract d8 from A.
// Flags:
//  -Z: If the result is 0
//  -N: Set to 1
//  -H: Set if the result of the subtraction resets bit 4
//  -C: Set if d8 > A
uint8_t SM83::sub_d8() {
    // Need to get the data
    uint8_t data = read(pc++);
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (data & 0xf);
    // Check for carry flag
    if(data > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    a_reg -= data;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);

    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value of E from A.
// Flags:
//  -Z: If the result is 0
//  -N: Set to 1
//  -H: Set if the result of the subtraction resets bit 4
//  -C: Set if E > A
uint8_t SM83::sub_e() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (e_reg & 0xf);
    // Check for carry flag
    if(e_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    a_reg -= e_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);

    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value of H from A.
// Flags:
//  -Z: If the result is 0
//  -N: Set to 1
//  -H: Set if the result of the subtraction resets bit 4
//  -C: Set if H > A
uint8_t SM83::sub_h() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (h_reg & 0xf);
    // Check for carry flag
    if(h_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    a_reg -= h_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);

    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Subtract the value of L from A.
// Flags:
//  -Z: If the result is 0
//  -N: Set to 1
//  -H: Set if the result of the subtraction resets bit 4
//  -C: Set if L > A
uint8_t SM83::sub_l() {
    // Disable high nibble bits for the half carry check
    uint8_t h_check = (a_reg & 0xf) - (l_reg & 0xf);
    // Check for carry flag
    if(l_reg > a_reg)
        setFlag(C, 1);
    else
        setFlag(C, 0);
    a_reg -= l_reg;
    // Zero flag check
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Half carry check - checking to see if the addition enabled bit 4
    if((h_check & 0x10) == 0x10)
        setFlag(H, 1);
    else
        setFlag(H, 0);

    // Set the sign flag
    setFlag(N, 1);
    return 0;
}

// Swap the upper 4 bits in register A with the lower 4.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::swap_a() {
    a_reg = ((a_reg & 0x0f) << 4 | (a_reg & 0xf0) >> 4);
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Swap the upper 4 bits in the data stored at the absolute address in HL with the lower 4.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::swap_abs_hl() {
    // Need to get the data
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    uint8_t data = fetch();
    data = ((data & 0x0f) << 4 | (data & 0xf0) >> 4);
    // Write the data
    write(addr_abs, data);
    // Check for zero flag
    if(data == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Swap the upper 4 bits in register B with the lower 4.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::swap_b() {
    b_reg = ((b_reg & 0x0f) << 4 | (b_reg & 0xf0) >> 4);
    // Check for zero flag
    if(b_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Swap the upper 4 bits in register C with the lower 4.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::swap_c() {
    c_reg = ((c_reg & 0x0f) << 4 | (c_reg & 0xf0) >> 4);
    // Check for zero flag
    if(c_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Swap the upper 4 bits in register D with the lower 4.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::swap_d() {
    d_reg = ((d_reg & 0x0f) << 4 | (d_reg & 0xf0) >> 4);
    // Check for zero flag
    if(d_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Swap the upper 4 bits in register E with the lower 4.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::swap_e() {
    e_reg = ((e_reg & 0x0f) << 4 | (e_reg & 0xf0) >> 4);
    // Check for zero flag
    if(e_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Swap the upper 4 bits in register H with the lower 4.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::swap_h() {
    h_reg = ((h_reg & 0x0f) << 4 | (h_reg & 0xf0) >> 4);
    // Check for zero flag
    if(h_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Swap the upper 4 bits in register L with the lower 4.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::swap_l() {
    l_reg = ((l_reg & 0x0f) << 4 | (l_reg & 0xf0) >> 4);
    // Check for zero flag
    if(l_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset rest of the flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Exclusive or the A register with itself.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::xor_a() {
    a_reg ^= a_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Exclusive or the A register with the data stored at the absolute address in HL. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::xor_abs_hl() {
    // Need to get the data from HL
    uint16_t lowByte = l_reg;
    uint16_t highByte = h_reg;
    addr_abs = (highByte << 8) | lowByte;
    // Fetch and store
    uint8_t data = fetch();
    a_reg ^= data;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Exclusive or the A and B registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::xor_b() {
    a_reg ^= b_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}


// Exclusive or the A and C registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::xor_c() {
    a_reg ^= c_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Exclusive or the A and D registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::xor_d() {
    a_reg ^= d_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}
// Exclusive or the A register with the immediate 8-bit data. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::xor_d8() {
    // Need to get the data
    uint8_t data = read(pc++);
    a_reg ^= data;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Exclusive or the A and E registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::xor_e() {
    a_reg ^= e_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Exclusive or the A and H registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::xor_h() {
    a_reg ^= h_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// Exclusive or the A and L registers. Store in A.
// Flags:
//  -Z: Set if the result is 0
//  -N: Reset to 0
//  -H: Reset to 0
//  -C: Reset to 0
uint8_t SM83::xor_l() {
    a_reg ^= l_reg;
    // Check for zero flag
    if(a_reg == 0x00)
        setFlag(Z, 1);
    else
        setFlag(Z, 0);
    // Reset sign, half carry, and carry flags
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

// No operation.
uint8_t SM83::nop() {
    return 0;
}

// Illegal opcode
uint8_t SM83::xxx() {
    return 0;
}
