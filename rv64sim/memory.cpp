/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2020

   Class members for memory

**************************************************************** */

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h>

#include "memory.h"
#include "outputImpl.h"
#include "processor.h"
#include "enum.h"

using namespace std;

// Constructor
memory::memory(bool_t verbose) {}

// Read a word of data from a word-aligned address.
// If the address is not a multiple of 4, it is rounded down to a multiple of 4.
uint32_t memory::fetch_pc(uint64_t address)
{
  address = (address >> 2) << 2;
  uint32_t mem_value;
  const uint64_t block_number = address / block_size;
  if (block_map.find(block_number) == block_map.end())
    mem_value = 0;
  else
    mem_value = *(uint32_t*)(block_map[block_number] + address % block_size);

  verbose_print("Fetch: pc = %016llx, ir = %08llx\n", address, mem_value);
  return mem_value;
}

// Read a doubleword of data from a doubleword-aligned address.
// If the address is not a multiple of 8, it is rounded down to a multiple of 8.
uint64_t memory::read_doubleword(uint64_t address)
{
  address = (address >> 3) << 3;
  uint64_t mem_value;
  const uint64_t block_number = address / block_size;
  if (block_map.find(block_number) == block_map.end())
    mem_value = 0;
  else
    mem_value = *(uint64_t*)(block_map[block_number] + address % block_size);

  verbose_print("Memory read doubleword: address = %016llx, data = %016llx\n",
                address, mem_value);
  return mem_value;
}

// Write a doubleword of data to a doubleword-aligned address.
// If the address is not a multiple of 8, it is rounded down to a multiple of 8.
// The mask contains 1s for bytes to be updated and 0s for bytes that are to
// be unchanged.
void memory::write_doubleword(uint64_t address, uint64_t write_data,
                              uint64_t mask)
{
  address = (address >> 3) << 3;
  verbose_print("Memory write doubleword: address = %016llx, data = %016llx"
                ", mask = %016llx\n",
                address, write_data, mask);
  const uint64_t block_number = address / block_size;
  if (block_map.find(block_number) == block_map.end())
  {
    char_t* const ptr = (block_map[block_number] = new char_t[block_size]);
    memset(ptr, 0, block_size);
  }
  uint64_t* const block_ptr =
      (uint64_t*)(block_map[block_number] + address % block_size);
  *block_ptr = (*block_ptr & ~mask) | (write_data & mask);
}

void memory::set_processor(processor* processor_ptr)
{
  this->processor_ptr = processor_ptr;
}

// Load a hex image file and provide the start address for execution from the
// file in start_address.
// Return true if the file was read without error, or false otherwise.
bool memory::load_file(string file_name, uint64_t& start_address)
{
  ifstream input_file(file_name);
  string input;
  unsigned int line_count = 0;
  unsigned int byte_count = 0;
  char record_start;
  char byte_string[3];
  char halfword_string[5];
  unsigned int record_length;
  unsigned int record_address;
  unsigned int record_type;
  unsigned int record_data;
  unsigned int record_checksum;
  bool end_of_file_record = false;
  uint64_t load_address;
  uint64_t load_data;
  uint64_t load_mask;
  uint64_t load_base_address = 0x0000000000000000ULL;
  start_address = 0x0000000000000000ULL;
  if (input_file.is_open())
  {
    while (true)
    {
      line_count++;
      input_file >> record_start;
      if (record_start != ':')
      {
        cout << "Input line " << dec << line_count
             << " does not start with colon character" << endl;
        return false;
      }
      input_file.get(byte_string, 3);
      sscanf(byte_string, "%x", &record_length);
      input_file.get(halfword_string, 5);
      sscanf(halfword_string, "%x", &record_address);
      input_file.get(byte_string, 3);
      sscanf(byte_string, "%x", &record_type);
      switch (record_type)
      {
      case 0x00: // Data record
        for (unsigned int i = 0; i < record_length; i++)
        {
          input_file.get(byte_string, 3);
          sscanf(byte_string, "%x", &record_data);
          load_address = (load_base_address | (uint64_t)(record_address)) + i;
          load_data = (uint64_t)(record_data) << ((load_address % 8) * 8);
          load_mask = 0x00000000000000ffULL << ((load_address % 8) * 8);
          write_doubleword(load_address & 0xfffffffffffffff8ULL, load_data,
                           load_mask);
          byte_count++;
        }
        break;
      case 0x01: // End of file
        end_of_file_record = true;
        break;
      case 0x02: // Extended seg address (set bits 19:4 of load base address)
        load_base_address = 0x0000000000000000ULL;
        for (unsigned int i = 0; i < record_length; i++)
        {
          input_file.get(byte_string, 3);
          sscanf(byte_string, "%x", &record_data);
          load_base_address = (load_base_address << 8) | (record_data << 4);
        }
        break;
      case 0x03: // Start segment address (ignored)
        for (unsigned int i = 0; i < record_length; i++)
        {
          input_file.get(byte_string, 3);
          sscanf(byte_string, "%x", &record_data);
        }
        break;
      case 0x04: // Extended linear addr (set upper halfword of load base addr)
        load_base_address = 0x0000000000000000ULL;
        for (unsigned int i = 0; i < record_length; i++)
        {
          input_file.get(byte_string, 3);
          sscanf(byte_string, "%x", &record_data);
          load_base_address = (load_base_address << 8) | (record_data << 16);
        }
        break;
      case 0x05: // Start linear address (set execution start address)
        start_address = 0x0000000000000000ULL;
        for (unsigned int i = 0; i < record_length; i++)
        {
          input_file.get(byte_string, 3);
          sscanf(byte_string, "%x", &record_data);
          start_address = (start_address << 8) | record_data;
        }
        break;
      }
      input_file.get(byte_string, 3);
      sscanf(byte_string, "%x", &record_checksum);
      input_file.ignore();
      if (end_of_file_record)
        break;
    }
    input_file.close();
    cout << dec << byte_count << " bytes loaded, start address = " << setw(16)
         << setfill('0') << hex << start_address << endl;
    return true;
  }
  else
  {
    cout << "Failed to open file" << endl;
    return false;
  }
}
