#include "sim.h"
#include "mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern simerror_t sim_error;
extern uint8_t step_mark;
extern cdbT cdb;
extern uint64_t clk;

memory_manage::memory_manage(uint8_t s){
  uint8_t i;
  ldsize = s;
  size = 30;
  sram = (uint32_t *)calloc(sizeof(uint32_t),SRAMSIZE);
  lut4.i = 0; lut4.rob = 0;
  lut3.i = 0; lut3.rob = 0;
  lut2.i = 0; lut2.rob = 0;
  lut1.i = 0; lut1.rob = 0;
  head=0; tail=0;
  for(i=0;i<ROBSIZE;i++){
    stlist[i].rob = 0;
    stlist[i].i = 0;
  }
  for(i=0;i<size;i++){
    q1[i] = 0;
    q2[i] = 0;
    opcode[i] = 0;
    st1[i] = 0;
    st2[i] = 0;
    robno[i] = 0;
    busy[i] = 0;
  }  
}

memory_manage::~memory_manage(){
  free(sram);  
}

void memory_manage::update(){
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

void memory_manage::reserve(sourceT s){
  q1[tail] = (s.o1).rob;
  q2[tail] = (s.o2).rob;
  st1[tail] = (s.o1).i;
  st2[tail] = (s.o2).i;
  robno[tail] = s.rob;
  opcode[tail] = s.opcode;
  busy[tail] = 1;
  if(tail == size-1)
    tail=0;
  else
    tail++;
  return;
}

uint8_t memory_manage::full(){
  if(head == tail)
    return busy[head];
  else
    return 0;
}
  
void memory_manage::flush(){
  for(uint8_t i=0;i<ROBSIZE;i++){
    stwait[i] = 0;
    stlist[i].rob = 0;
  }
  for(uint8_t i=0;i<size;i++){
    if(opcode[i] == I_Storer||opcode[i] == I_Storeb||opcode[i] == I_Store||
       opcode[i] == I_Storerf||opcode[i] == I_Storebf||opcode[i] == I_Storef)
      opcode[i] = I_Nop;
    q1[i]=0;
    q2[i]=0;
  }    
  return;
}

cdbT memory_manage::cdb_out(){
  cdbT ret;
  ret = lut4;
  lut4 = lut3;
  lut3 = lut2;
  lut2 = lut1;
  return ret;
}

void memory_manage::load_call(){
  uint8_t i;
  for(i=0;i<ldsize;i++){
    if(stwait[i] == 0 && loadbuf[i].rob != 0){
      lut1.i = sram[loadbuf[i].i];
      lut1.rob = loadbuf[i].rob;
      loadbuf[i].i = 0;
      loadbuf[i].rob = 0;
      return;
    }
  }
  lut1.i = 0;
  lut1.rob = 0;
  return;
}

void memory_manage::store_call(uint32_t addr,uint32_t data,uint8_t rob){
  uint8_t i;
  for(i=0;i<ldsize;i++){
    if(stwait[i] == rob)
	stwait[i] = 0;
  }
  stlist[rob].rob = 0;
  stlist[rob].i = 0;
  sram[addr] = data;
  lut1.i = 0;
  lut1.rob  = 0;
  return;
}

cdbT memory_manage::address_out(){
  cdbT ret,z;
  uint32_t o1,o2;
  uint8_t op,i,wait,rob;
  z.i = 0; z.rob = 0;
  op = opcode[head];
  rob = robno[head];
  o1 = st1[head];
  o2 = st2[head];

  // if(step_mark){
  //   printf("head %d\n",head);
  //   for(uint8_t i=0;i<size;i++)
  //     printf("ld %d,%d,%d,%d,%d\n",i,robno[i],opcode[i],q1[i],q2[i]);
  //   for(uint8_t i=0;i<ldsize;i++)
  //     printf("wtlist %d %d %d\n",stwait[i],loadbuf[i].rob,loadbuf[i].i);    
  // }

  if(busy[head] == 1){
    if(q1[head] == 0 && q2[head] == 0){
      if(op == I_Storer||op == I_Loadr||op == I_Store||op == I_Load||
	 op == I_Storerf||op == I_Loadrf||op == I_Storef||op == I_Loadf)
	ret.i = o1 + o2;
      else
	ret.i = o1 - o2;
      ret.i &= 0x000fffff;
  
      for(i=1;i<ROBSIZE;i++){
	if(ret.i == stlist[i].i && stlist[i].rob != 0){
	  wait = i;
	  break;
	}    
	wait = 0;
      }
  
      if(op == I_Loadr||op == I_Loadb||op == I_Load||
	 op == I_Loadrf||op == I_Loadbf||op == I_Loadf){
	for(i=0;i<ldsize;i++){
	  if(loadbuf[i].rob == 0){
	    busy[head]=0;
	    if(head == size-1)
	      head = 0;
	    else
	      head++;
	    loadbuf[i].rob = rob;
	    loadbuf[i].i = ret.i;
	    stwait[i] = wait;
	    return z;
	  }
	}
	return z;
      }else if(op == I_Storer||op == I_Storeb||op == I_Store||
	       op == I_Storerf||op == I_Storebf||op == I_Storef){
	ret.rob = rob;
	stlist[rob].rob = 1;
	stlist[rob].i = ret.i;
      }
      busy[head]=0;
      if(head == size-1)
	head = 0;
      else
	head++;                  
    }else
      ret = z;
  }else
    ret = z;
  return ret;
}
