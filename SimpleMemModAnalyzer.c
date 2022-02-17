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
void insertFuncNode(FunctionNode *func_head, FunctionNode *node) {

    if (func_head == NULL) {
        func_head = node;
    }
    else {
        FunctionNode *p = func_head;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = node;
    }
    
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
 * Deletes a FunctionNode linked list whose head is head
 * 
 */
void deleteFunctionList(FunctionNode *head) {
    FunctionNode *p = head;
    FunctionNode *q = NULL;

    while (p != NULL) {
        q = p->next;
        free(p);
        p = q;
    }
}


/**
 * Deletes a MemNode linked list whose head is head
 * 
 */
void deleteMemList(MemNode *head) {
    MemNode *p = head;
    MemNode *q = NULL;

    while (p != NULL) {
        q = p->next;
        free(p);
        p = q;
    }
}

/**
 * Returns the size of memory occupied by a type
 * 
 */
int getSize(char *type, int num_elements) {
    int size_per_element;
    if (strcmp(type, "int") == 0 || strcmp(type, "int []") == 0 || strcmp(type, "int[]") == 0) {
        size_per_element = sizeof(int);
    }
    else if (strcmp(type, "float") == 0 || strcmp(type, "float []") == 0 || strcmp(type, "float[]") == 0) {
        size_per_element = sizeof(float);
    }
    else if (strcmp(type, "char") == 0 || strcmp(type, "char []") == 0 || strcmp(type, "char[]") == 0) {
        size_per_element = sizeof(char);
    }
    else if (strcmp(type, "int *") == 0 || strcmp(type, "int*") == 0) {
        size_per_element = sizeof(int*);
    }
    else if (strcmp(type, "float *") == 0 || strcmp(type, "float*") == 0) {
        size_per_element = sizeof(float*);
    }
    else if (strcmp(type, "char *") == 0 || strcmp(type, "char*") == 0) {
        size_per_element = sizeof(char*);
    }
    else if (strcmp(type, "int **") == 0 || strcmp(type, "int**") == 0) {
        size_per_element = sizeof(int**);
    }
    else if (strcmp(type, "float **") == 0 || strcmp(type, "float**") == 0) {
        size_per_element = sizeof(float**);
    }
    else if (strcmp(type, "char **") == 0 || strcmp(type, "char**") == 0) {
        size_per_element = sizeof(char**);
    }

    return size_per_element * num_elements;
}

/**
 * Returns true iff str is whitespace
 * 
 */
bool isWhitespace(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        if (!isspace(str[i])) {
            return false;
        }
        i++;
    }
    return true;
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
 * Returns true iff line contains a variable declaration
 * and inserts the variable into the appropriate linked list
 * if a variable is contained
 * 
 */
bool isVar(char *line, char **types, int num_types, FunctionNode *curr_func, 
           MemNode *ro_head, MemNode *static_head, MemNode *heap_head, MemNode *stack_head) {
    
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
        char line_copy_var[1024]; // copy of line for tokenization
        strcpy(line_copy_var, line);

        // remove spaces at beginning
        char *ptr = line_copy_var;
        while (isspace(*ptr)) {
            ptr++;
        }


        char *next_var = strchr(ptr, ' ');
        char *var_name = strtok(next_var, ",;");
        int num_elements = 1;
        
        while (var_name != NULL) {
            
            // remove spaces at beginning
            while (isspace(*var_name)) {
                var_name++;
            }
            // cut out equals, square brackets and spaces
            char var_tmp[1024];
            int i = 0;
            while (i < strlen(var_name)) {
                if (var_name[i] == ' ' || var_name[i] == '=' || var_name[i] == '[') {
                    var_tmp[i] = '\0';
                    break;
                }
                
                var_tmp[i] = var_name[i];
                i++;
            }
            var_tmp[i] = '\0';
            var_name = var_tmp;


            if (strcmp(var_name, "") != 0) {
                // modify type field
                if (strstr(line, "[") && strstr(line, "]")) { // if variable is array
                    strcat(type, "[]");
                    char num_elements_str[100];
                    char *idx = strchr(line, '[') + 1;
                    int i = 0;
                    while (*idx != ']') {
                        num_elements_str[i] = *idx;
                        i++;
                        idx++;
                    }
                    num_elements_str[i] = '\0';
                    num_elements = atoi(num_elements_str);
                }
                if (var_name[0] == '*') {
                    strcat(type, "*");
                    var_name++;
                }
                else if (type[strlen(type) - 1] == ' '){ // remove added space
                    type[strlen(type) - 1] = '\0';
                }
                

                // construct node
                MemNode *new_var = malloc(sizeof(MemNode));

                strcpy(new_var->type, type);
                strcpy(new_var->var_name, var_name);
                new_var->size = getSize(type, num_elements);
                new_var->next = NULL;
                
                if (curr_func == NULL) { // if variable is global
                    strcpy(new_var->scope, "global");
                    insertMemNode(static_head, new_var);
                }
                else {
                    curr_func->num_variables++;

                    strcpy(new_var->scope, curr_func->function_name);

                    if (strstr(line, "malloc")) {
                        char line_malloc[1024];
                        // strcpy(line_malloc, line);
                        // char *token = strtok(line_malloc, "(");
                        // char *size_str = strtok(line_malloc, ")");
                        // num_elements = atoi(size_str);
                        insertMemNode(heap_head, new_var);
                    }
                    else if (strstr(line, "= \"")) {
                        insertMemNode(ro_head, new_var);
                    }
                    else  {
                        insertMemNode(stack_head, new_var);
                    }
                }
            }
            var_name = strtok(NULL, ",;");
        }
        
    }

    return contains_var;

}


/**
 * Initializes a FunctionLL node for the current function
 * whose header is header and inserts the new node in the linked
 * list whose head is head
 * 
 */
FunctionNode *initFunction(char *header, FunctionNode *func_head, MemNode *stack_head) {
    
    // copy of header (for later)
    char header_copy[1024];
    strcpy(header_copy, header);

    char *return_type = strtok(header, " ");
    char *function_name = strtok(NULL, "(");


    FunctionNode *new_func = malloc(sizeof(FunctionNode));
    new_func->num_lines = 0;
    new_func->num_variables = 0;
    strcpy(new_func->function_name, function_name);
    new_func->next = NULL;

    insertFuncNode(func_head, new_func);
    
    // keep track of paramaters as variables
    char *params = strtok(header_copy, "(");
    params = strtok(NULL, ")");
    params = strtok(params, " ");

    char *type;
    char *param_name;
    char param_name_str[1024];
    int num_elements;
    while (params != NULL && !isWhitespace(params)) {
        num_elements = 1;

        if (params[0] == ' ') {
            params++; // remove first character if it is a space
        }
        type = params;
        param_name = strtok(NULL, ",)");

        if (param_name[0] == '*' && param_name[1] == '*') {
            param_name += 2;
            strcpy(param_name_str, param_name);
            strcat(type, " **");
        }
        else if (param_name[0] == '*') {
            param_name++;
            strcpy(param_name_str, param_name);
            strcat(type, " *");
            printf("%s\n", param_name);
        }

        // construct node
        MemNode *new_var = malloc(sizeof(MemNode));
        strcpy(new_var->type, type);
        strcpy(new_var->var_name, param_name_str);
        strcpy(new_var->scope, new_func->function_name);
        new_var->size = getSize(type, num_elements);
        new_var->next = NULL;
        insertMemNode(stack_head, new_var);
        new_func->num_variables++;

        params = strtok(NULL, " ");
    }

    return new_func;
}


/**
 * Iterates and reads through lines of source file.
 * Returns 0 iff there is a valid file indicated by user
 * 
 */
int readFile(Stats *stats, int argc, char **argv, FunctionNode *func_head, MemNode *ro_head, MemNode *static_head, MemNode *heap_head, MemNode *stack_head) {
    

    FILE *src_file; // pointer to file with source code
    src_file = fopen(argv[1], "r");

    // no file name given
    if (argc == 1) {
        fprintf(stderr, "Please enter the name of the file containing the source code.\n");
        return 1;
    }
    // file does not exist
    if (src_file == NULL) {
        fprintf(stderr, "%s does not exist.\n", argv[1]);
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
                    curr_func = initFunction(line, func_head, stack_head);
                    stats->num_functions++; // increment num_functions
                }
                else {
                    // global variables
                    is_global_var = isVar(line, types, num_types, curr_func, ro_head, static_head, heap_head, stack_head);
                }
            }
            else {
                if (!strstr(line, "{") && !strstr(line, "}")) {
                    curr_func->num_lines++;
                }
                
                // end of function reached
                if (strstr(line, "}")) {
                    reading_function = false; 
                    curr_func = NULL;
                }

                isVar(line, types, num_types, curr_func, ro_head, static_head, heap_head, stack_head);
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
void printOutput(Stats *stats, FunctionNode *func_head, MemNode *ro_head, MemNode *static_head, 
                 MemNode *heap_head, MemNode *stack_head) {
    
    // free heads of linked lists
    FunctionNode *temp_f = func_head;
    func_head = func_head->next;
    free(temp_f);

    MemNode *temp = ro_head;
    ro_head = ro_head->next;
    free(temp);

    temp = static_head;
    static_head = static_head->next;
    free(temp);

    temp = heap_head;
    heap_head = heap_head->next;
    free(temp);

    temp = stack_head;
    stack_head = stack_head->next;
    free(temp);
    //

    printf(">>> Memory Model Layout <<<\n");

    printf("***  exec // text ***\n");
    printf("   %s\n", stats->program_name);

    MemNode *mp = NULL; // traversal pointer

    printf("\n### ROData ###       scope  type  size\n");
    // string_literals
    mp = ro_head;
    while (mp != NULL) {
        printf("   %s   %s   %s   %d\n", mp->var_name, mp->scope, mp->type, mp->size);
        mp = mp->next;
    }

    printf("\n### static data ###\n");
    // global_variables
    mp = static_head;
    while (mp != NULL) {
        printf("   %s   %s   %s   %d\n", mp->var_name, mp->scope, mp->type, mp->size);
        mp = mp->next;
    }

    printf("\n### heap ###\n");
    // heap variables
    mp = heap_head;
    while (mp != NULL) {
        printf("   %s   %s   %s   %d\n", mp->var_name, mp->scope, mp->type, mp->size);
        mp = mp->next;
    }

    printf("\n####################\n### unused space ###\n####################\n");

    printf("\n### stack ###\n");
    // stack variables
    mp = stack_head;
    while (mp != NULL) {
        printf("   %s   %s   %s   %d\n", mp->var_name, mp->scope, mp->type, mp->size);
        mp = mp->next;
    }

    printf("\n**** STATS ****\n");
    printf("  - Total number of lines in the file: %d\n", stats->num_lines);
    
    FunctionNode *p = NULL; // pointer for traversal
    
    printf("  - Total number of functions: %d\n    ", stats->num_functions);
    p = func_head;
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
    p = func_head;
    while (p != NULL) {
        printf("    %s: %d\n", p->function_name, p->num_lines);
        p = p->next;
    }

    printf("  - Total number of variables per function:\n");
    p = func_head;
    while (p != NULL) {
        printf("    %s: %d\n", p->function_name, p->num_variables);
        p = p->next;
    }

    printf("//////////////////////////////\n");

    deleteFunctionList(func_head);
    deleteMemList(ro_head);
    deleteMemList(static_head);
    deleteMemList(heap_head);
    deleteMemList(stack_head);

}

int main(int argc, char **argv) {
    Stats *stats = malloc(sizeof(Stats));
    FunctionNode *func_head = malloc(sizeof(MemNode));
    MemNode *ro_head = malloc(sizeof(MemNode));
    MemNode *static_head = malloc(sizeof(MemNode));
    MemNode *heap_head = malloc(sizeof(MemNode));
    MemNode *stack_head = malloc(sizeof(MemNode));

    // Initialize fields of stats
    if (argv[1] != NULL)
        strcpy(stats->program_name, argv[1]);
    stats->num_lines = 0;
    stats->num_functions = 0;

    // Call functions
    int validFile = readFile(stats, argc, argv, func_head, ro_head, static_head, heap_head, stack_head);
    if (validFile != 0) {
        return validFile;
    }

    printOutput(stats, func_head, ro_head, static_head, heap_head, stack_head);

    free(stats); // free stats struct
    
    return 0;
}
