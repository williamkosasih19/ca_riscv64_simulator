#pragma once
/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2020

  Header file for enums

**************************************************************** */
enum opcode_e
{
  reg_op = 0b0110011,
  imm_op = 0b0010011,
  lod_op = 0b0000011,
  str_op = 0b0100011,
  bra_op = 0b1100011,
  lui_op = 0b0110111,
  auipc_op = 0b0010111,
  imm64_op = 0b0011011,
  jal_op = 0b1101111,
  jalr_op = 0b1100111,
  reg64_op = 0b0111011,
  sys_op = 0b1110011,
};
enum reg_funct_e
{
  add_reg = 0b0000000000,
  sub_reg = 0b0000100000,
  sll_reg = 0b0010000000,
  slt_reg = 0b0100000000,
  sltu_reg = 0b0110000000,
  xor_reg = 0b1000000000,
  srl_reg = 0b1010000000,
  sra_reg = 0b1010100000,
  or_reg = 0b1100000000,
  and_reg = 0b1110000000
};
enum imm_funct_e
{
  addi_imm = 0b000,
  slti_imm = 0b010,
  sltiu_imm = 0b011,
  xori_imm = 0b100,
  ori_imm = 0b110,
  andi_imm = 0b111,
  slli_imm = 0b001,
  slli10_imm = 0b001000000,
  srli_imm = 0b101,
  srli10_imm = 0b101000000,
  srai10_imm = 0b101010000
};
enum bra_funct_e
{
  beq_bra = 0b000,
  bne_bra = 0b001,
  blt_bra = 0b100,
  bge_bra = 0b101,
  bltu_bra = 0b110,
  bgeu_bra = 0b111
};
enum imm64_funct_e
{
  addiw_imm64 = 0b000,
  slliw_imm64 = 0b001,
  slliw10_imm64 = 0b0010000000,
  srliw_imm64 = 0b101,
  srliw10_imm64 = 0b1010000000,
  sraiw_imm64 = 0b101,
  sraiw10_imm64 = 0b1010100000
};
enum reg64_funct_e
{
  addw_reg64 = 0b0000000000,
  subw_reg64 = 0b0000100000,
  sllw_reg64 = 0b0010000000,
  srlw_reg64 = 0b1010000000,
  sraw_reg64 = 0b1010100000
};
enum lod_funct_e
{
  lb_lod = 0b000,
  lh_lod = 0b001,
  lw_lod = 0b010,
  lbu_lod = 0b100,
  lhu_lod = 0b101,
  lwu_lod = 0b110,
  ld_lod = 0b011
};
enum stor_funct_e
{
  sb_stor = 0b000,
  sh_stor = 0b001,
  sw_stor = 0b010,
  sd_stor = 0b011
};
enum sys_funct_e
{
  ecall_sys = 0b000,
  ebreak_sys = 0b001,
  mret_sys = 0b001100000010,
  uret_sys = 0b000000000010,
  sret_sys = 0b000100000010,
  csrrw_zicsr = 0b001,
  csrrs_zicsr = 0b010,
  csrrc_zicsr = 0b011,
  csrrwi_zicsr = 0b101,
  csrrsi_zicsr = 0b110,
  csrrci_zicsr = 0b111
};
enum csr_reg_e
{
  mvendorid_csr_reg = 0xF11,
  marchid_csr_reg = 0xF12,
  mimpid_csr_reg = 0xF13,
  mhartid_csr_reg = 0xF14,
  mstatus_csr_reg = 0x300,
  misa_csr_reg = 0x301,
  mie_csr_reg = 0x304,
  mtvec_csr_reg = 0x305,
  mscratch_csr_reg = 0x340,
  mepc_csr_reg = 0x341,
  mcause_csr_reg = 0x342,
  mtval_csr_reg = 0x343,
  mip_csr_reg = 0x344
};
enum csr_cause_e
{
  instr_misaligned_csr_cause = 0,
  illegal_instr_csr_cause = 2,
  breakpoint_csr_cause = 3,
  load_misaligned_csr_cause = 4,
  store_misaligned_csr_cause = 6,
  env_call_u_mode_csr_cause = 8,
  enc_call_m_mode_csr_cause = 11
};
enum memory_misaligned_e
{
  none_misaligned,
  pc_misaligned,
  load_misaligned,
  store_misaligned
};
enum cause_interrupt_e
{
  user_software = 0,
  machine_software = 3,
  user_timer = 4,
  machine_timer = 7,
  user_external = 8,
  machine_external = 11
};
enum int_e
{
  us_int = 0x01,
  ms_int = 1 << 3,
  ut_int = 1 << 4,
  mt_int = 1 << 7,
  ue_int = 1 << 8,
  me_int = 1 << 11,
};