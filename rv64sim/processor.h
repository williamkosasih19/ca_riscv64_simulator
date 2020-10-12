#pragma once

/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2020

   Class for processor

**************************************************************** */

#include <vector>
#include <unordered_map>

#include "memory.h"

using namespace std;

class reg
{
  private:
    vector<uint64_t> registers;
  public:
    reg();
    uint64_t& operator[](byte_t reg_num);
};

class csr_reg
{
  private:
    unordered_map<uint32_t, uint64_t> reg_map;
  public:
    csr_reg();
    uint64_t& operator[](const uint32_t reg_num);
    uint64_t invalid_reg;
};

class processor {

 private:
   uint64_t pc;
   uint64_t instruction_ctr;
   reg registers;
   csr_reg csr;
   memory* memory_ptr;
   uint64_t break_point;
   bool_t break_point_enabled;
   uint32_t prv;
   uint32_t instruction;

   // TODO: Add private members here

 public:
  // Consructor
   processor(memory* const main_memory, const bool_t verbose,
             const bool_t stage2);

   // Display PC value
   void show_pc();

   // Set PC to new value
   void set_pc(const uint64_t new_pc);

   // Display register value
   void show_reg(const uint32_t reg_num);

   // Set register to new value
   void set_reg(const unsigned int reg_num, const uint64_t new_value);

   // Execute a number of instructions
   void execute(unsigned int num, bool breakpoint_check);

   // Clear breakpoint
   void clear_breakpoint();

   // Set breakpoint at an address
   void set_breakpoint(const uint64_t address);

   // Show privilege level
   // Empty implementation for stage 1, required for stage 2
   void show_prv() const;

   // Set privilege level
   // Empty implementation for stage 1, required for stage 2
   void set_prv(const unsigned int prv_num);

   // Display CSR value
   // Empty implementation for stage 1, required for stage 2
   void show_csr(const unsigned int csr_num);

   // Set CSR to new value
   // Empty implementation for stage 1, required for stage 2
   void set_csr(const unsigned int csr_num, const uint64_t new_value);

   uint64_t get_instruction_count();

   // Used for Postgraduate assignment. Undergraduate assignment can return 0.
   uint64_t get_cycle_count();

   void raise_exception(const uint16_t cause, uint64_t address = 0);
};
