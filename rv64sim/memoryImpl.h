/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2020

   Implementation for unspecialized template functions in the
   memory class.

**************************************************************** */
#include <cmath>
#include <cstring>

#include "memory.h"
#include "outputImpl.h"

template <typename T> T memory::read_n_bytes(uint64_t address)
{
  const uint16_t shift_amount = log2(sizeof(T));
  address = (address >> shift_amount) << shift_amount;
  T mem_value;
  const uint64_t block_number = address / block_size;
  if (block_map.find(block_number) == block_map.end())
    mem_value = 0;
  else
    mem_value = *(T*)(block_map[block_number] + address % block_size);
  verbose_print("Memory read %d bytes: address = %016llx, data = %08llx\n",
                sizeof(T), address, mem_value);
  return mem_value;
}

template <typename T> 
  void memory::write_n_bytes(uint64_t address, const T write_data)
{
  const uint16_t shift_amount = log2(sizeof(T));
  address = (address >> shift_amount) << shift_amount;
  verbose_print("Memory write %d bytes: address = %016llx, data = %016llx\n",
                sizeof(T), address, write_data);
  const uint64_t block_number = address / block_size;
  if (block_map.find(block_number) == block_map.end())
  {
    char_t* const ptr = (block_map[block_number] = new char_t[block_size]);
    memset(ptr, 0, block_size);
  }
  T* const block_ptr = (T*)(block_map[block_number] + address % block_size);
  *block_ptr = write_data;
}