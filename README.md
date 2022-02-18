# SimpleMemModAnalyzer

## Approach


## Function Overview
#### Primary Functions
| Function | Description |
| --- | --- |
| `bool isFunctionHeader(char *line, char **types, int num_types)` | Returns true if and only if `line` is a function header, where `types` is an array of all valid types and `num_types` is the size of the array `types` |
| `FunctionNode *initFunction(char *header, FunctionNode *func_head, MemNode *stack_head)` | Initializes a FunctionLL node for the current function whose header is `header` and inserts the new node in the linked list whose head is `head` |
| `bool isVar(char *line, char **types, int num_types, FunctionNode *curr_func, MemNode *ro_head, MemNode *static_head, MemNode *heap_head, MemNode *stack_head)` | Returns true if anf only if `line` contains a variable declaration and inserts the variable into the appropriate linked list if a variable is contained, where `types` is an array of all valid types and `num_types` is the size of the array `types`|
| `int readFile(Stats *stats, int argc, char **argv, FunctionNode *func_head, MemNode *ro_head, MemNode *static_head, MemNode *heap_head, MemNode *stack_head)` | Iterates and reads through lines of source file. Returns 0 if and only if there is a valid file indicated by user |
| `void printOutput(Stats *stats, FunctionNode *func_head, MemNode *ro_head, MemNode *static_head, MemNode *heap_head, MemNode *stack_head)` | Prints the output given information read from source file |

#### Helper Functions
| Function | Description |
| --- | --- |
| `int getSize(char *type, int num_elements)` | Returns the size of memory occupied by `num_elements` elements of type `type` |
| `void insertFuncNode(FunctionNode *func_head, FunctionNode *node)` | Inserts FunctionNode `node` into linked list whose head is `head` |
| `void insertMemNode(MemNode *func_head, MemNode *node)` | Inserts MemNode `node` into linked list whose head is `head` |
| `void deleteFunctionList(FunctionNode *head)` | Deletes a FunctionNode linked list whose head is `head` |
| `void deleteMemList(MemNode *head)` | Deletes a MemNode linked list whose head is `head` |
| `bool isWhitespace(char *str)` | Returns true iff `str` is whitespace |

## Running the Program
  1. Navigate to the directory (i.e., `cd`) in which `SimpleMemModAnalyzer.c` is saved on your machine.
  2. In the terminal, enter `gcc SimpleMemModAnalyzer.c -o SimpleMemModAnalyzer` to compile the program.
  3. To execute the program, enter `./SimpleMemModAnalyzer file.c` into the terminal, where `file` is the name of the file with the C source code.

## Test Cases
#### Test Case #1
- Contains global variables (part of static data) 
- All variables in functions should go to the stack
- Empty heap and RO data
- Comments are present (should be ignored by program)

**Source Code:**
```
// evil global variables
int evil_glob_var_1;
float evil_glob_var_2;

void fun1(int x)
{
  int y;
  int z;
  printf("%d \n", x+y+z); 
}

int fun2(float z)
{
   float x;
   return (int)(z+x);
}

int main(int argc, char** argv)
{
  int w;

  fun1(w);
  fun2();

  return 0;
}
```

**Output:**
```
>>> Memory Model Layout <<<
***  exec // text ***
   prog1.c

### ROData ###       scope  type  size

### static data ###
   evil_glob_var_1   global   int   4
   evil_glob_var_2   global   float   4

### heap ###

####################
### unused space ###
####################

### stack ###
   x   fun1   int   4
   y   fun1   int   4
   z   fun1   int   4
   z   fun2   float   4
   x   fun2   float   4
   argc   main   int   4
   argv   main   char**   8
   w   main   int   4

**** STATS ****
  - Total number of lines in the file: 26
  - Total number of functions: 3
    fun1, fun2, main
  - Total number of lines per functions:
    fun1: 3
    fun2: 2
    main: 6
  - Total number of variables per function:
    fun1: 3
    fun2: 2
    main: 3
//////////////////////////////
```

#### Test Case #2
- Has variables that are present on stack
- Has dynamically-allocated memory (using `malloc`) 
  - Both in a line in which the pointer variable is declared as well as in a line in which it is not declared
- Has an array
- Comments are present (should be ignored by program)

**Source Code:**
```
/* prog2.c */


void f1(int **i, const char *x)
{
 i = malloc(sizeof(int));
 int *h = malloc(sizeof(char **));
}


int f2()
{

  float x[5];
}

int main(int argc, char **argv)
{
  int i;
  int j;

  i = f2();
  f1(j);
}

```

**Output:**
```
***  exec // text ***
   prog2.c

### ROData ###       scope  type  size

### static data ###

### heap ###
   i   f1   int    4
   h   f1   int   8

####################
### unused space ###
####################

### stack ###
   i   f1   int **   8
   x   f1   const char *   8
   h   f1   int*   8
   x   f2   float[]   20
   argc   main   int   4
   argv   main   char **   8
   i   main   int   4
   j   main   int   4

**** STATS ****
  - Total number of lines in the file: 24
  - Total number of functions: 3
    f1, f2, main
  - Total number of lines per functions:
    f1: 2
    f2: 2
    main: 5
  - Total number of variables per function:
    f1: 3
    f2: 1
    main: 4
//////////////////////////////
```

#### Test Case #3
- Has variables that are present on stack
- Has global variables and variables declared as `static` (both are stored in static data)

**Source Code:**
```
/* prog3.c */

int x = 10;
int y;
 
int f(int p, int q)
{
    static int j = 5;

    x = 5;
    return p * q + j;
}
 
int main()
{
   int i = x;
 
   y = f(i, i);
   return 0;
}

```

**Output:**
```
>>> Memory Model Layout <<<
***  exec // text ***
   prog3.c

### ROData ###       scope  type  size

### static data ###
   x   global   int   4
   y   global   int   4
   j   f   static int   4

### heap ###

####################
### unused space ###
####################

### stack ###
   p   f   int   4
   q   f   int   4
   i   main   int   4

**** STATS ****
  - Total number of lines in the file: 20
  - Total number of functions: 2
    f, main
  - Total number of lines per functions:
    f: 4
    main: 4
  - Total number of variables per function:
    f: 3
    main: 1
//////////////////////////////
```

#### Test Case #4
- Has variables that are present on stack
- Has arrays that are declared but not initialized
- Has functions whose return type is a pointer
- Has `malloc` call in which its parameter is not an integer or of the form `sizeof(T)` where `T` is a type

**Source Code:**
```
/* string_tools.c */

char *concat_wrong(const char *s1, const char *s2) 
{
  char result[70];

  strcpy (result, s1);
  strcat (result, s2);

  return result;
}


char *concat(const char *s1, const char *s2)
{
  char *result;

  result = malloc(strlen(s1) + strlen(s2) + 1);
  if (result == NULL) {
      printf ("Error: malloc failed\n");
      exit(1);
  }

  strcpy (result, s1);
  strcat (result, s2);

  return result;
}

```

**Output:**
```
>>> Memory Model Layout <<<
***  exec // text ***
   string_tools.c

### ROData ###       scope  type  size

### static data ###

### heap ###
   result   concat   char   strlen(s1) + strlen(s2) + 1

####################
### unused space ###
####################

### stack ###
   s1   concat_wrong   const char *   8
   s2   concat_wrong   const char *   8
   result   concat_wrong   char[]   70
   s1   concat   const char *   8
   s2   concat   const char *   8
   result   concat   char*   8

**** STATS ****
  - Total number of lines in the file: 28
  - Total number of functions: 2
    concat_wrong, concat
  - Total number of lines per functions:
    concat_wrong: 6
    concat: 12
  - Total number of variables per function:
    concat_wrong: 3
    concat: 3
//////////////////////////////
```
#### Test Case #5
- Has variables that are present on stack
- Has variables of the same type listed in a row, separated by commas
- Has a `for` loop
- Has an array initialized
- Has a string literal

**Source Code:**
```
/* sumArray.c */

int sum(int *a, int size) {
   int i, s = 0;

   for(i = 0; i < size; i++) 
       s += a[i];

   return s;
}


int main()
{
  int N = 5;
  int i[N] = {10, 9, 8, 7, 6};
  char string[1024] = "Hello World";

  printf("sum is %d\n", sum(i,N));

  return 0;
}

```

**Output:**
```
>>> Memory Model Layout <<<
***  exec // text ***
   sumArray.c

### ROData ###       scope  type  size
   string   main   char[]   1024

### static data ###

### heap ###

####################
### unused space ###
####################

### stack ###
   a   sum   int *   8
   size   sum   int   4
   i   sum   int   4
   N   main   int   4
   i   main   int[]   20
   string   main   char[]   1024

**** STATS ****
  - Total number of lines in the file: 22
  - Total number of functions: 2
    sum, main
  - Total number of lines per functions:
    sum: 6
    main: 7
  - Total number of variables per function:
    sum: 3
    main: 3
//////////////////////////////
```
