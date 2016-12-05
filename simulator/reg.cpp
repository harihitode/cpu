#include "sim.h"
#include "reg.h"
#include <stdint.h>
#include <stdio.h>

extern uint8_t debug_mark;
extern uint8_t step_mark;

registerFile::registerFile(){
  int i;
  for(i=0;i<DATASIZE;i++)
    file[i] = 0;
  for(i=0;i<DATASIZE;i++)
    query[i] = 0;
}

cdbT registerFile::read(uint8_t n){
  cdbT ret;
  // if(file[28] == 0x0f5ea && file[29] == 0x00220)
  // if(file[9] != 0x00)
  //   step_mark++;
  // //  if(file[29] == 0x0)    
  ret.rob = query[n];
  ret.i   = file[n];
  return ret;
}

void registerFile::reserve(uint8_t dest, uint8_t robNo){
  query[dest] = robNo;
  return;
}

void registerFile::writeBack(uint32_t data, uint8_t dest, uint8_t robNo){
  file[dest] = data;
  if(query[dest] == robNo)
    query[dest] = 0;
  return;
}

void registerFile::flush(){
  for(uint8_t i=0;i<32;i++)
    query[i] = 0;
  return;
}
  
void registerFile::dump(){
  int i;
  printf("REGISTER DUMP\n");
  for(i=DATASIZE-1;i>=0;i--)
    printf("regno.%2d [%d] 0x%8x\n",i,query[i],file[i]);
}
