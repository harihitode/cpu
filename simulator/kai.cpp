#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "sim.h"
#include "rsv.h"
#include "kai.h"

extern simerror_t sim_error;
extern uint8_t step_mark;
extern uint32_t output_count;

serial_port::serial_port() : station(1){
  ifp = NULL;
  ofp = fopen("output","w");
}

void
serial_port::set_input(char *file_name){
  ifp = fopen(file_name,"r");
  if(ifp == NULL)
    sim_error = ER_INVALID_INPUT;
  return;
}

serial_port::~serial_port(){
  fclose(ifp);
  fclose(ofp);
}

cdbT serial_port::cdb_out(){
  cdbT ret;
  uint32_t o1;
  uint8_t op;
  fi box;
  station::ready_st_no();  
  op = readop();
  o1 = read1();
  if(op == I_Input){
    ret.rob = readrob();
    if(!fscanf(ifp,"%d",&(box.s)))
      sim_error = ER_MEMORY_ACCESS;
    ret.i = box.i;
  }else if(op == I_Inputf){
    ret.rob = readrob();
    if(!fscanf(ifp,"%f",&(box.f)))
      sim_error = ER_MEMORY_ACCESS;    
    ret.i = box.i;    
  }else if(op == I_Output){
    output_count++;    
    fwrite(&o1,sizeof(uint8_t),1,ofp);
    fflush(ofp);
    if(output_count == 49167)
      sim_error = ER_SUCCESS;
  }else
    ret.rob = 0;
  return ret;
}
