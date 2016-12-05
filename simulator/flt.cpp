#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "sim.h"
#include "rsv.h"
#include "flt.h"

extern uint8_t step_mark;

float_calc::float_calc(uint8_t size) : station(size){}

cdbT float_calc::cdb_out(){
  cdbT ret;
  fi o1,o2;
  uint8_t op,fnct;
  station::ready_st_no();
  op = readop();
  ret.rob = readrob();
  o1.i = read1();
  o2.i = read2();
  fnct = readfnct();

  if(op == I_Setf1)
    ret.i = (o2.i << 16) | (o1.i & 0x0000ffff);
  else if(op == I_Setf2)
    ret.i = (o1.i & 0xffff0000) | (o2.i & 0x0000ffff);
  else if(op == I_Ftoi)
    ret.i = (int)o1.f;
  else if(op == I_Fneg)
    ret.i = 0x80000000 ^ o1.i;
  else if(op == I_Fabs)
    ret.i = 0x7fffffff & o1.i;
  else if(op == I_Addf){
    o1.f = o1.f + o2.f;
    if(fnct)
      ret.i = 0x7fffffff & o1.i;
    else
      ret.i = o1.i;
  }else if(op == I_Subf){
    o1.f = o1.f - o2.f;
    if(fnct)
      ret.i = 0x7fffffff & o1.i;
    else
      ret.i = o1.i;    
  }else if(op == I_Mulf){
    o1.f = o1.f * o2.f;
    if(fnct)
      ret.i = 0x7fffffff & o1.i;
    else
      ret.i = o1.i;    
  }else if(op == I_Invf){
    o1.f = 1.0 / o1.f;
    if(fnct)
      ret.i = 0x7fffffff & o1.i;
    else
      ret.i = o1.i;    
  }else if(op == I_Sqrtf){    
    o1.f = sqrtf(o1.f);
    if(fnct)
      ret.i = 0x7fffffff & o1.i;
    else
      ret.i = o1.i;    
  }else
    ret.i = 0;
  o1.i = ret.i;
  // if(isnan(o1.f)){
  //   printf("invalid operation %f\n",o1.f);
  //   ret.i = 0;
  //   step_mark++;
  // }
  return ret;
}

