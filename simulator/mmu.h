#ifndef NATSUIRO_MMU_H
#define NATSUIRO_MMU_H
#include <stdint.h>
#include "rsv.h"
#include "sim.h"
#include "rob.h"
#define SRAMSIZE 1048576
const uint8_t MMUSTSIZE = 31;

class memory_manage// : public station
{
  uint32_t *sram;
  cdbT lut1,lut2,lut3,lut4;
  uint8_t ldsize,size;
  cdbT loadbuf[ROBSIZE];
  uint8_t stwait[ROBSIZE];
  cdbT stlist[ROBSIZE];

  uint8_t opcode[MMUSTSIZE];
  uint8_t robno[MMUSTSIZE];
  uint8_t q1[MMUSTSIZE];
  uint8_t q2[MMUSTSIZE];
  uint32_t st1[MMUSTSIZE];
  uint32_t st2[MMUSTSIZE];
  uint8_t busy[MMUSTSIZE];
  
  uint8_t head;
  uint8_t tail;
  
 public:
  memory_manage(uint8_t s);
  cdbT cdb_out();
  cdbT address_out();
  void store_call(uint32_t addr,uint32_t data,uint8_t rob);
  void load_call();
  void flush();

  void reserve(sourceT s);
  void update();
  uint8_t full();  
  ~memory_manage();
};
#endif
