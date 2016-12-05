#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "sim.h"
#include "laplace.h"

extern int debug_mark;
extern simerror_t sim_error;
extern uint8_t step_mark;
extern uint32_t branch_count;

laplace::laplace(){
  pc = 3;
  lut1.i = 0; lut1.pc = 0;
  lut1.miss = 0; lut1.pr = 0;
  lut2.i = 0; lut2.pc = 0;
  lut2.miss = 0; lut2.pr = 0;  
  pht = (uint8_t *)calloc(sizeof(uint8_t),GROBSIZE);
  for(uint32_t i=0;i<65536;i++)
    pht[i] = 0x01;
  grrc = 0;
}

laplace::~laplace(){
  free(pht);
}

int
laplace::set_program(char *file_name){
  FILE *fp;
  int i = 0;
  char c;  
  if(!(fp = fopen(file_name,"r")))
    return -1;
  while((c = fread(&(iROM[i]),4,1,fp))){
    iROM[i] = ntohl(iROM[i]);
    i++;
    if(i > IROMSIZE){
      sim_error = ER_TOO_LARGE_INSTRUCTION;
      break;
    }
  }  
  fclose(fp);
  return 0;
}


void laplace::phtupdate(uint32_t prpc,uint8_t cmtbr){
  uint8_t t;
  t = pht[prpc];
  if(cmtbr){
    switch(t){
    case 0x00:
      t = 0x01;
      break;
    case 0x01:
      t = 0x02;
      break;
    default:
      t = 0x03;
      break;
    }
  }else{
    switch(t){
    case 0x03:
      t = 0x02;
      break;
    case 0x02:
      t = 0x01;
      break;
    default:
      t = 0x00;
      break;
    }
  }
  pht[prpc] = t;
  return;
}

uint8_t laplace::gshare(uint32_t prpc){
  if(pht[prpc] < 2)
    return 0;
  else
    return 1;
}

instT laplace::update(int hlt,cmitT msg){
  instT ret;
  uint8_t miss,op,braflag,taken,cmtpr,cmtbr;
  uint32_t predicate,code,prpc;
  
  code = iROM[pc];
  op = code >> 26;
  if(msg.commit_no == CMT_BRA){    
    cmtbr = (msg.data >> 30) & 0x00000001;
    cmtpr = (msg.data >> 31) & 0x00000001;    
    grrc = ((grrc << 1) | cmtbr) & 0x0000ffff;
    miss = cmtbr ^ cmtpr;
    if(miss)
      phtupdate(msg.prpc,cmtbr);    
    branch_count++;
  }else
    miss = 0;
  lut2.miss = 0;

  if(op == I_Ble||op == I_Blei||op == I_Blt||op == I_Bltf||
     op == I_Blti||op == I_Beq||op == I_Beqf||op == I_Beqi)
    braflag = 1;
  else
    braflag = 0;
  
  if(miss){
    lut2.miss = miss;    
    pc = msg.data & 0x003fffff;
    lut1.pc = 0; lut1.i = 0;
    lut2.pc = 0; lut2.i = 0;
  }else if(hlt == 0 && op == I_Jmp){
    lut2 = lut1;
    lut1.pc = pc;
    lut1.i = code;
    pc = code & 0x0003ffff;
  }else if(hlt == 0 && braflag == 1){
    prpc = (pc ^ grrc) & 0x0000ffff;
    
    lut2 = lut1;
    lut1.pc = pc;
    lut1.i  = code;
    lut1.prpc = prpc;
    taken = gshare(prpc);
    // if(taken)
    //   printf("%x,%d\n",prpc,taken);   
    if(taken){
      pc++;
      predicate = 0x80000000;
      lut1.pr = predicate | pc;
      pc = 0x00007fff & code;
    }else{     
      predicate = 0x00000000;
      lut1.pr = predicate | (0x00007fff & code);
      pc++;
    }
    if(pc > IROMSIZE)
      pc = 0;
  }else if(hlt == 0){
    lut2 = lut1;
    lut1.pc = pc;
    lut1.i  = code;
    predicate = 0x00000000;
    lut1.pr = predicate;
    pc++;
  }
  ret = lut2;  
  return ret;
}

