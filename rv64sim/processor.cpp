/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2020

   Class member for Processor

**************************************************************** */
#include <bitset>
#include <iostream>
#include <sstream>

#include "memory.h"
#include "memoryImpl.h"
#include "outputImpl.h"
#include "processor.h"
#include "enum.h"

using namespace std;

reg::reg()
{
  registers.resize(32);
  fill(registers.begin(), registers.end(), 0);
}

uint64_t& reg::operator[](const byte_t reg_num)
{
  if (reg_num == 0)
    registers[reg_num] = 0;
  return registers[reg_num];
}

csr_reg::csr_reg::csr_reg() {}

uint64_t& csr_reg::operator[](const uint32_t reg_num)
{
  switch (csr_reg_e(reg_num))
  {
  case mvendorid_csr_reg:
    return (reg_map[reg_num] = 0);
    break;
  case marchid_csr_reg:
    return (reg_map[reg_num] = 0);
    break;
  case mimpid_csr_reg:
    return (reg_map[reg_num] = 0x2020020000000000);
    break;
  case mhartid_csr_reg:
    return (reg_map[reg_num] = 0);
    break;
  case mstatus_csr_reg:
    return (reg_map[reg_num] = (reg_map[reg_num] & 0x200001888) | 0x200000000);
    break;
  case misa_csr_reg:
    return (reg_map[reg_num] = 0x8000000000100100);
    break;
  case mie_csr_reg:
    return (reg_map[reg_num] &= 0x999);
    break;
  case mtvec_csr_reg:
    if (reg_map[reg_num] % 2)
      reg_map[reg_num] &= 0xFFFFFFFFFFFFFF01;
    else
      reg_map[reg_num] &= 0xFFFFFFFFFFFFFFFD;
    return reg_map[reg_num];
    break;
  case mscratch_csr_reg:
    return reg_map[reg_num];
    break;
  case mepc_csr_reg:
    return (reg_map[reg_num] &= 0xFFFFFFFFFFFFFFFC);
    break;
  case mcause_csr_reg:
    return (reg_map[reg_num] &= 0x800000000000000F);
    break;
  case mtval_csr_reg:
    return reg_map[reg_num];
    break;
  case mip_csr_reg:
    return (reg_map[reg_num] &= 0x999);
    break;
  default:
    return invalid_reg;
    break;
  }
}

processor::processor(memory* const main_memory, 
                     const bool_t verbose, const bool_t stage2)
{
  pc = uint64_t(0);
  instruction_ctr = 0;
  memory_ptr = main_memory;
  main_memory->set_processor(this);
  prv = 3;

}

void processor::show_pc()
{
  // :TODO: Make this conform to the format requested
  printf("%016llx\n", pc);
}

void processor::set_pc(const uint64_t new_pc) { pc = new_pc; }

void processor::show_reg(const uint32_t reg_num)
{
  printf("%016llx\n", registers[reg_num]);
}

void processor::set_reg(const uint32_t reg_num, const uint64_t new_value)
{
  registers[reg_num] = new_value;
}

void processor::raise_exception(uint16_t cause, uint64_t address)
{
  csr[mcause_csr_reg] = cause;
  csr[mepc_csr_reg] = pc;
  pc = (csr[mtvec_csr_reg] & 0xFFFFFFFFFFFFFFFC);

  // Backup mie value
  const byte_t mie = (csr[mstatus_csr_reg] >> 3) % 2;
  // Clear mie, mpp and mpp bits
  csr[mstatus_csr_reg] &= 0xFFFFFFFFFFFFE777;
  // Copy backed up mie value to mpie
  csr[mstatus_csr_reg] |= (uint64_t(prv) << 11 | uint64_t(mie) << 7);
  prv = 3;
  
  switch (cause)
  {
    case illegal_instr_csr_cause:
      csr[mtval_csr_reg] = instruction;
      break;
    case instr_misaligned_csr_cause:
    case load_misaligned_csr_cause:
    case store_misaligned_csr_cause:
      csr[mtval_csr_reg] = address;
      break;
    }
}

void processor::execute(const uint32_t num, const bool_t breakpoint_check)
{
  // Here just to make this pass stage 1 dot test.
  if (pc % 4 != 0)
  {
    printf("Error: misaligned pc\n");
    return;
  }
  
  for (index32_t i = 0; i < num; i++)
  {
    // Service pending interrupt here.
    {
      const uint64_t mip = csr[mip_csr_reg];
      const uint64_t mie = csr[mie_csr_reg];
      const uint64_t mstatus = csr[mstatus_csr_reg];
      
      const byte_t mstatus_mie = (mstatus >> 3) & 1;

      bool_t updated = false;

      if (prv == 3 && mstatus_mie || prv == 0)
      {
        if ((mip & me_int) && (mie & me_int))
        {
          csr[mcause_csr_reg] =
              (uint64_t(1) << 63) | machine_external;
          updated = true;
        }
        else if ((mip & ms_int) && (mie & ms_int))
        {
          csr[mcause_csr_reg] =
              (uint64_t(1) << 63) | machine_software;
          updated = true;
        }
        else if ((mip & mt_int) && (mie & mt_int))
        {
          csr[mcause_csr_reg] =
              (uint64_t(1) << 63) | machine_timer;
          updated = true;
        }
        else if ((mip & ue_int) && (mie & ue_int))
        {
          csr[mcause_csr_reg] =
              (uint64_t(1) << 63) | user_external;
          updated = true;
        }
        else if ((mip & us_int) && (mie & us_int))
        {
          csr[mcause_csr_reg] =
              (uint64_t(1) << 63) | user_software;
          updated = true;
        }
        else if ((mip & ut_int) && (mie & ut_int))
        {
          csr[mcause_csr_reg] =
              (uint64_t(1) << 63) | user_timer;
          updated = true;
        }
        if (updated)
        {
          // Backup mie value
          const byte_t mie = (csr[mstatus_csr_reg] >> 3) & 1;
          // Clear mie, mpp and mpp bits
          csr[mstatus_csr_reg] &= 0xFFFFFFFFFFFFE777;
          // Copy backed up mie value to mpie
          csr[mstatus_csr_reg] |=
              (uint64_t(prv) << 11 | uint64_t(mie) << 7);
          csr[mepc_csr_reg] = pc;
          const uint64_t mtvec = csr[mtvec_csr_reg];
          if (mtvec % 2)
            pc = ((mtvec & 0xFFFFFFFFFFFFFFFC) +
                      4 * csr[mcause_csr_reg] &
                  0xFFFF);
          else
            pc = mtvec;
          prv = 3;
        }
      }
    }

    if (pc % 4 != 0)
    {
      raise_exception(instr_misaligned_csr_cause, pc);
      continue;
    }

    if (breakpoint_check && break_point_enabled && pc == break_point)
    {
      printf("Breakpoint reached at %016llx\n", pc);
      break;
    }

    instruction = memory_ptr->fetch_pc(pc);
    const char_t op = instruction & 0x0000007F;

    switch (opcode_e(op))
    {
    case bra_op:
    {
      const byte_t imm1 = (instruction & 0x00000F80) >> 7;
      const byte_t funct3 = (instruction & 0x00007000) >> 12;
      const byte_t rs1 = (instruction & 0x000F8000) >> 15;
      const byte_t rs2 = (instruction & 0x1F00000) >> 20;
      const byte_t imm2 = (instruction & 0xFE000000) >> 25;
      const int64_t imm =
          int64_t(uint64_t(imm2 >> 6) << 63 | uint64_t(imm1 % 2) << 62 |
                  uint64_t(imm2 & 0x3F) << 56 | uint64_t(imm1 >> 1) << 52) >>
          51;
      switch (bra_funct_e(funct3))
      {
      case bne_bra:
        if (registers[rs1] != registers[rs2])
          pc += imm - 4;
        break;
      case beq_bra:
        if (registers[rs1] == registers[rs2])
          pc += imm - 4;
        break;
      case blt_bra:
        if (int64_t(registers[rs1]) < int64_t(registers[rs2]))
          pc += imm - 4;
        break;
      case bge_bra:
        if (int64_t(registers[rs1]) >= int64_t(registers[rs2]))
          pc += imm - 4;
        break;
      case bltu_bra:
        if (registers[rs1] < registers[rs2])
          pc += imm - 4;
        break;
      case bgeu_bra:
        if (registers[rs1] >= registers[rs2])
          pc += imm - 4;
        break;
      default:
        raise_exception(illegal_instr_csr_cause, instruction);
        continue;
        break;
      }
    }
    break;
    case imm_op:
    {
      const byte_t rd = (instruction & 0x00000F80) >> 7;
      const byte_t funct3 = (instruction & 0x00007000) >> 12;
      const byte_t rs1 = (instruction & 0x000F8000) >> 15;
      const int64_t imm =
          static_cast<int64_t>((static_cast<uint64_t>(instruction) << 32)) >>
          52;

      switch (imm_funct_e(funct3))
      {
      case addi_imm:
        registers[rd] = registers[rs1] + imm;
        break;
      case slti_imm:
        if (static_cast<int64_t>(registers[rs1]) < imm)
          registers[rd] = 1;
        else
          registers[rd] = 0;
        break;
      case sltiu_imm:
        if (registers[rs1] < static_cast<uint64_t>(imm))
          registers[rd] = 1;
        else
          registers[rd] = 0;
        break;
      case xori_imm:
        registers[rd] = registers[rs1] ^ imm;
        break;
      case ori_imm:
        registers[rd] = registers[rs1] | imm;
        break;
      case andi_imm:
        registers[rd] = registers[rs1] & imm;
        break;
      case slli_imm:
      case srli_imm: // srai_imm has same funct3
      {
        const uint16_t funct = static_cast<uint16_t>(instruction >> 26) |
                               static_cast<uint16_t>(funct3) << 6;
        const byte_t shamt = (instruction & 0x3F00000) >> 20;
        switch (imm_funct_e(funct))
        {
        case slli10_imm:
          registers[rd] = registers[rs1] << shamt;
          break;
        case srli10_imm:
          registers[rd] = registers[rs1] >> shamt;
          break;
        case srai10_imm:
          registers[rd] = static_cast<int64_t>(registers[rs1]) >> shamt;
          break;
        default:
        raise_exception(illegal_instr_csr_cause, instruction);
        continue;
        break;
        }
      }
      }
    }
    break;
    case lod_op:
    {
      const byte_t rd = (instruction & 0x00000F80) >> 7;
      const byte_t funct3 = (instruction & 0x00007000) >> 12;
      const byte_t rs1 = (instruction & 0x000F8000) >> 15;
      const int64_t imm =
          static_cast<int64_t>((static_cast<uint64_t>(instruction) << 32)) >>
          52;
      const uint64_t address = registers[rs1] + imm;
      switch (lod_funct_e(funct3))
      {
      case lb_lod:
      {
        const uint64_t mem_byte = memory_ptr->read_n_bytes<byte_t>(address);
        registers[rd] = int64_t(uint64_t(mem_byte) << 56) >> 56;
      }
      break;
      case lbu_lod:
      {
        const uint64_t mem_byte = memory_ptr->read_n_bytes<byte_t>(address);
        registers[rd] = mem_byte & 0xFF;
      }
      break;
      case lh_lod:
      {
        if (address % 2 != 0)
        {
          raise_exception(load_misaligned_csr_cause, address);
          continue;
        }
        const uint64_t mem_halfword =
            memory_ptr->read_n_bytes<uint16_t>(int64_t(address));

        registers[rd] = int64_t(uint64_t(mem_halfword) << 48) >> 48;
      }
      break;
      case lhu_lod:
      {
        if (address % 2 != 0)
        {
          raise_exception(load_misaligned_csr_cause, address);
          continue;
        }
        const uint64_t mem_halfword =
            memory_ptr->read_n_bytes<uint16_t>(int64_t(address));

        registers[rd] = mem_halfword & 0xFFFF;
      }
      break;
      case lw_lod:
      {
        if (address % 4 != 0)
        {
          raise_exception(load_misaligned_csr_cause, address);
          continue;
        }
        const uint64_t mem_word =
            memory_ptr->read_n_bytes<uint32_t>(int64_t(address));

        registers[rd] = int64_t(uint64_t(mem_word) << 32) >> 32;
      }
      break;
      case lwu_lod:
      {
        if (address % 4 != 0)
        {
          raise_exception(load_misaligned_csr_cause, address);
          continue;
        }
        const uint64_t mem_word =
            memory_ptr->read_n_bytes<uint32_t>(int64_t(address));
        registers[rd] = mem_word & 0xFFFFFFFF;
      }
      break;
      case ld_lod:
      {
        if (address % 8 != 0)
        {
          raise_exception(load_misaligned_csr_cause, address);
          continue;
        }
        const uint64_t mem_dword = memory_ptr->read_doubleword(address);
        registers[rd] = mem_dword;
      }
      break;
      default:
        raise_exception(illegal_instr_csr_cause, instruction);
        continue;
        break;
      }
    }
    break;
    case reg_op:
    {
      const byte_t rd = (instruction & 0x00000F80) >> 7;
      const byte_t rs1 = (instruction & 0x000F8000) >> 15;
      const byte_t rs2 = (instruction & 0x1F00000) >> 20;
      const uint16_t funct =
          static_cast<uint16_t>(instruction >> 25) |
          static_cast<uint16_t>((instruction & 0x00007000) >> 12) << 7;
      switch (reg_funct_e(funct))
      {
      case add_reg:
        registers[rd] = registers[rs1] + registers[rs2];
        break;
      case sub_reg:
        registers[rd] = registers[rs1] - registers[rs2];
        break;
      case slt_reg:
        if (static_cast<int64_t>(registers[rs1]) <
            static_cast<int64_t>(registers[rs2]))
        {
          registers[rd] = 1;
        }
        else
          registers[rd] = 0;
        break;
      case sltu_reg:
        if (registers[rs1] < registers[rs2])
          registers[rd] = 1;
        else
          registers[rd] = 0;
        break;
      case xor_reg:
        registers[rd] = registers[rs1] ^ registers[rs2];
        break;
      case or_reg:
        registers[rd] = registers[rs1] | registers[rs2];
        break;
      case and_reg:
        registers[rd] = registers[rs1] & registers[rs2];
        break;
      case sll_reg:
        registers[rd] = registers[rs1] << registers[rs2];
        break;
      case srl_reg:
        registers[rd] = registers[rs1] >> registers[rs2];
        break;
      case sra_reg:
        registers[rd] = static_cast<int64_t>(registers[rs1]) >> registers[rs2];
        break;
        default:
        raise_exception(illegal_instr_csr_cause, instruction);
        continue;
        break;
      }
    }
    break;
    case str_op:
    {
      const byte_t imm1 = (instruction & 0x00000F80) >> 7;
      const byte_t funct3 = (instruction & 0x00007000) >> 12;
      const byte_t rs1 = (instruction & 0x000F8000) >> 15;
      const byte_t rs2 = (instruction & 0x1F00000) >> 20;
      const byte_t imm2 = (instruction & 0xFE000000) >> 25;
      const int64_t imm10 =
          int64_t((uint64_t(imm1) | uint64_t(imm2) << 5) << 52) >> 52;
      const uint64_t address = registers[rs1] + imm10;
      switch (stor_funct_e(funct3))
      {
      case sb_stor:
        memory_ptr->write_n_bytes<byte_t>(address, registers[rs2] & 0xFF);
        
        break;
      case sh_stor:
        if (address % 2 != 0)
        {
          raise_exception(store_misaligned_csr_cause, address);
          continue;
        }
        memory_ptr->write_n_bytes<uint16_t>(address, registers[rs2] & 0xFFFF);
        
        break;
      case sw_stor:
        if (address % 4 != 0)
        {
          raise_exception(store_misaligned_csr_cause, address);
          continue;
        }
        memory_ptr->write_n_bytes<uint32_t>(
            address, uint32_t(registers[rs2] & 0xFFFFFFFF));
        
        break;
      case sd_stor:
        if (address % 8 != 0)
        {
          raise_exception(store_misaligned_csr_cause, address);
          continue;
        }
        memory_ptr->write_doubleword(address, registers[rs2]);
        break;
        default:
        raise_exception(illegal_instr_csr_cause, instruction);
        continue;
        break;
      }
    }
    break;
    case lui_op:
    {
      const byte_t rd = (instruction & 0x00000F80) >> 7;
      const int64_t imm =
          int64_t((uint64_t(instruction) & 0xFFFFF000) << 32) >> 32;
      registers[rd] = imm;
    }
    break;
    case auipc_op:
    {
      const byte_t rd = (instruction & 0x00000F80) >> 7;
      const int64_t imm =
          int64_t(uint64_t(instruction & 0xFFFFF000) << 32) >> 32;
      registers[rd] = int64_t(pc) + imm;
    }
    break;
    case imm64_op:
    {
      const byte_t rd = (instruction & 0x00000F80) >> 7;
      const byte_t funct3 = (instruction & 0x00007000) >> 12;
      const byte_t rs1 = (instruction & 0x000F8000) >> 15;
      switch (imm64_funct_e(funct3))
      {
      case addiw_imm64:
      {
        const int64_t imm =
            static_cast<int64_t>(
                (static_cast<uint64_t>(instruction) & 0xFFF00000) << 32) >>
            52;
        registers[rd] =
            int64_t(uint64_t(int64_t(registers[rs1]) + imm) << 32) >> 32;
      }
      break;
      case slliw_imm64:
      case srliw_imm64:
      {
        const int32_t imm = static_cast<uint32_t>(instruction) << 20;
        const uint16_t funct = static_cast<uint16_t>(instruction >> 25) |
                               static_cast<uint16_t>(funct3) << 7;
        const byte_t shamt = (instruction & 0x1F00000) >> 20;
        switch (imm64_funct_e(funct))
        {
        case slliw10_imm64:
          registers[rd] = (static_cast<int64_t>(
                               static_cast<uint32_t>(registers[rs1]) << shamt)
                           << 32) >>
                          32;
          break;
        case srliw10_imm64:
          registers[rd] = (static_cast<int64_t>(
                               static_cast<uint32_t>(registers[rs1]) >> shamt)
                           << 32) >>
                          32;
          break;
        case sraiw10_imm64:
          registers[rd] = static_cast<int64_t>(
                              (static_cast<int32_t>(registers[rs1]) >> shamt))
                              << 32 >>
                          32;
          break;
        default:
          raise_exception(illegal_instr_csr_cause, instruction);
          continue;
          break;
        }
      }
      break;
      }
    }
    break;
    case jal_op:
    {
      const byte_t rd = (instruction & 0x00000F80) >> 7;
      const int64_t imm =
          static_cast<int64_t>(
              (static_cast<uint64_t>(instruction) & 0x80000000 |
               static_cast<uint64_t>(instruction) & 0x3FF000 << 9 |
               static_cast<uint64_t>(instruction) & 0x400000 >> 2 |
               static_cast<uint64_t>(instruction) & 0x7F800000 >> 11)
              << 32) >>
          52;
      registers[rd] = pc + 4;
      pc = pc + imm - 4;
      // Test for misaligned pc after adding rs1 to imm, do I need to print
      // an error here?
      pc = (pc >> 1) << 1;
    }
    break;
    case jalr_op:
    {
      const byte_t rd = (instruction & 0x00000F80) >> 7;
      const byte_t rs1 = (instruction & 0x000F8000) >> 15;
      const int64_t imm =
          static_cast<int64_t>((static_cast<uint64_t>(instruction) << 32)) >>
          52;
      const uint64_t link_address = pc + 4;
      pc = static_cast<int64_t>(registers[rs1]) + imm - 4;
      registers[rd] = link_address;
      // Test for misaligned pc after adding rs1 to imm, do I need to print
      // an error here?
      pc = (pc >> 1) << 1;
    }
    break;
    case reg64_op:
    {
      const byte_t rd = (instruction & 0x00000F80) >> 7;
      const byte_t rs1 = (instruction & 0x000F8000) >> 15;
      const byte_t rs2 = (instruction & 0x1F00000) >> 20;
      const uint16_t funct =
          static_cast<uint16_t>(instruction >> 25) |
          static_cast<uint16_t>((instruction & 0x00007000) >> 12) << 7;
      switch (reg64_funct_e(funct))
      {
      case addw_reg64:
        registers[rd] = (static_cast<int64_t>(registers[rs1]) +
                             static_cast<int64_t>(registers[rs2])
                         << 32) >>
                        32;
        break;
      case subw_reg64:
        registers[rd] = (static_cast<int64_t>(registers[rs1]) -
                             static_cast<int64_t>(registers[rs2])
                         << 32) >>
                        32;
        break;
      case sllw_reg64:
        registers[rd] =
            (static_cast<int64_t>(static_cast<uint32_t>(registers[rs1])
                                  << registers[rs2])
             << 32) >>
            32;
        break;
      case srlw_reg64:
        registers[rd] =
            (static_cast<int64_t>(static_cast<uint32_t>(registers[rs1]) >>
                                  registers[rs2])
             << 32) >>
            32;
        break;
      case sraw_reg64:
        registers[rd] =
            (static_cast<int64_t>(static_cast<int32_t>(registers[rs1]) >>
                                  registers[rs2])
             << 32) >>
            32;
        break;
        default:
        raise_exception(illegal_instr_csr_cause, instruction);
        continue;
        break;
      }
    }
    break;
    case sys_op:
    {
      const uint16_t funct7 = instruction >> 20;
      const byte_t rd = (instruction & 0x00000F80) >> 7;
      const byte_t funct3 = (instruction & 0x00007000) >> 12;
      const byte_t rs1 = (instruction & 0x000F8000) >> 15;
      const uint32_t csr_num = instruction >> 20;
      
      const auto has_privelege = [&]() {
        if (prv == 0)
        {
          raise_exception(illegal_instr_csr_cause);
          return false;
        }
        else if (&csr[csr_num] == &csr.invalid_reg)
        {
          raise_exception(illegal_instr_csr_cause);
          return false;
        }
        else
        {
          switch (csr_reg_e(csr_num))
          {
          case mvendorid_csr_reg:
          case marchid_csr_reg:
          case mimpid_csr_reg:
          case mhartid_csr_reg:
            if (rs1)
            {
              raise_exception(illegal_instr_csr_cause);
              return false;
            }
            break;
          }
        }
        return true;
      };
      switch (sys_funct_e(funct3))
      {
      case 0:
        switch (sys_funct_e(funct7))
        {
        case ecall_sys:
          if (prv == 0)
            raise_exception(env_call_u_mode_csr_cause);
          else
            raise_exception(enc_call_m_mode_csr_cause);
          continue;
          break;
        case ebreak_sys:
        {
          raise_exception(breakpoint_csr_cause);
          continue;
        }
        break;
        case mret_sys:
        {
          if (prv == 0)
          {
            raise_exception(illegal_instr_csr_cause);
            continue;
          }
          const uint64_t mstatus = csr[mstatus_csr_reg];
          pc = csr[mepc_csr_reg] - 4;
          prv = (mstatus >> 11) % 4;
          const byte_t mpie = (mstatus >> 7) % 2;
          csr[mstatus_csr_reg] &= 0xFFFFFFFFFFFFE777;
          csr[mstatus_csr_reg] |=
              (0x200000080 | uint32_t(mpie) << 3);
          break;
        }
        break;
        }
        break;
      case csrrw_zicsr:
        if (!has_privelege())
          continue;
        if (prv == 0)
        {
          raise_exception(illegal_instr_csr_cause);
          continue;
        }
        registers[rd] = csr[csr_num];
        if (csr_num == mip_csr_reg)
        {
          csr[csr_num] = registers[rs1] & 0x111;
        }
        else
          csr[csr_num] = registers[rs1];
        break;
      case csrrs_zicsr:
        if (!has_privelege())
          continue;
        registers[rd] = csr[csr_num];
        if (rs1 != 0)
        {
          csr[csr_num] |= registers[rs1];
          if (csr_num == mip_csr_reg)
            csr[csr_num] &= 0x111;
        }
        break;
      case csrrc_zicsr:
        if (!has_privelege())
          continue;
        registers[rd] = csr[csr_num];
        if (rs1)
        {
          csr[csr_num] &= (~registers[rs1]);
          if (csr_num == mip_csr_reg)
            csr[csr_num] &= 0x111;
        }
        break;
      case csrrwi_zicsr:
        if (!has_privelege())
          continue;
        registers[rd] = csr[csr_num];
          csr[csr_num] = rs1; // rs1 is encoded as immediate value in this op
          if (csr_num == mip_csr_reg)
            csr[csr_num] &= 0x111;
          break;
      case csrrsi_zicsr:
        if (!has_privelege())
          continue;
        registers[rd] = csr[csr_num];
        if (rs1)
        {
          csr[csr_num] |= rs1; // rs1 is encoded as immediate value in this op
          if (csr_num == mip_csr_reg)
            csr[csr_num] &= 0x111;
        }
        break;
      case csrrci_zicsr:
        if (!has_privelege())
          continue;
        registers[rd] = csr[csr_num];
        if (rs1)
        {
          csr[csr_num] &= (~rs1); // rs1 is encoded as immediate value in this op
          if (csr_num == mip_csr_reg)
            csr[csr_num] &= 0x111;
        }
        break;
        default:
        raise_exception(illegal_instr_csr_cause, instruction);
        continue;
        break;
      }
      }
      break;
      default:
        raise_exception(illegal_instr_csr_cause, instruction);
        continue;
        break;
    }
    instruction_ctr++;
    pc += 4;
  }
}

void processor::clear_breakpoint()
{
  break_point_enabled = false;
}

void processor::set_breakpoint(const uint64_t address)
{
  break_point = address;
  break_point_enabled = true;
}

void processor::show_prv() const
{
  printf("%u ", prv);
  switch (prv)
  {
  case 0:
    printf("(user)\n");
    break;
  case 3:
    printf("(machine)\n");
    break;
  }
}

void processor::set_prv(const uint32_t num) { prv = num; }

void processor::show_csr(const uint32_t csr_num)
{
  uint64_t& csr_value = csr[csr_num];
  if (&csr_value == &csr.invalid_reg)
    printf("Illegal CSR number\n");
  else
  {
    printf("%016llx\n", csr[csr_num]);
  }
}

void processor::set_csr(const uint32_t csr_num, const uint64_t new_value)
{
  uint64_t& csr_value = csr[csr_num];
  if (&csr_value == &csr.invalid_reg)
    printf("Illegal CSR number\n");
  else
  {
    switch (csr_reg_e(csr_num))
    {
    case mvendorid_csr_reg:
    case marchid_csr_reg:
    case mimpid_csr_reg:
    case mhartid_csr_reg:
      printf("Illegal write to read-only CSR\n");
      break;
    default:
      csr_value = new_value;
      break;
    }
  }
}

uint64_t processor::get_instruction_count() { return instruction_ctr; }

uint64_t processor::get_cycle_count()
{
  // :NOT YET IMPLEMENTED:
}