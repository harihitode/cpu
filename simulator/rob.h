#ifndef NATSUIRO_ROB_H
#define NATSUIRO_ROB_H
#include <stdint.h>
#include "sim.h"
const uint8_t ROBSIZE = 16;

class reorderBuffer
{
  uint32_t buff[ROBSIZE];
  uint32_t dest[ROBSIZE];
  uint8_t cond[ROBSIZE];
  uint8_t mode[ROBSIZE];
  uint8_t phtaddr[ROBSIZE];    
  uint8_t head;
  uint8_t tail;
 public:
  reorderBuffer();
  uint8_t number();
  cmitT commit();
  cdbT read(uint8_t robNo);
  void writeBack(cdbT address);
  void reserve(instT inst);
  void dump();
  void flush();
  uint8_t full();
  uint8_t bra();
};
#endif
