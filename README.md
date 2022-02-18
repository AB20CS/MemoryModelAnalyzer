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

## Test Cases

