#ifndef NATSUIRO_NOE_H
#define NATSUIRO_NOE_H
#include "rsv.h"

class integer_calc : public station
{
 public:
  integer_calc(uint8_t size);
  cdbT cdb_out();
};

#endif

