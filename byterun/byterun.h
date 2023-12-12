typedef enum {
  L_PLUS = 0,
  L_MINUS,
  L_MUL,  
  L_DIV,  
  L_MOD,  
  L_LT,   
  L_LTEQ, 
  L_GT,   
  L_GTEQ, 
  L_EQ,   
  L_NEQ,  
  L_AND,  
  L_OR
} BinOpCodes;

typedef enum {
  PATT_STR = 0,
  PATT_TAG_STR,
  PATT_TAG_ARR,
  PATT_TAG_SEXP,
  PATT_BOXED,   
  PATT_UNBOXED, 
  PATT_TAG_CLOSURE
} PattOpCodes;

typedef enum {
  L_BINOP = 0,
  L_LD = 2,
  L_LDA = 3,
  L_ST = 4,
  L_PATT = 6,
  L_EXIT = 15
} ComplexOpCodes;

typedef enum {
    L_CONST = 0x10,
    L_STRING = 0x11,
    L_SEXP = 0x12,
    L_STI = 0x13,
    L_STA = 0x14,
    L_JMP = 0x15,
    L_END = 0x16,
    L_RET = 0x17,
    L_DROP = 0x18,
    L_DUP = 0x19,
    L_SWAP = 0x1a,
    L_ELEM = 0x1b,
    L_CJMP_Z = 0x50,
    L_CJMP_NZ = 0x51,
    L_BEGIN = 0x52,
    L_CBEGIN = 0x53,
    L_CLOSURE = 0x54,
    L_CALLC = 0x55,
    L_CALL = 0x56,
    L_TAG = 0x57,
    L_ARRAY = 0x58,
    L_FAIL = 0x59,
    L_LINE = 0x5a,
    L_CALL_READ = 0x70,
    L_CALL_WRITE = 0x71,
    L_CALL_LENGTH = 0x72,
    L_CALL_STRING = 0x73,
    L_CALL_ARRAY = 0x74
} SimpleOpCodes;