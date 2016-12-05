#ifndef NATSUIRO_SIM_H
#define NATSUIRO_SIM_H
#include <stdint.h>
#include <stdlib.h>

#define DATASIZE 32

#define CMT_NUL 0
#define CMT_BRA 1
#define CMT_STR 2
#define CMT_GRG 3
#define CMT_FRG 4
#define CMT_FST 5

#define NONE 0
#define GREG 1
#define FREG 2

typedef enum _simerror_t {
  ER_NONE,
  ER_INVALID_INPUT,
  ER_NO_INPUT,
  ER_MEMORY_ACCESS,
  ER_NO_INSTRUCTION,  
  ER_TOO_LARGE_INSTRUCTION,
  ER_INVALID_PC,
  ER_SUCCESS
} simerror_t;

#define U_UNDEFINED 0
#define U_MMU 1
#define U_KAI 2
#define U_NOE 3
#define U_BRA 4
#define U_FLT 5

#define I_Nop 0
#define I_Add 1
#define I_Ble 2
#define I_Addi 3
#define I_Sub 4
#define I_Blei 5
#define I_Subi 6
#define I_Loadr 8
#define I_Storer 9
#define I_Loadrf 10
#define I_Storerf 11
#define I_Fabs 12
#define I_Floor 14
#define I_Fneg 15

#define I_Blt 16
#define I_Bltf 17
#define I_Blti 18
#define I_Beq 19
#define I_Beqf 20
#define I_Beqi 21

#define I_Sl 26
#define I_Lsr 27
#define I_Asr 28

#define I_Jmp 30
#define I_Jmpr 32
#define I_Save 34

#define I_Seti1 36
#define I_Seti2 37
#define I_Setf1 38
#define I_Setf2 39

#define I_Load 40
#define I_Store 42
#define I_Input 44
#define I_Inputf 45
#define I_Output 48

#define I_Loadb 50
#define I_Loadbf 51
#define I_Storeb 52
#define I_Storebf 53
#define I_Loadf 54
#define I_Storef 55

#define I_Itof 56
#define I_Ftoi 57

#define I_Addf 58
#define I_Subf 59
#define I_Mulf 60
#define I_Invf 61
#define I_Sqrtf 62
#define I_Sli 63

typedef struct _cdbT{
  uint8_t rob;
  uint32_t i;
} cdbT;

typedef struct _instT{
  uint32_t pr;
  uint32_t prpc;
  uint32_t pc;
  uint32_t i;
  uint8_t miss;
} instT;

typedef struct _sourceT{
  uint8_t opcode;
  uint8_t rob;
  uint8_t fnct;
  cdbT o1;
  cdbT o2;
} sourceT;

typedef struct _cmitT{
  uint8_t commit_no;
  uint8_t rob;
  uint32_t dest;
  uint32_t data;
  uint32_t prpc;
} cmitT;

typedef union _fi{
float f;
int s;
uint32_t i;
} fi;

uint8_t which_reg(uint8_t op);
uint8_t unit_number(uint8_t op);
size_t opcode_symbol(char *dst,uint8_t op,size_t n);
#endif
