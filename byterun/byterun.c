/* Lama SM Bytecode interpreter */

# include <string.h>
# include <stdio.h>
# include <errno.h>
# include <malloc.h>
# include "../runtime/runtime.h"
# include "byterun.h"

extern void __gc_init();
extern size_t *__gc_stack_top, *__gc_stack_bottom;
extern size_t SPACE_SIZE;

void *__start_custom_data;
void *__stop_custom_data;

struct {
  size_t *bot;
  size_t *top;
  size_t **cur;
} typedef virt_stack;

static inline virt_stack *vstack_create (size_t size, size_t **cur) {
  virt_stack *st = (virt_stack*) malloc(sizeof(virt_stack));
  if (st == NULL) {
    return NULL;
  }

  size_t *stack = (size_t*) malloc(sizeof(size_t) * size);
  if (stack == NULL) {
    free(st);
    return NULL;
  }

  st->bot = stack;
  st->top = stack + size;
  st->cur = cur;
  *cur = st->top;
  return st;
}

static inline void vstack_destruct (virt_stack *st) { 
  free(st->bot);
  free(st); 
}

static inline size_t vstack_size (virt_stack *st) {
  return st->top - *st->cur;
}

static inline void vstack_push (virt_stack *st, size_t value) {
  if (*st->cur == st->bot) {
    failure("Stack overflow");
  }
  --(*st->cur);
  **st->cur = value;
}

static inline void vstack_reverse (virt_stack *st, size_t size) {
  if (vstack_size(st) < size) {
    failure("Stack size less than reversing size");
  }
  for (size_t i = 0; i * 2 < size; i++) {
    size_t tmp = *(*st->cur + i);
    *(*st->cur + i) = *(*st->cur + (size - i - 1));
    *(*st->cur + (size - i - 1)) = tmp;
  }
}

static inline size_t vstack_pop (virt_stack *st) {
  if (*st->cur == st->top) {
    failure("Pop from empty stack");
  }
  size_t value = **st->cur;
  ++(*st->cur);
  return value;
}

static inline size_t *vstack_top (virt_stack *st) { 
  return *st->cur;
}

static inline size_t vstack_kth_from_cur (virt_stack *st, size_t k) {
  if (vstack_size(st) < k) {
    failure("Stack size less than index");
  }
  return *(*st->cur + k);
}

static inline size_t *vstack_access(virt_stack *st, size_t *addr) {
  if (!(*st->cur <= addr && addr < st->top)) {
    failure("Accessing stack by removed address");
  }
  return addr;
}

/* The unpacked representation of bytecode file */
typedef struct {
  char *string_ptr;              /* A pointer to the beginning of the string table */
  int  *public_ptr;              /* A pointer to the beginning of publics table    */
  char *code_ptr;                /* A pointer to the bytecode itself               */
  int  *global_ptr;              /* A pointer to the global area                   */
  int   stringtab_size;          /* The size (in bytes) of the string table        */
  int   global_area_size;        /* The size (in words) of global area             */
  int   public_symbols_number;   /* The number of public symbols                   */
  char  buffer[0];               
} bytefile;

/* Gets a string from a string table by an index */
char* get_string (bytefile *f, int pos) {
  return &f->string_ptr[pos];
}

/* Gets a name for a public symbol */
char* get_public_name (bytefile *f, int i) {
  return get_string (f, f->public_ptr[i*2]);
}

/* Gets an offset for a publie symbol */
int get_public_offset (bytefile *f, int i) {
  return f->public_ptr[i*2+1];
}

/* Reads a binary bytecode file by name and unpacks it */
bytefile* read_file (char *fname) {
  FILE *f = fopen (fname, "rb");
  long size;
  bytefile *file;

  if (f == 0) {
    failure ("%s\n", strerror (errno));
  }
  
  if (fseek (f, 0, SEEK_END) == -1) {
    failure ("%s\n", strerror (errno));
  }

  file = (bytefile*) malloc (sizeof(int)*4 + (size = ftell (f)));

  if (file == 0) {
    failure ("*** FAILURE: unable to allocate memory.\n");
  }
  
  rewind (f);

  if (size != fread (&file->stringtab_size, 1, size, f)) {
    failure ("%s\n", strerror (errno));
  }
  
  fclose (f);
  
  file->string_ptr  = &file->buffer [file->public_symbols_number * 2 * sizeof(int)];
  file->public_ptr  = (int*) file->buffer;
  file->code_ptr    = &file->string_ptr [file->stringtab_size];
  file->global_ptr  = (int*) malloc (file->global_area_size * sizeof (int));
  
  return file;
}

size_t eval_binop(size_t left, size_t right, char binop_code) {
  switch (binop_code)
  {
  case L_PLUS: // "+"
    return (size_t) Ls__Infix_43((void*) left, (void*) right);
  case L_MINUS: // "-"
    return (size_t) Ls__Infix_45((void*) left, (void*) right);
  case L_MUL: // "*"
    return (size_t) Ls__Infix_42((void*) left, (void*) right);
  case L_DIV: // "/"
    return (size_t) Ls__Infix_47((void*) left, (void*) right);
  case L_MOD: // "%"
    return (size_t) Ls__Infix_37((void*) left, (void*) right);
  case L_LT: // "<"
    return (size_t) Ls__Infix_60((void*) left, (void*) right);
  case L_LTEQ: // "<="
    return (size_t) Ls__Infix_6061((void*) left, (void*) right);
  case L_GT: // ">"
    return (size_t) Ls__Infix_62((void*) left, (void*) right);
  case L_GTEQ: // ">="
    return (size_t) Ls__Infix_6261((void*) left, (void*) right);
  case L_EQ: // "=="
    return (size_t) Ls__Infix_6161((void*) left, (void*) right);
  case L_NEQ: // "!="
    return (size_t) Ls__Infix_3361((void*) left, (void*) right);
  case L_AND: // "&&"
    return (size_t) Ls__Infix_3838((void*) left, (void*) right);
  case L_OR: // "!!"
    return (size_t) Ls__Infix_3333((void*) left, (void*) right);
  
  default:
    failure("Unexpected binop code %d", (int) binop_code);
    // Dead code
    return 0;
  }
}

size_t* get_memory_addr(char type, int offset, size_t *fp, bytefile *bf, virt_stack *v_stack) {
  switch (type)
  {
  case 0: // Global
    return bf->global_ptr + offset;

  case 1: // Local
    return vstack_access(v_stack, fp - offset - 1);
  
  case 2: // Arg
    return vstack_access(v_stack, fp + offset + 3);

  case 3: // Access
    size_t closure_args = *vstack_access(v_stack, fp + 1) - 1;
    size_t closure_addr = *vstack_access(v_stack, fp + 3 + closure_args);
    return (size_t*) Belem_ref((void*) closure_addr, BOX(offset + 1));

  default:
    break;
  }
}

/* Disassembles the bytecode pool */
void interpret (bytefile *bf, virt_stack* v_stack) {
  
# define INT    (ip += sizeof (int), *(int*)(ip - sizeof (int)))
# define BYTE   *ip++
# define STRING get_string (bf, INT)
# define FAIL   failure ("ERROR: invalid opcode %d-%d\n", h, l)
  

  vstack_push(v_stack, 0);
  vstack_push(v_stack, 0);
  vstack_push(v_stack, 0);
  vstack_push(v_stack, 2);

  char *ip     = bf->code_ptr;
  size_t *fp   = vstack_top(v_stack);
  while (ip != NULL) {
    char x = BYTE,
         h = (x & 0xF0) >> 4,
         l = x & 0x0F;
    
    switch (h) {
    case L_EXIT:
      return;
      
    case L_BINOP:
      size_t right = vstack_pop(v_stack);
      size_t left = vstack_pop(v_stack);
      vstack_push(v_stack, eval_binop(left, right, l - 1));
      continue;
      
    case L_LD:
      vstack_push(v_stack, *get_memory_addr(l, INT, fp, bf, v_stack));
      continue;

    case L_LDA:
      vstack_push(v_stack, (size_t) get_memory_addr(l, INT, fp, bf, v_stack));
      continue;
    
    case L_ST: {
      size_t value = vstack_pop(v_stack);
      size_t *stack_value = get_memory_addr(l, INT, fp, bf, v_stack);
      *stack_value = value;
      vstack_push(v_stack, value);
      continue;
    }
      
    case L_PATT:
      size_t value;
      switch (l)
      {
      case PATT_STR:
        size_t value1 = vstack_pop(v_stack);
        size_t value2 = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bstring_patt((void*) value1, (void*) value2));
        continue;

      case PATT_TAG_STR:
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bstring_tag_patt((void*) value));
        continue;

      case PATT_TAG_ARR:
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Barray_tag_patt((void*) value));
        continue;
      
      case PATT_TAG_SEXP:
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bsexp_tag_patt((void*) value));
        continue;

      case PATT_BOXED:
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bboxed_patt((void*) value));
        continue;
      
      case PATT_UNBOXED:
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bunboxed_patt((void*) value));
        continue;

      case PATT_TAG_CLOSURE:
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bclosure_tag_patt((void*) value));
        continue;
      
      default:
        failure("Unexpected %d code in PATT command", (int) l);
        continue;
      }
      continue;
      
    default:
      // nop
    }
  
    size_t value;
    int addr;
    switch (x)
    {
    case L_CONST:
      vstack_push(v_stack, (size_t) BOX(INT));
      break;

    case L_STRING:
      vstack_push(v_stack, (size_t) Bstring(STRING));
      break;

    case L_SEXP: {
        char* name = STRING;
        size_t argc = (size_t) INT;
        int tag = LtagHash(name);
        vstack_reverse(v_stack, argc);
        void* sexp = Bsexp_data(BOX(argc + 1), tag, (int*) vstack_top(v_stack));
        for (size_t i = 0; i < argc; i++) {
          vstack_pop(v_stack);
        }
        vstack_push(v_stack, (size_t) sexp);
        break;
      }
        
    case L_STI:
      failure("Unsupported STI byte code");
      break;
      
    case L_STA:
      value = vstack_pop(v_stack);
      size_t index = vstack_pop(v_stack);
      if (UNBOXED(index)) {
        size_t x = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bsta((void*) value, (int) index, (void*) x));
      } else {
        vstack_push(v_stack, (size_t) Bsta((void*) value, (int) index, NULL));
      }
      break;
      
    case L_JMP:
      ip = bf->code_ptr + INT;
      break;
      
    case L_END: {
      size_t value = vstack_pop(v_stack);
      while (vstack_top(v_stack) != fp) {
        vstack_pop(v_stack);
      }
      fp = (size_t*) vstack_pop(v_stack);
      size_t argc = vstack_pop(v_stack);
      ip = (char*) vstack_pop(v_stack);
      
      for (size_t i = 0; i < argc; i++) {
        vstack_pop(v_stack);
      }
      vstack_push(v_stack, value);
      break;
    }
      
    case L_RET:
      failure("Unsupported RET byte code");
      break;
      
    case L_DROP:
      vstack_pop(v_stack);
      break;
      
    case L_DUP:
      value = vstack_pop(v_stack);
      vstack_push(v_stack, value);
      vstack_push(v_stack, value);
      break;
      
    case L_SWAP: 
      size_t high = vstack_pop(v_stack);
      size_t bot = vstack_pop(v_stack);
      vstack_push(v_stack, high);
      vstack_push(v_stack, bot);
      break;

    case L_ELEM: { 
      size_t index = vstack_pop(v_stack);
      size_t value = vstack_pop(v_stack);
      vstack_push(v_stack, (size_t) Belem((void*) value, index));
      break;
    }

    case L_CJMP_Z: 
      addr = INT;
      if (UNBOX(vstack_pop(v_stack)) == 0) {
        ip = bf->code_ptr + addr;
      }
      break;
      
    case L_CJMP_NZ:
      addr = INT;
      if (UNBOX(vstack_pop(v_stack)) != 0) {
        ip = bf->code_ptr + addr;
      }
      break;
      
    case L_BEGIN:
    case L_CBEGIN: {
      int argc = INT;
      int localc = INT;

      vstack_push(v_stack, (size_t) fp);
      fp = vstack_top(v_stack);
      for (int i = 0; i < localc; i++) {
        vstack_push(v_stack, BOX(0));
      }
    }
      break;
      
    case L_CLOSURE: {
      size_t addr = INT;
      size_t argc = INT;
      int values[argc];
      for (size_t i = 0; i < argc; i++) {
        char type = BYTE;
        values[i] = *get_memory_addr(type, INT, fp, bf, v_stack);
      }

      vstack_push(v_stack, (size_t) Bclosure_values(BOX(argc), (void*) (bf->code_ptr + addr), values));
      break;
    }
        
    case L_CALLC: {
      size_t argc = INT;
      vstack_reverse(v_stack, argc);

      char* func_ip = (char*) Belem((void*) vstack_top(v_stack)[argc], BOX(0));
      vstack_push(v_stack, (size_t) ip);
      vstack_push(v_stack, argc + 1);
      ip = func_ip;
      break;
    }
      
    case L_CALL: {
      size_t addr = INT;
      size_t argc = INT;
      vstack_reverse(v_stack, argc);
      vstack_push(v_stack, (size_t) ip);
      vstack_push(v_stack, argc);
      ip = bf->code_ptr + addr;
      break;
    }
      
    case L_TAG:
      char* name = STRING;
      size_t size = (size_t) INT;
      size_t data = vstack_pop(v_stack);
      vstack_push(v_stack, (size_t) Btag((void*) data, LtagHash(name), BOX(size)));
      break;
      
    case L_ARRAY: {
      size_t size = INT;
      size_t arr = vstack_pop(v_stack);
      vstack_push(v_stack, (size_t) Barray_patt((void*) arr, BOX(size)));
      break;
    }
      
    case L_FAIL:
      int a = INT;
      int b = INT;
      failure("FAIL %d %d", a, b);
      break;
      
    case L_LINE:
      INT;
      break;

    case L_CALL_READ:
      vstack_push(v_stack, (size_t) Lread());
      break;
      
    case L_CALL_WRITE:
      vstack_push(v_stack, (size_t) Lwrite(vstack_pop(v_stack)));
      break;

    case L_CALL_LENGTH:
      void* array = (void*) vstack_pop(v_stack);
      vstack_push(v_stack, (size_t) Llength(array));
      break;

    case L_CALL_STRING:
      void* value = (void*) vstack_pop(v_stack);
      vstack_push(v_stack, (size_t) Lstring(value));
      break;

    case L_CALL_ARRAY: {
      int size = INT;
      vstack_reverse(v_stack, (size_t) size);
      void* array = Barray_data(BOX(size), (int*) *v_stack->cur);
      for (int i = 0; i < size; i++) {
        vstack_pop(v_stack);
      }
      vstack_push(v_stack, (size_t) array);
      break;
    }

    default:
      FAIL;
      break;
    }
  }
}

int main (int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: byterun <lama executable file>");
    return 1;
  }
  
  bytefile *f = read_file (argv[1]);

  virt_stack* v_stack = vstack_create(SPACE_SIZE, &__gc_stack_top);
  if (v_stack == NULL) {
    failure("Failed to create virtual stack");
  }
  
  __gc_stack_bottom = __gc_stack_top;
  __gc_init();

  interpret(f, v_stack);

  vstack_destruct(v_stack);
  return 0;
}
