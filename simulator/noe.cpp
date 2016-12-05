#include <stdio.h>
#include <stdint.h>
#include "sim.h"
#include "rsv.h"
#include "noe.h"

integer_calc::integer_calc(uint8_t size) : station(size){}

cdbT integer_calc::cdb_out(){
  cdbT ret;
  uint32_t o1,o2;
  uint8_t op;
  fi mask;
  station::ready_st_no();
  op = readop();
  ret.rob = readrob();
  o1 = read1();
  o2 = read2();
  mask.i = o1;

  if(op == I_Add||op == I_Addi)
    ret.i = o1 + o2;
  else if(op == I_Sub||op == I_Subi)
    ret.i = o1 - o2;
  else if(op == I_Sl||op == I_Sli)
    ret.i = o1 << o2;
  else if(op == I_Asr)
    ret.i = (int)o1 >> o2;
  else if(op == I_Lsr)
    ret.i = o1 >> o2;
  else if(op == I_Save)
    ret.i = o1 + 2;
  else if(op == I_Seti1)
    ret.i = (o2 << 16) | (o1 & 0x0000ffff);
  else if(op == I_Seti2)
    ret.i = o1;
  else if(op == I_Itof)
    ret.i = (int)mask.f;
  else
    ret.i = 0;

  return ret;
}

