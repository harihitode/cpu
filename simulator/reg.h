#ifndef NATSUIRO_REG_H
#define NATSUIRO_REG_H
#include <stdint.h>
class registerFile
{
  uint32_t file[32];
  uint8_t query[32];
 public:
  registerFile();
  void reserve(uint8_t dest, uint8_t robNo);
  cdbT read(uint8_t n);
  void flush();
  void writeBack(uint32_t data, uint8_t dest, uint8_t robNo);
  void dump();
};
#endif
