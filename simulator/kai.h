#ifndef NATSUIRO_KAI_H
#define NATSUIRO_KAI_H
#include "rsv.h"

class serial_port : public station
{
 public:
  serial_port();
  void set_input(char *);
  FILE *ifp;
  FILE *ofp;
  cdbT cdb_out();
  ~serial_port();  
};
#endif
