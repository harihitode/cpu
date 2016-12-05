#include <stdint.h>
#include <stdio.h>
#include "bra.h"
#include "rsv.h"
#include "sim.h"

extern uint8_t step_mark;

branch_manage::branch_manage(uint8_t size) : station(size) {}

void branch_manage::reserve(sourceT s,uint32_t pre){
  uint8_t i;
  for(i=0;i<size;i++){
    if(op[i] == 0){
      yosoku[i] = pre;
    }
  }
  station::reserve(s);
  return;
}

uint32_t branch_manage::readyosoku(){
  uint32_t ret;
  if(ready_no == 0xff)
    return 0;
  else{
    ret = yosoku[ready_no];
    yosoku[ready_no] = 0;
    return ret;
  }
}

cdbT branch_manage::cdb_out(){
  cdbT ret;
  fi o1,o2;
  uint8_t op,direction;
  uint32_t yosoku;
  station::ready_st_no();
  
  op = readop();
  ret.rob = readrob();
  o1.i = read1();
  o2.i = read2();
  yosoku = readyosoku();
    
  if(op == I_Ble||op == I_Blei){
    if(o1.s <= o2.s)
      direction = 1;
    else
      direction = 0;
  }else if(op == I_Blt||op == I_Blti){
    if(o1.s < o2.s)
      direction = 1;
    else
      direction = 0;
    
  }else if(op == I_Bltf){
    if(o1.f < o2.f)
      direction = 1;
    else
      direction = 0;
  }else if(op == I_Beq||op == I_Beqf||op == I_Beqi){
    if(o1.i == o2.i)
      direction = 1;
    else
      direction = 0;
  }else if(op == I_Jmpr){
    direction = 1;    
    yosoku = 0x003fffff & o1.i;
  }else
    direction = 0;
  ret.i = yosoku | (direction << 30);
  return ret;
}
