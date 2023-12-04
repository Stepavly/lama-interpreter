/* Lama SM Bytecode interpreter */

# include <string.h>
# include <stdio.h>
# include <errno.h>
# include <malloc.h>
# include "../runtime/runtime.h"
# include "../runtime/virt_stack.h"

extern void __gc_init();
extern size_t *__gc_stack_top, *__gc_stack_bottom;
extern size_t SPACE_SIZE;

void *__start_custom_data;
void *__stop_custom_data;

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
  case 0: // "+"
    return (size_t) Ls__Infix_43((void*) left, (void*) right);
  case 1: // "-"
    return (size_t) Ls__Infix_45((void*) left, (void*) right);
  case 2: // "*"
    return (size_t) Ls__Infix_42((void*) left, (void*) right);
  case 3: // "/"
    return (size_t) Ls__Infix_47((void*) left, (void*) right);
  case 4: // "%"
    return (size_t) Ls__Infix_37((void*) left, (void*) right);
  case 5: // "<"
    return (size_t) Ls__Infix_60((void*) left, (void*) right);
  case 6: // "<="
    return (size_t) Ls__Infix_6061((void*) left, (void*) right);
  case 7: // ">"
    return (size_t) Ls__Infix_62((void*) left, (void*) right);
  case 8: // ">="
    return (size_t) Ls__Infix_6261((void*) left, (void*) right);
  case 9: // "=="
    return (size_t) Ls__Infix_6161((void*) left, (void*) right);
  case 10: // "!="
    return (size_t) Ls__Infix_3361((void*) left, (void*) right);
  case 11: // "&&"
    return (size_t) Ls__Infix_3838((void*) left, (void*) right);
  case 12: // "!!"
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
    case 15:
      return;
      
    /* BINOP */
    case 0:
      size_t right = vstack_pop(v_stack);
      size_t left = vstack_pop(v_stack);
      vstack_push(v_stack, eval_binop(left, right, l - 1));
      break;
      
    case 1:
      size_t value;
      switch (l) {
      case  0: // CONST
        vstack_push(v_stack, (size_t) BOX(INT));
        break;
        
      case  1:
        vstack_push(v_stack, (size_t) Bstring(STRING));
        break;
          
      case  2: { // SEXP
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
        
      case  3: // STI
        failure("Unsupported STI byte code");
        break;
        
      case  4: // STA
        value = vstack_pop(v_stack);
        size_t index = vstack_pop(v_stack);
        if (UNBOXED(index)) {
          size_t x = vstack_pop(v_stack);
          vstack_push(v_stack, (size_t) Bsta((void*) value, (int) index, (void*) x));
        } else {
          vstack_push(v_stack, (size_t) Bsta((void*) value, (int) index, NULL));
        }
        break;
        
      case  5: // JMP
        ip = bf->code_ptr + INT;
        break;
        
      case  6: { // END
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
        
      case  7: // RET
        failure("Unsupported RET byte code");
        break;
        
      case  8: // DROP
        vstack_pop(v_stack);
        break;
        
      case  9: // DUP
        value = vstack_pop(v_stack);
        vstack_push(v_stack, value);
        vstack_push(v_stack, value);
        break;
        
      case 10:  // SWAP
        size_t high = vstack_pop(v_stack);
        size_t bot = vstack_pop(v_stack);
        vstack_push(v_stack, high);
        vstack_push(v_stack, bot);
        break;

      case 11: { // ELEM
        size_t index = vstack_pop(v_stack);
        size_t value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Belem((void*) value, index));
        break;
      }
        
      default:
        FAIL;
      }
      break;
      
    case 2:  // LD
      vstack_push(v_stack, *get_memory_addr(l, INT, fp, bf, v_stack));
      break;
    case 3: // LDA
      vstack_push(v_stack, (size_t) get_memory_addr(l, INT, fp, bf, v_stack));
      break;
    case 4: { // ST
      size_t value = vstack_pop(v_stack);
      size_t *stack_value = get_memory_addr(l, INT, fp, bf, v_stack);
      *stack_value = value;
      vstack_push(v_stack, value);
      break;
    }
      break;
      
    case 5:
      int addr;
      switch (l) {
      case  0: // CJMPz
        addr = INT;
        if (UNBOX(vstack_pop(v_stack)) == 0) {
          ip = bf->code_ptr + addr;
        }
        break;
        
      case  1: // CJMPnz
        addr = INT;
        if (UNBOX(vstack_pop(v_stack)) != 0) {
          ip = bf->code_ptr + addr;
        }
        break;
        
      case  2: // BEGIN
      case  3: { // CBEGIN
        int argc = INT;
        int localc = INT;

        vstack_push(v_stack, (size_t) fp);
        fp = vstack_top(v_stack);
        for (int i = 0; i < localc; i++) {
          vstack_push(v_stack, BOX(0));
        }
      }
        break;
        
      case  4: { // CLOSURE
        size_t addr = INT;
        size_t argc = INT;
        int* values = (int*) malloc(sizeof(int) * argc);
        if (values == NULL) {
          failure("Failed to allocate memory for closure args");
        }
        for (size_t i = 0; i < argc; i++) {
          char type = BYTE;
          values[i] = *get_memory_addr(type, INT, fp, bf, v_stack);
        }

        vstack_push(v_stack, (size_t) Bclosure_values(BOX(argc), (void*) (bf->code_ptr + addr), values));
        free(values);
        break;
      }
          
      case  5: { // CALLC
        size_t argc = INT;
        vstack_reverse(v_stack, argc);

        char* func_ip = (char*) Belem((void*) vstack_top(v_stack)[argc], BOX(0));
        vstack_push(v_stack, (size_t) ip);
        vstack_push(v_stack, argc + 1);
        ip = func_ip;
        break;
      }
        
      case  6: { // CALL
        size_t addr = INT;
        size_t argc = INT;
        vstack_reverse(v_stack, argc);
        vstack_push(v_stack, (size_t) ip);
        vstack_push(v_stack, argc);
        ip = bf->code_ptr + addr;
        break;
      }
        
      case  7: // TAG
        char* name = STRING;
        size_t size = (size_t) INT;
        size_t data = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Btag((void*) data, LtagHash(name), BOX(size)));
        break;
        
      case  8: { // ARRAY
        size_t size = INT;
        size_t arr = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Barray_patt((void*) arr, BOX(size)));
        break;
      }
        
      case  9:
        int a = INT;
        int b = INT;
        failure("FAIL %d %d", a, b);
        break;
        
      case 10: // LINE
        INT;
        break;

      default:
        FAIL;
      }
      break;
      
    case 6: // PATT
      switch (l)
      {
      case 0: // StrCmp
        size_t value1 = vstack_pop(v_stack);
        size_t value2 = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bstring_patt((void*) value1, (void*) value2));
        break;

      case 1: // String
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bstring_tag_patt((void*) value));
        break;

      case 2: // Array
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Barray_tag_patt((void*) value));
        break;
      
      case 3: // Sexp
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bsexp_tag_patt((void*) value));
        break;

      case 4: // Boxed
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bboxed_patt((void*) value));
        break;
      
      case 5: // Unboxed
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bunboxed_patt((void*) value));
        break;

      case 6: // Fun
        value = vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Bclosure_tag_patt((void*) value));
        break;
      
      default:
        failure("Unexpected %d code in PATT command", (int) l);
        break;
      }
      break;

    case 7: {
      switch (l) {
      case 0:
        vstack_push(v_stack, (size_t) Lread());
        break;
        
      case 1:
        vstack_push(v_stack, (size_t) Lwrite(vstack_pop(v_stack)));
        break;

      case 2:
        void* array = (void*) vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Llength(array));
        break;

      case 3:
        void* value = (void*) vstack_pop(v_stack);
        vstack_push(v_stack, (size_t) Lstring(value));
        break;

      case 4: {
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
      }
    }
    break;
      
    default:
      FAIL;
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
