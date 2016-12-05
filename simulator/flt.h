#ifndef NATSUIRO_FLT_H
#define NATSUIRO_FLT_H
#include "rsv.h"

class float_calc : public station
{
 public:
  float_calc(uint8_t size);
  cdbT cdb_out();
};

#endif
