#include "sim.h"
#include <stdint.h>

#ifndef NATSUIRO_LAPLACE_H
#define NATSUIRO_LAPLACE_H
const long IROMSIZE = 30000;
const long GROBSIZE = 65536;

class laplace
{
  uint32_t iROM[IROMSIZE];
  uint8_t *pht;
  instT lut1,lut2;
  uint64_t pc;
  uint32_t grrc;
 public:
  laplace();
  instT update(int halt,cmitT brms);
  int set_program(char *file_name);
  uint8_t gshare(uint32_t prpc);
  void phtupdate(uint32_t prpc,uint8_t cmtbr);
  ~laplace();
};
  
#endif
