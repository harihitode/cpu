#ifndef NATSUIRO_BRA_H
#define NATSUIRO_BRA_H
#include <stdint.h>
#include "rsv.h"
#include "sim.h"
class branch_manage : public station
{
 public:
  branch_manage(uint8_t size);
  uint32_t yosoku[MAXSTSIZE];  
  void reserve(sourceT s,uint32_t pre);
  cdbT cdb_out();
  uint32_t readyosoku();  
};

#endif

