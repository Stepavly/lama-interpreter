#include "../runtime/gc.h"
#include "../runtime/runtime_common.h"
#include "../runtime/runtime.h"
#include "../byterun/byterun.c"

extern size_t __gc_stack_top, __gc_stack_bottom;

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: lamai <lama executable file>");
    return 1;
  }

  bytefile* file = read_file(argv[1]);
  

  __gc_init();
}