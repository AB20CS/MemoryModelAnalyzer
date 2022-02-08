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
typedef struct FunctionLinkedList {
    char function_name[1024];
    int num_lines;
    int num_variables;
    struct FunctionLinkedList *next;
} FunctionNode;

typedef struct LinkedListHeads {
    FunctionNode *func_head;
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
 * Returns true iff str contains only whitespace characters
 * 
 */
bool isEmpty(char *str) {
    while (*str != '\0') {
        if(!isspace((char)*str)) {
            return false;
        }
        str++;
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
        char *types[10] = {"void ", "int ", "float ", "char ", "int* ", "float* ", "char* ", "int [] ", "float [] ", "char [] "};
        bool reading_function = false;
        FunctionNode *curr_func = NULL;

        char line[1024];
        while (fgets(line, sizeof(line), src_file) != NULL) {
            
            if (!reading_function) {
                reading_function = isFunctionHeader(line, types, 10);
                if (reading_function == true) {
                    curr_func = initFunction(line, heads);
                    stats->num_functions++; // increment num_functions
                }
            }
            else {
                if (!strstr(line, "{") && !strstr(line, "}") && !isEmpty(line)) {
                    printf("-%s\n", line);
                    curr_func->num_lines++;
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

    // Call functions
    int validFile = readFile(stats, argc, argv, heads);
    if (validFile != 0) {
        return validFile;
    }
    

    printOutput(stats, heads);

    free(stats); // free struct
    return 0;
}
