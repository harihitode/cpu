#include <stdint.h>
#include <stdio.h>
#include "rob.h"
#include "sim.h"

extern cdbT cdb;

reorderBuffer::reorderBuffer(){
  int i;
  for(i=0;i<ROBSIZE;i++)
    buff[i] = 0;
  for(i=0;i<ROBSIZE;i++)
    dest[i] = 0;
  for(i=0;i<ROBSIZE;i++)
    cond[i] = 0;
  for(i=0;i<ROBSIZE;i++)
    mode[i] = 0;
  for(i=0;i<ROBSIZE;i++)
    phtaddr[i] = 0;  
  head = 1;
  tail = 1;
}

uint8_t reorderBuffer::number(){
  return tail;
}

cmitT reorderBuffer::commit(){
  cmitT ret;
  ret.dest = dest[head];
  ret.data = buff[head];
  ret.prpc = phtaddr[head];
  ret.rob = head;
  if(cond[head] == 0x0011){
    ret.commit_no = mode[head];
    mode[head] = 0;
    dest[head] = 0;
    cond[head] = 0;
    buff[head] = 0;    
    if(head == ROBSIZE - 1)
      head = 1;
    else
      head++;
  }else
    ret.commit_no = CMT_NUL;
  return ret;
}

void reorderBuffer::flush(){
  for(uint8_t i=0;i<ROBSIZE;i++){
    if(mode[i] == CMT_STR||mode[i] == CMT_FST)
      cond[i] = 0x011;
    mode[i] = CMT_NUL;
  }
}

uint8_t reorderBuffer::full(){
  if(head == tail)
    return cond[head];
  else
    return 0;
}

cdbT reorderBuffer::read(uint8_t robNo){
  cdbT ret;
  ret.i = buff[robNo];
  if((cond[robNo] & 0x010))
    ret.rob = 0;
  else
    ret.rob = robNo;
  return ret;
}

void reorderBuffer::reserve(instT inst){
  uint8_t op;
  op = inst.i >> 26;
  if(op == I_Storer||
     op == I_Store||
     op == I_Storeb){
    mode[tail] = CMT_STR;
    cond[tail] = 0x001;
    dest[tail] = (inst.i & 0x03e00000) >> 21;
  }else if(op == I_Storerf||
	   op == I_Storef||
	   op == I_Storebf){
    mode[tail] = CMT_FST;
    cond[tail] = 0x001;
    dest[tail] = (inst.i & 0x03e00000) >> 21;    
  }else if(op == I_Jmpr||
	   op == I_Ble||
	   op == I_Blei||
	   op == I_Blt||
	   op == I_Bltf||
	   op == I_Blti||
	   op == I_Beq||
	   op == I_Beqf||
	   op == I_Beqi){
    mode[tail] = CMT_BRA;
    cond[tail] = 0x001;
    phtaddr[tail] = inst.prpc;
  }else if(op == I_Add||
	   op == I_Addi||
	   op == I_Sub||
	   op == I_Subi||
	   op == I_Sl||
	   op == I_Lsr||
	   op == I_Asr||
	   op == I_Sli||
	   op == I_Save||
	   op == I_Ftoi||
	   op == I_Seti1||
	   op == I_Seti2||
	   op == I_Input||
	   op == I_Loadr||
	   op == I_Load||
	   op == I_Loadb){
    mode[tail] = CMT_GRG;
    cond[tail] = 0x001;
    dest[tail] = (inst.i & 0x03e00000) >> 21;
  }else if(op == I_Loadrf||
	   op == I_Loadf||
	   op == I_Inputf||
	   op == I_Setf1||
	   op == I_Setf2||
	   op == I_Addf||
	   op == I_Subf||
	   op == I_Mulf||
	   op == I_Invf||
	   op == I_Sqrtf||
	   op == I_Fneg||
	   op == I_Fabs||
	   op == I_Floor||
	   op == I_Itof||
	   op == I_Loadbf){
    mode[tail] = CMT_FRG;
    cond[tail] = 0x001;
    dest[tail] = (inst.i & 0x03e00000) >> 21;        
  }else{
    mode[tail] = CMT_NUL;
    tail--;
  }
  tail++;
  if(tail == ROBSIZE)
    tail = 1;
  return;
}

void reorderBuffer::writeBack(cdbT address){
  if(address.rob){
    buff[address.rob] = address.i;  
    cond[address.rob] |= 0x0010;
  }
  buff[cdb.rob] = cdb.i;
  cond[cdb.rob] = cond[cdb.rob] | 0x0010;  
  return;
}

uint8_t reorderBuffer::bra(){
  for(uint8_t i=0;i<ROBSIZE;i++){
    if(mode[i] == CMT_BRA)
      return 1;
  }
  return 0;
}

void reorderBuffer::dump(){
  uint8_t i;
  printf("ROB DUMP---head:%d tail:%d\n",head,tail);  
  for(i=0;i<ROBSIZE;i++)
    printf("[%2d] data:%8x dest:%d cond:%x mode:%d\n",i,buff[i],dest[i],cond[i],mode[i]);
  return;
}
