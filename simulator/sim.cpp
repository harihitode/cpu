#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "sim.h"
#include "laplace.h"
#include "reg.h"
#include "rob.h"
#include "noe.h"
#include "bra.h"
#include "kai.h"
#include "flt.h"
#include "mmu.h"

cdbT cdb;
uint64_t clk;
registerFile greg,freg;
reorderBuffer rob;  

uint8_t debug_mark=0;
simerror_t sim_error=ER_NONE;
uint8_t step_mark=0;
uint32_t output_count=0;
uint32_t branch_count=0;

#define SAMPLE 100

void
error_message(simerror_t code){
  switch(code){
  case ER_INVALID_INPUT:
    printf("could not read input file\n");
    break;
  case ER_NO_INPUT:
    printf("need input file\n");
    break;
  case ER_NO_INSTRUCTION:
    printf("need program file\n");
    break;
  case ER_TOO_LARGE_INSTRUCTION:
    printf("this program file is too large\n");
    break;
  case ER_MEMORY_ACCESS:
    printf("invalid memory access\n");
    break;
  case ER_SUCCESS:
    break;
  default:
    printf("CODE[%d] Oops! sorry, unknown error\n",code);
    break;
  }
  return;
}

inline sourceT
source_maker(uint8_t arg1,uint8_t arg2,uint8_t arg3,uint32_t c,uint8_t op){
  sourceT ret;
  cdbT q1,q2;
  q1.rob = 0; q2.rob = 0;
  if(op == I_Add||op == I_Sub||op == I_Sl||op == I_Lsr||op == I_Asr||
     op == I_Storer||op == I_Storerf||op == I_Loadr||op == I_Loadrf){
    q1 = greg.read(arg2);
    q2 = greg.read(arg3);
  }else if(op == I_Ble||op == I_Blt||op == I_Beq){
    q1 = greg.read(arg1);
    q2 = greg.read(arg2);
  }else if(op == I_Beqf||op == I_Bltf){
    q1 = freg.read(arg1);
    q2 = freg.read(arg2);
  }else if(op == I_Addf||op == I_Subf||op == I_Mulf||op == I_Sqrtf||op == I_Invf){
    q1 = freg.read(arg2);
    q2 = freg.read(arg3);    
  }else if(op == I_Addi||op == I_Subi||op == I_Sli||
	   op == I_Store||op == I_Storeb||op == I_Storef||op == I_Storebf||
	   op == I_Load||op == I_Loadb||op == I_Loadf||op == I_Loadbf){
    q1 = greg.read(arg2);
    q2.i = c;
  }else if(op == I_Save||op == I_Seti2){
    q1.i = c;
  }else if(op == I_Seti1||op == I_Jmpr||op == I_Blei||op == I_Blti||
	   op == I_Beqi||op == I_Output||op == I_Itof){
    q1 = greg.read(arg1);
    q2.i = c;
  }else if(op == I_Setf1||op == I_Setf2){
    q1 = freg.read(arg1);
    q2.i = c;
  }else if(op == I_Ftoi||op == I_Floor||op == I_Fneg||op == I_Fabs){
    q1 = freg.read(arg2);
  }

  if(q1.rob == 0)
    ret.o1 = q1;
  else if(q1.rob == cdb.rob){
    ret.o1 = cdb;
    ret.o1.rob = 0;
  }else
    ret.o1 = rob.read(q1.rob);
  
  if(q2.rob == 0)
    ret.o2 = q2;
  else if(q2.rob == cdb.rob){
    ret.o2 = cdb;
    ret.o2.rob = 0;
  }else
    ret.o2 = rob.read(q2.rob);  
  return ret;
}

int
main(int argc, char *argv[]){
  laplace lap;
  integer_calc noe(10);
  float_calc flt(15);
  branch_manage bra(15);
  memory_manage mmu(10);
  serial_port kai;
  instT inst;
  cmitT commit_msg;
  char op_sym[10],opc;//,sc;
  sourceT s;
  uint8_t op,unitNo,hlt,arg1,arg2,arg3,robNo,misstake,limitmark=0,donemark=0;
  cdbT address;
  uint32_t gc;
  int limit=0,hltcheck=0;
  uint64_t done=0,donelimit;
  time_t start,end;
  FILE *resfp = fopen("result","w");
  //FILE *hltfp = fopen("haltgraph","w");
  uint32_t hltc = 0,misstakec=0,hlti[64];
  
  for(uint8_t i=0;i<64;i++)
    hlti[i]=0;
  start = time(NULL);
  
  while((opc = getopt(argc,argv,"n:s:d:"))!=-1){
    switch(opc){
    case 'n':
      limitmark++;
      limit = atoi(optarg);
      printf("limit marked\n");
      break;
    case 'd':
      donemark++;
      donelimit = atoi(optarg);
      break;      
    case 's':
      kai.set_input(optarg);
      break;
    default:
      printf("error: invalid option!\n");
      return -1;
    }
  }
  
  if(optind >= argc){
    printf("need program file\n");
    return -1;
  }

  if(lap.set_program(argv[optind]) < 0){
    printf("failed to read program file\n");
    return -1;
  }
  
  commit_msg.commit_no = CMT_NUL;
  commit_msg.dest = 0;
  commit_msg.data = 0;

  address.rob = 0; address.i = 0;

  for(clk=0;;clk++){
    if(sim_error != ER_NONE){
      error_message(sim_error);
      break;
    }
    // if(clk > 1187900)
    //   step_mark++;
    if(hltcheck > 100){
      printf("this program has gone to infinite loop");
      break;
    }
    // if(step_mark){
    //   if((sc = getchar())<0){
    // 	printf("done\n");
    // 	break;	
    //   }
    //   if(sc == 'd'){
    // 	rob.dump();
    // 	greg.dump();
    // 	freg.dump();
    //   }else if(sc == 'c')
    // 	step_mark = 0;
    // }    
    inst = lap.update(hlt,commit_msg);
    misstake = inst.miss;
    op = inst.i >> 26;
    arg1 = (inst.i&0x03e00000)>>21;
    arg2 = (inst.i&0x001f0000)>>16;
    arg3 = (inst.i&0x0000f800)>>11;    
    
    robNo = rob.number();
    unitNo = unit_number(op);

    //printf("%x,%u,%d\n",inst.i,inst.pc,unitNo);
    
    if(unitNo == U_MMU)
      hlt = mmu.full();
    else if(unitNo == U_KAI){
      if(rob.bra())
	hlt = 1;
      else
	hlt = kai.full();
    }else if(unitNo == U_NOE)
      hlt = noe.full();
    else if(unitNo == U_FLT)
      hlt = flt.full();
    else if(unitNo == U_BRA)
      hlt = bra.full();    
    else
      hlt = 0;

    hlt |= rob.full();
    hlt |= misstake;

    if(op == I_Nop || hlt == 1)
      done++;
    if(hlt){
      hlti[op]++;
      hltc++;
    }
    if(hlt == 1)
      hltcheck++;
    else
      hltcheck=0;
    //cdb update//
    cdb = mmu.cdb_out();
    if(cdb.rob == 0){
      cdb = kai.cdb_out();        
      if(cdb.rob == 0){
	cdb = noe.cdb_out();      
	if(cdb.rob == 0){
	  cdb = flt.cdb_out();
	  if(cdb.rob == 0)
	    cdb = bra.cdb_out();
	}
      }
    }
    // opcode_symbol(op_sym,op,9);                    
    // printf("%llu,%u,%s\n",clk,inst.pc,op_sym);                  
    //cdb update//
    mmu.update();
    kai.update();
    noe.update();
    flt.update();    
    bra.update();
    address = mmu.address_out();

    if(misstake){
      misstakec++;
      rob.flush();
      mmu.flush();
      greg.flush();
      freg.flush();
    }        
    if(op == I_Save)
      gc = inst.pc;
    else if(op == I_Blei||op == I_Blti||op == I_Beqi)
      gc = (inst.i & 0x001f8000)>>15;
    else
      gc = inst.i & 0x0000ffff;
    s = source_maker(arg1,arg2,arg3,gc,op);
    s.fnct = inst.i & 0x00000001;
    s.rob = robNo;
    s.opcode = op;    
    if(hlt == 0){
      switch(unitNo){
      case U_MMU:
      	mmu.reserve(s);
      	break;
      case U_KAI:
      	kai.reserve(s);
      	break;
      case U_NOE:      
    	noe.reserve(s);
    	break;
      case U_FLT:
	flt.reserve(s);
	break;
      case U_BRA:
      	bra.reserve(s,inst.pr);
      	break;
      default:
    	break;
      }
      rob.reserve(inst);
      if(which_reg(op) == GREG)
	greg.reserve(arg1,robNo);
      else if(which_reg(op) == FREG)
	freg.reserve(arg1,robNo);
    }
        
    commit_msg = rob.commit();    
    if(commit_msg.commit_no == CMT_STR)
      mmu.store_call(commit_msg.data,(greg.read(commit_msg.dest)).i,commit_msg.rob);
    else if(commit_msg.commit_no == CMT_FST)
      mmu.store_call(commit_msg.data,(freg.read(commit_msg.dest)).i,commit_msg.rob);
    else if(commit_msg.commit_no == CMT_GRG){
      greg.writeBack(commit_msg.data,commit_msg.dest,commit_msg.rob);
      mmu.load_call();
    }else if(commit_msg.commit_no == CMT_FRG){
      freg.writeBack(commit_msg.data,commit_msg.dest,commit_msg.rob);
      mmu.load_call();
    }else
      mmu.load_call();
    
    rob.writeBack(address);
    
    // if(!(clk%SAMPLE)){
    //   fprintf(hltfp,"%llu %u %u\n",clk,inst.pc,hltc);
    //   hltc = 0;
    // }    
    // if(step_mark){
    //   opcode_symbol(op_sym,op,9);                      
    //   printf("done:%llu:(pc:%d)halt[%d]  NOW_CDB[%x] -> [%d]\n",done,inst.pc,hlt,cdb.i,cdb.rob);
    //   printf("clk:%llu:%2x|%0.8x[%7s(%d,%d,%d)]->[%d],<rob.%d>NOW_COMMIT(%d)[%0.8x] -> [reg.%d]\n",clk,inst.pc,inst.i,op_sym,arg1,arg2,arg3,robNo,commit_msg.rob,commit_msg.commit_no,commit_msg.data,commit_msg.dest);
    // }
    // if(debug_mark){
    // 	printf("////////////////\n");
    // 	printf("   DEBUG STOP \n");
    // 	printf("////////////////\n");	
    //   break;
    // }
  }
  for(uint8_t i=0;i<64;i++){
    opcode_symbol(op_sym,i,9);                
    fprintf(resfp,"%d %d %7s\n",i,hlti[i],op_sym);
  }
  fprintf(resfp,"branch -> %d, misstake -> %d\n",branch_count,misstakec);  
  if(sim_error == ER_NONE || sim_error == ER_MEMORY_ACCESS){
    rob.dump();
    greg.dump();
    freg.dump();
  }else if(sim_error == ER_SUCCESS)
    printf("success!\n");
  end = time(NULL);
  printf("clk=%llu,hlt=%llu[%f] time:%ld[s]\n",clk,done,(done*100.0)/clk,end-start);
  fprintf(resfp,"clk=%llu,hlt=%llu[%f] time:%ld[s]\n",clk,done,(done*100.0)/clk,end-start);
  fclose(resfp);
  //fclose(hltfp);
  return 0;
}

inline uint8_t
which_reg(uint8_t op){
  uint8_t ret;
  switch(op){
  case I_Add:
  case I_Addi:    
  case I_Sub:
  case I_Subi:
  case I_Loadr:
  case I_Sl:
  case I_Lsr:
  case I_Asr:
  case I_Save:
  case I_Seti1:
  case I_Seti2:
  case I_Load:
  case I_Input:
  case I_Loadb:
  case I_Sli:
  case I_Ftoi:
    ret = GREG;
    break;
  case I_Inputf:
  case I_Setf1:
  case I_Setf2:
  case I_Loadf:
  case I_Loadbf:
  case I_Loadrf:    
  case I_Itof:
  case I_Addf:
  case I_Subf:
  case I_Mulf:
  case I_Sqrtf:    
  case I_Invf:
  case I_Fabs:
  case I_Fneg:  
  case I_Floor:
    ret = FREG;
    break;
  default:
    ret = NONE;
  }
  return ret; 
}

size_t
opcode_symbol(char *dst,uint8_t op,size_t n){
  if(op == I_Nop)
    strncpy(dst,"nop",n);
  else if(op == I_Add)
    strncpy(dst,"add",n);
  else if(op == I_Ble)
    strncpy(dst,"ble",n);
  else if(op == I_Addi)
    strncpy(dst,"addi",n);
  else if(op == I_Sub)
    strncpy(dst,"sub",n);
  else if(op == I_Blei)
    strncpy(dst,"blei",n);
  else if(op == I_Subi)
    strncpy(dst,"subi",n);
  else if(op == I_Loadr)
    strncpy(dst,"loadr",n);
  else if(op == I_Storer)
    strncpy(dst,"storer",n);
  else if(op == I_Loadrf)
    strncpy(dst,"loadrf",n);
  else if(op == I_Storerf)
    strncpy(dst,"storerf",n);  
  else if(op == I_Blt)    
    strncpy(dst,"blt",n);
  else if(op == I_Floor)
    strncpy(dst,"floor",n);
  else if(op == I_Fabs)
    strncpy(dst,"fabs",n);  
  else if(op == I_Fneg)    
    strncpy(dst,"Fneg",n);
  else if(op == I_Bltf)
    strncpy(dst,"bltf",n);
  else if(op == I_Blti)
    strncpy(dst,"blti",n);
  else if(op == I_Beq)
    strncpy(dst,"beq",n);
  else if(op == I_Beqf)
    strncpy(dst,"beqf",n);
  else if(op == I_Beqi)
    strncpy(dst,"beqi",n);
  else if(op == I_Sl)
    strncpy(dst,"sl",n);
  else if(op == I_Lsr)
    strncpy(dst,"Lsr",n);
  else if(op == I_Asr)
    strncpy(dst,"Asr",n);
  else if(op == I_Jmp)
    strncpy(dst,"jmp",n);
  else if(op == I_Jmpr)
    strncpy(dst,"jmpr",n);
  else if(op == I_Save)
    strncpy(dst,"save",n);          
  else if(op == I_Seti1)
    strncpy(dst,"seti1",n);
  else if(op == I_Seti2)
    strncpy(dst,"seti2",n);
  else if(op == I_Setf1)
    strncpy(dst,"setf1",n);
  else if(op == I_Setf2)
    strncpy(dst,"setf2",n);
  else if(op == I_Load)
    strncpy(dst,"load",n);
  else if(op == I_Store)
    strncpy(dst,"store",n);
  else if(op == I_Input)
    strncpy(dst,"input",n);          
  else if(op == I_Inputf)
    strncpy(dst,"inputf",n);
  else if(op == I_Output)
    strncpy(dst,"output",n);
  else if(op == I_Loadb)
    strncpy(dst,"loadb",n);
  else if(op == I_Storeb)
    strncpy(dst,"storeb",n);
  else if(op == I_Loadbf)
    strncpy(dst,"loadbf",n);
  else if(op == I_Storebf)
    strncpy(dst,"storebf",n);
  else if(op == I_Loadf)
    strncpy(dst,"loadf",n);          
  else if(op == I_Storef)
    strncpy(dst,"storef",n);
  else if(op == I_Sli)
    strncpy(dst,"sli",n);
  else if(op == I_Addf)
    strncpy(dst,"addf",n);
  else if(op == I_Ftoi)
    strncpy(dst,"ftoi",n);
  else if(op == I_Itof)
    strncpy(dst,"itof",n);  
  else if(op == I_Subf)
    strncpy(dst,"subf",n);
  else if(op == I_Mulf)
    strncpy(dst,"mulf",n);
  else if(op == I_Invf)
    strncpy(dst,"invf",n);
  else if(op == I_Sqrtf)
    strncpy(dst,"sqrtf",n);              
  else
    strncpy(dst,"undef",n);
  return n;
}

inline uint8_t
unit_number(uint8_t op){
  uint8_t ret;
  switch(op){    
  case I_Add:
  case I_Sub:
  case I_Addi:
  case I_Subi:
  case I_Sl:
  case I_Lsr:
  case I_Asr:
  case I_Sli:    
  case I_Save:
  case I_Seti1:
  case I_Seti2:
    
  case I_Itof:
    ret = U_NOE;
    break;
  case I_Input:
  case I_Inputf:
  case I_Output:
    ret = U_KAI;
    break;
  case I_Loadr:
  case I_Storer:
  case I_Loadrf:
  case I_Storerf:
  case I_Load:    
  case I_Store:
  case I_Loadf:    
  case I_Storef:    
  case I_Loadb:
  case I_Storeb:
  case I_Loadbf:
  case I_Storebf:
    ret = U_MMU;
    break;
  case I_Ble:
  case I_Blei:
  case I_Blt:
  case I_Bltf:
  case I_Blti:
  case I_Beq:
  case I_Beqf:
  case I_Beqi:
  case I_Jmpr:
    ret = U_BRA;
    break;
  case I_Setf1:    
  case I_Setf2:
  case I_Floor:
  case I_Fabs:
  case I_Fneg:

  case I_Addf:
  case I_Subf:
  case I_Mulf:
  case I_Invf:
  case I_Sqrtf:        
  case I_Ftoi:
    ret = U_FLT;
    break;
  default:
    ret = U_UNDEFINED;
    break;  
  }
  return ret;
}
