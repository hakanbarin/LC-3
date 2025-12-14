#ifndef VM_H
#define VM_H


#include <algorithm>
#include <array>
#include <bit>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

inline constexpr int MEMORY_MAX = 1 << 16;
inline constexpr uint32_t PC_START = 0x3000;

// little endian - big endian dönüşümü için yapılıyor.
// c++23 bitswap kullandığım için bunu kullanmadım.
constexpr uint16_t swap16(uint16_t x) {
  return static_cast<uint16_t>((x << 8) | (x >> 8));
}

// pozitif/negatif sayıların extend edilmesi amacıyla yazılıyor.
// basit bir fonksiyon her yerde kullanılabilir.
constexpr uint16_t sign_extend(uint16_t x, int bit_count) {
  if ((x >> (bit_count - 1)) & 1) {
    x |= static_cast<uint16_t>(0xFFFF << bit_count);
  }
  return x;
}

enum class Register : uint16_t {
  R0 = 0,
  R1,
  R2,
  R3,
  R4,
  R5,
  R6,
  R7,
  PC,
  COND,
  COUNT
};

enum class Trap : uint16_t {
  GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
  OUT = 0x21,   /* output a character */
  PUTS = 0x22,  /* output a word string */
  IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
  PUTSP = 0x24, /* output a byte string */
  HALT = 0x25   /* halt the program */
};

enum class Opcode : uint16_t {
  BR = 0, /* branch */
  ADD,    /* add  */
  LD,     /* load */
  ST,     /* store */
  JSR,    /* jump register */
  AND,    /* bitwise and */
  LDR,    /* load register */
  STR,    /* store register */
  RTI,    /* unused */
  NOT,    /* bitwise not */
  LDI,    /* load indirect */
  STI,    /* store indirect */
  JMP,    /* jump */
  RES,    /* reserved (unused) */
  LEA,    /* load effective address */
  TRAP    /* execute trap */
};

enum class ConditionFlag : uint16_t {
  POS = 1 << 0, /* P */
  ZRO = 1 << 1, /* Z */
  NEG = 1 << 2, /* N */
};

enum class MemoryMappedRegister : uint16_t {
  KBSR = 0xFE00, /* keyboard status */
  KBDR = 0xFE02  /* keyboard data */
};

// Helper to convert enum class to underlying type
template <typename E> 
constexpr auto to_underlying(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

class VirtualMachine {
public:

  [[nodiscard]] int run(int argc, const char *argv[]);


  [[nodiscard]] bool read_image(const std::filesystem::path &path);

  void mem_write(uint16_t address, uint16_t val);
  [[nodiscard]] uint16_t mem_read(uint16_t address);


  [[nodiscard]] bool check_key();

  void update_flags(uint16_t r);

private:
  std::array<uint16_t, MEMORY_MAX> memory{};
  std::array<uint16_t, to_underlying(Register::COUNT)> reg{};
  uint16_t instr = 0;
  uint16_t op = 0;
  bool running = true;

  void read_image_file(std::ifstream &file);


  void process_ADD(uint16_t instr);
  void process_AND(uint16_t instr);
  void process_NOT(uint16_t instr);
  void process_BR(uint16_t instr);
  void process_JMP(uint16_t instr);
  void process_JSR(uint16_t instr);
  void process_LD(uint16_t instr);
  void process_LDI(uint16_t instr);
  void process_LDR(uint16_t instr);
  void process_LEA(uint16_t instr);
  void process_ST(uint16_t instr);
  void process_STI(uint16_t instr);
  void process_STR(uint16_t instr);
  void process_TRAP(uint16_t instr);
};

#endif // VM_H