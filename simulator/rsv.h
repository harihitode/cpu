#include "sim.h"
#include <stdint.h>

#ifndef NATSUIRO_RSV_H
#define NATSUIRO_RSV_H

#define MAXSTSIZE 30

class station
{
  uint8_t rob[MAXSTSIZE];
  uint8_t q1[MAXSTSIZE];
  uint32_t st1[MAXSTSIZE];
  uint8_t q2[MAXSTSIZE];
  uint32_t st2[MAXSTSIZE];
  uint8_t fnct[MAXSTSIZE];
 protected:
  uint8_t size;    
  uint8_t ready_no;
  uint8_t op[MAXSTSIZE];  
 public:
  station(uint8_t s);
  void ready_st_no();
  uint32_t read1();
  uint32_t read2();
  uint8_t readop();
  uint8_t readrob();
  uint8_t readfnct();
  uint8_t full();
  void reserve(sourceT s);
  void update();
};

#endif
