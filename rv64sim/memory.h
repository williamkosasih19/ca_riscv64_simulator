#pragma once
/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2020

   Class for memory

**************************************************************** */

#include <vector>
#include <unordered_map>

#include "types.h"

using namespace std;

class processor;

class memory {
 private:
  const uint32_t block_size = 4096;
  unordered_map<uint64_t, char_t*> block_map;
  processor* processor_ptr;
public:
  // Constructor
  memory(bool_t verbose);

  // Read a word of data from a word-aligned address.
  // If the address is not a multiple of 4, it is rounded down to a multiple
  // of 4.
  uint32_t fetch_pc(uint64_t address);

  template <typename T> T read_n_bytes(uint64_t address);

  // Read a doubleword of data from a doubleword-aligned address.
  // If the address is not a multiple of 8, it is rounded down to a multiple
  // of 8.
  uint64_t read_doubleword(uint64_t address);

  template <typename T> void write_n_bytes(uint64_t address, T write_data);

  // Write a doubleword of data to a doubleword-aligned address.
  // If the address is not a multiple of 8, it is rounded down to a multiple
  // of 8. The mask contains 1s for bytes to be updated and 0s for bytes that
  // are to be unchanged.
  void write_doubleword(uint64_t address, uint64_t write_data,
                        uint64_t mask = 0xFFFFFFFFFFFFFFFF);

  void set_processor(processor* processor_ptr);

  // Load a hex image file and provide the start address for execution from the
  // file in start_address. Return true if the file was read without error, or
  // false otherwise.
  bool_t load_file(string file_name, uint64_t& start_address);
};