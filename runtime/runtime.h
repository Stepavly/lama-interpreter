# ifndef __LAMA_RUNTIME__
# define __LAMA_RUNTIME__

# include <stdio.h>
# include <stdio.h>
# include <string.h>
# include <stdarg.h>
# include <stdlib.h>
# include <sys/mman.h>
# include <assert.h>
# include <errno.h>
# include <regex.h>
# include <time.h>
# include <limits.h>
# include <ctype.h>

# define WORD_SIZE (CHAR_BIT * sizeof(int))

# define UNBOXED(x)  (((int) (x)) &  0x0001)
# define UNBOX(x)    (((int) (x)) >> 1)
# define BOX(x)      ((((int) (x)) << 1) | 0x0001)

# define ASSERT_BOXED(memo, x)               \
  do if (UNBOXED(x)) failure ("boxed value expected in %s\n", memo); while (0)
# define ASSERT_UNBOXED(memo, x)             \
  do if (!UNBOXED(x)) failure ("unboxed value expected in %s\n", memo); while (0)

void failure (char *s, ...);

int Lread();
int Lwrite(int);
int Llength(void*);
void* Lstring (void *p);
void* Bstring(void*);
void* Belem (void *p, int i);
void* Bsta (void *v, int i, void *x);
void* Barray_data (int bn, int *data_);
void* Bsexp_data (int bn, int tag, int *data_);
int LtagHash (char *s);
int Btag (void *d, int t, int n);
int Barray_patt (void *d, int n);
void* Bclosure_values (int bn, void *entry, int *values);
void* Belem_ref (void *p, int i);
int Bstring_patt (void *x, void *y);
int Bstring_tag_patt (void *x);
int Barray_tag_patt (void *x);
int Bsexp_tag_patt (void *x);
int Bunboxed_patt (void *x);
int Bboxed_patt (void *x);
int Bclosure_tag_patt (void *x);

# endif