#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// struct to hold STATS information
typedef struct StatsStruct {
    char program_name[1024];
    int num_lines; // number of lines in source code
    int num_functions; // number of functions in source code
} Stats;


// node in linked list for functions
typedef struct FunctionLinkedListNode {
    char function_name[1024];
    int num_lines;
    int num_variables;
    struct FunctionLinkedListNode *next;
} FunctionNode;

// node in linked list for a memory unit
typedef struct MemoryUnitListNode {
    char var_name[1024];
    char scope[1024];
    char type[1024];
    int size;
    struct MemoryUnitListNode *next;
} MemNode;

// holds head nodes in all linked lists used
typedef struct LinkedListHeads {
    FunctionNode *func_head;
    MemNode *ro_head;
    MemNode *static_head;
    MemNode *heap_head;
    MemNode *stack_head;
} LLHeads;

/**
 * Inserts node into linked list whose head is head
 * 
 */
void insertFuncNode(LLHeads* heads, FunctionNode *node) {

    if (heads->func_head == NULL) {
        heads->func_head = node;
    }
    else {
        FunctionNode *p = heads->func_head;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = node;
    }
    
}


/**
 * Returns true iff line is a function header
 * 
 */
bool isFunctionHeader(char *line, char **types, int num_types) {
    if (strstr(line, ";") || !strstr(line, "(")) {
        return false; // function headers do not contain ';' and must contain '('
    }
    else {
        for (int i = 0; i < num_types; i++) {
            if (strstr(line, types[i])) {
                return true;
            }
        }
    }
}

/**
 * Initializes a FunctionLL node for the current function
 * whose header is header and inserts the new node in the linked
 * list whose head is head
 * 
 */
FunctionNode *initFunction(char *header, LLHeads *heads) {
    
    char *return_type = strtok(header, " ");
    char *function_name = strtok(NULL, "(");


    FunctionNode *new_func = malloc(sizeof(FunctionNode));
    new_func->num_lines = 0;
    new_func->num_variables = 0;
    strcpy(new_func->function_name, function_name);
    new_func->next = NULL;

    insertFuncNode(heads, new_func);

    return new_func;

}

/**
 * Inserts node into linked list whose head is head
 * 
 */
void insertMemNode(MemNode *head, MemNode *node) {

    if (head == NULL) {
        head = node;
    }
    else {
        MemNode *p = head;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = node;
    }
    
}

/**
 * Returns true iff line contains a variable declaration
 * and inserts the variable into the appropriate linked list
 * if a variable is contained
 * 
 */
bool isVar(char *line, char **types, int num_types, FunctionNode *curr_func, MemNode *head) {
    
    bool contains_var = false;

    // empty string
    if (strlen(line) == 0) {
        return false;
    }
    
    char line_copy[1024]; // copy of line for tokenization
    strcpy(line_copy, line);
    char *type = strtok(line_copy, " ");

    strcat(type, " ");
    
    for (int i = 0; i < num_types; i++) {
        if (strcmp(type, types[i]) == 0) {
            contains_var = true;
            break;
        }
    }

    if (contains_var) {
        // modify type field
        if (strstr(line, "[") && strstr(line, "]")) { // if variable is array
            strcat(type, "[]");
        }
        else { // remove added space
            type[strlen(type) - 1] = '\0';
        }


        // construct node
        MemNode *new_var = malloc(sizeof(MemNode));

        strcpy(new_var->type, type);
        if (curr_func == NULL) { // if variable is global
            strcpy(new_var->scope, "global");
        }
        else {
            strcpy(new_var->scope, curr_func->function_name);
        }
        insertMemNode(head, new_var);
    }
    

    return contains_var;

}


/**
 * Iterates and reads through lines of source file.
 * Returns 0 iff there is a valid file indicated by user
 * 
 */
int readFile(Stats *stats, int argc, char **argv, LLHeads *heads) {
    FILE *src_file; // pointer to file with source code
    src_file = fopen(argv[1], "r");

    // no file name given
    if (argc == 1) {
        printf("Please enter the name of the file containing the source code.\n");
        return 1;
    }
    // file does not exist
    if (src_file == NULL) {
        printf("%s does not exist.\n", argv[1]);
        return 1;
    }
    else {
        char *types[7] = {"void ", "int ", "float ", "char ", "int* ", "float* ", "char* "};
        int num_types = 7;

        bool reading_function = false;
        FunctionNode *curr_func = NULL;

        bool is_global_var;

        char line[1024]; // holds one line at a time verbatim from source code
        while (fgets(line, sizeof(line), src_file) != NULL) {    
            // ignore comments
            if (strncmp(line, "//", 2) == 0) { // if whole line is a comment
                strcpy(line, "");
            }
            else {
                strcpy(line, strtok(line, "//"));
            }


            if (!reading_function) {
                reading_function = isFunctionHeader(line, types, num_types);
                if (reading_function == true) {
                    curr_func = initFunction(line, heads);
                    stats->num_functions++; // increment num_functions
                }
                else {
                    // global variables
                    is_global_var = isVar(line, types, num_types, curr_func, heads->static_head);
                }
            }
            else {
                if (!strstr(line, "{") && !strstr(line, "}")) {
                    curr_func->num_lines++;
                }
                
                if (isVar(line, types, num_types, curr_func, heads->static_head) == true) {
                    curr_func->num_variables++;
                }

                // end of function reached
                if (strstr(line, "}")) {
                    reading_function = false; 
                    curr_func = NULL;
                }
            }

            stats->num_lines++;
            
        }

        fclose(src_file); // close file
        return 0;
    }
}

/**
 * Prints the output given information read from source file
 * 
 */
void printOutput(Stats *stats, LLHeads *heads) {
    printf(">>> Memory Model Layout <<<\n");

    printf("***  exec // text ***\n");
    printf("   %s\n", stats->program_name);

    printf("\n### ROData ###       scope  type  size\n");
    // string_literals

    printf("\n### static data ###\n");
    // global_variables

    printf("\n### heap ###\n");
    // heap variables

    printf("\n####################\n### unused space ###\n####################\n");

    printf("\n### stack ###\n");
    // stack variables
    
    printf("\n**** STATS ****\n");
    printf("  - Total number of lines in the file: %d\n", stats->num_lines);
    
    FunctionNode *p = NULL; // pointer for traversal
    
    printf("  - Total number of functions: %d\n    ", stats->num_functions);
    p = heads->func_head;
    while (p != NULL) {
        if (p->next != NULL) {
            printf("%s, ", p->function_name);
        }
        else {
            printf("%s\n", p->function_name); // last function name to print
        }
        p = p->next;
    }

    printf("  - Total number of lines per functions:\n");
    p = heads->func_head;
    while (p != NULL) {
        printf("    %s: %d\n", p->function_name, p->num_lines);
        p = p->next;
    }

    printf("  - Total number of variables per function:\n");
    p = heads->func_head;
    while (p != NULL) {
        printf("    %s: %d\n", p->function_name, p->num_variables);
        p = p->next;
    }

    printf("//////////////////////////////\n");
}

int main(int argc, char **argv) {
    Stats *stats = malloc(sizeof(Stats));
    LLHeads *heads = malloc(sizeof(heads));

    // Initialize fields of stats
    if (argv[1] != NULL)
        strcpy(stats->program_name, argv[1]);
    stats->num_lines = 0;
    stats->num_functions = 0;

    heads->func_head = NULL;
    heads->stack_head = NULL;

    // Call functions
    int validFile = readFile(stats, argc, argv, heads);
    if (validFile != 0) {
        return validFile;
    }
    

    printOutput(stats, heads);

    free(stats); // free stats struct
    free(heads); // free linked list heads

    return 0;
}
