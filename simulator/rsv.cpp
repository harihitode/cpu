#include <stdio.h>
#include <stdint.h>
#include "rsv.h"

extern cdbT cdb;

station::station(uint8_t s){  
  int i;
  size = s;
  for(i=0;i<size;i++){
    q1[i] = 0;
    q2[i] = 0;
    op[i] = 0;
    st1[i] = 0;
    st2[i] = 0;
    rob[i] = 0;
    fnct[i] = 0;
  }
}

void station::update(){
  uint8_t i,cdbQ;
  cdbQ = cdb.rob;
  if(cdbQ != 0){
    for(i=0;i<size;i++){
      if(cdbQ == q1[i]){
	q1[i] = 0;
	st1[i] = cdb.i;
      }
      if(cdbQ == q2[i]){
	q2[i] = 0;
	st2[i] = cdb.i;
      }      
    }
  }      
  return;
}

void station::ready_st_no(){
  uint8_t i;
  for(i=0;i<size;i++){
    if(q1[i] == 0 && q2[i] == 0 && op[i] != 0){
      ready_no = i;
      return;
    }
  }
  ready_no = 0xff;
  return;
}

void station::reserve(sourceT s){
  uint8_t i;
  for(i=0;i<size;i++){
    if(op[i] == 0){
      q1[i] = (s.o1).rob;
      q2[i] = (s.o2).rob;
      st1[i] = (s.o1).i;
      st2[i] = (s.o2).i;
      rob[i] = s.rob;
      op[i] = s.opcode;
      fnct[i] = s.fnct;
      break;
    }
  }
  return;
}

uint8_t station::full(){
  int i;
  for(i=0;i<size;i++){
    if(op[i] == 0)
      return 0;
  }

  return 1;
}
  
uint32_t station::read1(){
  uint32_t ret;
  if(ready_no == 0xff)
    return 0;
  else{
    ret = st1[ready_no];
    st1[ready_no] = 0;
    return ret;
  }
}

uint32_t station::read2(){
  uint32_t ret;
  if(ready_no == 0xff)
    return 0;
  else{
    ret = st2[ready_no];
    st2[ready_no] = 0;
    return ret;
  }
}

uint8_t station::readop(){
  uint8_t ret;
  if(ready_no == 0xff)
    return 0;
  else{
    ret = op[ready_no];
    op[ready_no] = 0;
    return ret;
  }
  return 0;
}

uint8_t station::readrob(){
  uint8_t ret;
  if(ready_no == 0xff)
    return 0;
  else{
    ret = rob[ready_no];
    rob[ready_no] = 0;
    return ret;
  }
  return 0;
}

uint8_t station::readfnct(){
  uint8_t ret;
  if(ready_no == 0xff)
    return 0;
  else{
    ret = fnct[ready_no];
    fnct[ready_no] = 0;
    return ret;
  }
  return 0;
}

