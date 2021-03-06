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
    char size[1024];
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
    if (strstr(type, "int") && !strstr(type, "*")) {
        size_per_element = sizeof(int);
    }
    else if (strstr(type, "float") && !strstr(type, "*")) {
        size_per_element = sizeof(float);
    }
    else if (strstr(type, "char") && !strstr(type, "*")) {
        size_per_element = sizeof(char);
    }
    else if (strstr(type, "int") && strstr(type, "*")) {
        size_per_element = sizeof(int*);
    }
    else if (strstr(type, "float") && strstr(type, "*")) {
        size_per_element = sizeof(float*);
    }
    else if (strstr(type, "char") && strstr(type, "*")) {
        size_per_element = sizeof(char*);
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
    char type_word2[1024];
    strcpy(type_word2, "");

    bool is_static = false;

    if (strcmp(type, "const") == 0 || strcmp(type, "static") == 0) {
        
        if (strcmp(type, "static") == 0) {
            is_static = true;
        }
        
        strcpy(type_word2, " ");
        strcat(type_word2, strtok(NULL, " "));
        strcat(type, type_word2);
        strcat(type_word2, " "); 
        
    }

    for (int i = 0; i < num_types; i++) {
        char curr_type[10];
        strcpy(curr_type, types[i]);
        curr_type[strlen(curr_type)-1] = '\0';
        
        // remove spaces in beginning and end of type_word2
        
        char type_word2_no_spaces[1024];
        if (type_word2[0] == ' ') { // if space in beginning
            strcpy(type_word2_no_spaces, &type_word2[1]);
        }
        else {
            strcpy(type_word2_no_spaces, type_word2);
        }
        // if space at end
        if (type_word2_no_spaces[strlen(type_word2_no_spaces) - 1] == ' ') {
            type_word2_no_spaces[strlen(type_word2_no_spaces) - 1] = '\0';
        }

        // comparisons between read type and accepted types
        if ((strlen(type_word2_no_spaces) == 0) && (strcmp(type, curr_type) == 0)) {
            contains_var = true;
            break;
        }
        else if ((strlen(type_word2_no_spaces) != 0) && (strcmp(type_word2_no_spaces, curr_type) == 0)) {
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

        char *var_name = strtok(NULL, ",;");
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
                char num_elements_not_numeric[1024];
                strcpy(num_elements_not_numeric, "");
                
                // if array elements initialized in "type arr[] = {_, _, ... , _};" form
                if (strstr(line, "[") && strstr(line, "]") && strstr(line, "{")) {
                    if (strstr(line, "[]")) {
                        strcat(type, "[]");
                        int num_commas = 0;
                        char *idx = strchr(line, '{');
                        while (*idx != '}') {
                            if (*idx == ',') {
                                num_commas++;
                            }
                            idx++;
                        }
                        num_elements = num_commas + 1;
                    }
                    else  {
                        bool is_numeric = true;
                        char num_elements_str[100];
                        strcat(type, "[");
                        
                        int i = 0;
                        char *idx = strchr(line, '[') + 1;
                        while (*idx != ']') {
                            num_elements_str[i] = *idx;
                            if (!isdigit(num_elements_str[i])) {
                                is_numeric = false;
                            }
                            i++;
                            idx++;
                        }
                        num_elements_str[i] = '\0';
                        strcat(type, num_elements_str);
                        strcat(type, "]");
                        if (is_numeric) {
                            num_elements = atoi(num_elements_str);
                        }
                        else {
                            num_elements = -1;
                            strcpy(num_elements_not_numeric, num_elements_str);
                        }
                    }
                    
                }

                else if (strstr(line, "[") && strstr(line, "]")) { // if variable is array
                    strcat(type, "[");
                    char num_elements_str[100];
                    char *idx = strchr(line, '[') + 1;
                    int i = 0;
                    bool is_numeric = true;
                    while (*idx != ']') {
                        num_elements_str[i] = *idx;
                        if (!isdigit(num_elements_str[i])) {
                            is_numeric = false;
                        }
                        i++;
                        idx++;
                    }
                    num_elements_str[i] = '\0';
                    strcat(type, num_elements_str);
                    strcat(type, "]");
                    if (is_numeric) {
                        num_elements = atoi(num_elements_str);
                    }
                    else {
                        num_elements = -1;
                        strcpy(num_elements_not_numeric, num_elements_str);
                    }
                }

                // modify type field
                if (strlen(var_name) > 2 && var_name[0] == '*' && var_name[1] == '*') {
                    strcat(type, "**");
                    var_name += 2;
                }
                else if (var_name[0] == '*') {
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
                if (num_elements != -1) {
                    sprintf(new_var->size, "%d", getSize(type, num_elements));
                }
                else {
                    if (strstr(type, "int")) {
                        sprintf(new_var->size, "%s*sizeof(int)", num_elements_not_numeric);
                    }
                    else if (strstr(type, "float")) {
                        sprintf(new_var->size, "%s*sizeof(float)", num_elements_not_numeric);
                    }
                    else if (strstr(type, "char")) {
                        sprintf(new_var->size, "%s*sizeof(char)", num_elements_not_numeric);
                    }
                }
                new_var->next = NULL;
                
                if (curr_func == NULL) { // if variable is global
                    strcpy(new_var->scope, "global");
                    insertMemNode(static_head, new_var);
                }
                else {
                    curr_func->num_variables++;

                    strcpy(new_var->scope, curr_func->function_name);

                    if (strstr(line, "*") && (strstr(line, "= \"") || strstr(line, "=\""))) {
                        // hold variable in stack
                        insertMemNode(stack_head, new_var);

                        MemNode *ro_var = malloc(sizeof(MemNode));
                        memcpy(ro_var, new_var, sizeof(MemNode));
                        // declared as pointer (as opposed to an array)
                        if (strstr(ro_var->type, "*")) {
                            char *start_str_lit = strchr(line, '\"') + 1;
                            int size_str_lit = 0;
                            while (*start_str_lit != '\"') {
                                size_str_lit++;
                                start_str_lit++;
                            }
                            size_str_lit++;
                            sprintf(ro_var->size, "%d", size_str_lit);
                            strcpy(ro_var->type, "literal");
                        }

                        insertMemNode(ro_head, ro_var);
                    }
                    else if (is_static){
                        insertMemNode(static_head, new_var);
                    }
                    else  {
                        insertMemNode(stack_head, new_var);
                    }
                }

                if (strstr(line, "malloc")) {
                    MemNode *new_heap_var = malloc(sizeof(MemNode));
                    sprintf(new_heap_var->var_name, "*%s", var_name);
                    new_heap_var->next = NULL;

                    if (curr_func == NULL) { // if variable is global
                        strcpy(new_heap_var->scope, "global");
                    }
                    else {
                        strcpy(new_heap_var->scope, curr_func->function_name);
                    }

                    char malloc_type[1024];
                    strcpy(malloc_type, new_var->type);
                    if (strstr(malloc_type, "*")) {
                        char *ptr_symbol_pos = strchr(malloc_type, '*');
                        *ptr_symbol_pos = '\0';
                    }
                    strcpy(new_heap_var->type, malloc_type);

                    char *start_param = strchr(line, '(') + 1;
                    char malloc_param[1024];
                    int i = 0;
                    while (*start_param != ';') {
                        malloc_param[i] = *start_param;
                        start_param++;
                        i++;
                    }
                    malloc_param[i-1] = '\0'; // take out last parentheses
                
                    if (strcmp(malloc_param, "sizeof(int)") == 0) {
                        sprintf(new_heap_var->size, "%ld", sizeof(int));
                    }
                    else if (strcmp(malloc_param, "sizeof(float)") == 0) {
                        sprintf(new_heap_var->size, "%ld", sizeof(float));
                    }
                    else if (strcmp(malloc_param, "sizeof(char)") == 0) {
                        sprintf(new_heap_var->size, "%ld", sizeof(char));
                    }
                    else if (strstr(malloc_param, "sizeof(") && strstr(malloc_param, "int") && strstr(malloc_param, "*")) {
                        sprintf(new_heap_var->size, "%ld", sizeof(int*));
                    }
                    else if (strstr(malloc_param, "sizeof(") && strstr(malloc_param, "float") && strstr(malloc_param, "*")) {
                        sprintf(new_heap_var->size, "%ld", sizeof(float*));
                    }
                    else if (strstr(malloc_param, "sizeof(") && strstr(malloc_param, "char") && strstr(malloc_param, "*")) {
                        sprintf(new_heap_var->size, "%ld", sizeof(char*));
                    }
                    else {
                        strcpy(new_heap_var->size, malloc_param);
                    }

                    insertMemNode(heap_head, new_heap_var);
                }
            }
            if (strstr(line, "[") && strstr(line, "]") && strstr(line, "{")) {
                break;
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

    while(function_name[0] == '*') {
        function_name++;
    }

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

        if (strcmp(params, "const") == 0 || strcmp(params, "static") == 0) {
            char type_word2[1024];
            strcpy(type_word2, " ");
            strcat(type_word2, strtok(NULL, " "));
            strcat(type, type_word2);
        }

        param_name = strtok(NULL, ",)");

        if (param_name[0] == '*' && param_name[1] == '*') {
            param_name += 2;
            strcpy(param_name_str, param_name);
            strcat(type, "**");
            
        }
        else if (param_name[0] == '*') {
            param_name++;
            strcpy(param_name_str, param_name);
            strcat(type, "*");
            
        }
        

        // construct node
        MemNode *new_var = malloc(sizeof(MemNode));
        
        if (strlen(param_name_str) == 0)
            strcpy(new_var->var_name, param_name);
        else
            strcpy(new_var->var_name, param_name_str);
        
        char *idx_bracket = strchr(param_name, '[');
        if (idx_bracket) {
            *idx_bracket = '\0';
            strcpy(new_var->var_name, param_name);
            
            idx_bracket++;
            char num_elements_str[100];
            int i = 0;
            bool is_numeric = true;

            strcat(type, "[");
            while (*idx_bracket != ']') {
                num_elements_str[i] = *idx_bracket;
                if (!isdigit(num_elements_str[i])) {
                    is_numeric = false;
                }
                i++;
                idx_bracket++;
            }
            num_elements_str[i] = '\0';
            strcat(type, num_elements_str);
            strcat(type, "]");

            if (is_numeric) {
                num_elements = atoi(num_elements_str);
            }
        }

        strcpy(new_var->type, type);
        strcpy(new_var->scope, new_func->function_name);
        sprintf(new_var->size, "%d", getSize(type, num_elements));
        new_var->next = NULL;

        insertMemNode(stack_head, new_var);
        new_func->num_variables++;

        params = strtok(NULL, " ");
        strcpy(param_name, "");
        strcpy(param_name_str, "");
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
                if (line[0] != '{' && line[0] != '}') {
                    curr_func->num_lines++;
                }
                
                // end of function reached
                if (line[0] == '}') {
                    reading_function = false; 
                    curr_func = NULL;
                }

                bool containsVar = isVar(line, types, num_types, curr_func, ro_head, static_head, heap_head, stack_head);
                if (!containsVar && strstr(line, "malloc") && strstr(line, "=")) {
                    MemNode *new_heap_var = malloc(sizeof(MemNode));
                    new_heap_var->next = NULL;

                    char line_copy[1024];
                    strcpy(line_copy, line);

                    char malloc_type[1024];

                    char *malloc_token = strtok(line_copy, " =");
                    char var_name[1024];
                    strcpy(var_name, malloc_token);

                    MemNode *ptr = stack_head;
                    while(ptr != NULL) {
                        if(strcmp(ptr->var_name, var_name) == 0 && strcmp(ptr->scope, curr_func->function_name) == 0) {
                            sprintf(new_heap_var->var_name, "*%s", var_name);
                            strcpy(malloc_type, ptr->type);
                            strcpy(new_heap_var->scope, ptr->scope);
                            break;
                        }
                        ptr = ptr->next;
                    }

                    if (strstr(malloc_type, "*")) {
                        char *ptr_symbol_pos = strchr(malloc_type, '*');
                        *ptr_symbol_pos = '\0';
                    }
                    strcpy(new_heap_var->type, malloc_type);

                    char *start_param = strchr(line, '(') + 1;
                    char malloc_param[1024];
                    int i = 0;
                    while (*start_param != ';') {
                        malloc_param[i] = *start_param;
                        start_param++;
                        i++;
                    }
                    malloc_param[i-1] = '\0'; // take out last parentheses
                
                    if (strcmp(malloc_param, "sizeof(int)") == 0) {
                        sprintf(new_heap_var->size, "%ld", sizeof(int));
                    }
                    else if (strcmp(malloc_param, "sizeof(float)") == 0) {
                        sprintf(new_heap_var->size, "%ld", sizeof(float));
                    }
                    else if (strcmp(malloc_param, "sizeof(char)") == 0) {
                        sprintf(new_heap_var->size, "%ld", sizeof(char));
                    }
                    else if (strstr(malloc_param, "sizeof(") && strstr(malloc_param, "int") && strstr(malloc_param, "*")) {
                        sprintf(new_heap_var->size, "%ld", sizeof(int*));
                    }
                    else if (strstr(malloc_param, "sizeof(") && strstr(malloc_param, "float") && strstr(malloc_param, "*")) {
                        sprintf(new_heap_var->size, "%ld", sizeof(float*));
                    }
                    else if (strstr(malloc_param, "sizeof(") && strstr(malloc_param, "char") && strstr(malloc_param, "*")) {
                        sprintf(new_heap_var->size, "%ld", sizeof(char*));
                    }
                    else {
                        strcpy(new_heap_var->size, malloc_param);
                    }

                    insertMemNode(heap_head, new_heap_var);
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
        printf("   %s   %s   %s   %s\n", mp->var_name, mp->scope, mp->type, mp->size);
        mp = mp->next;
    }

    printf("\n### static data ###\n");
    // global_variables
    mp = static_head;
    while (mp != NULL) {
        printf("   %s   %s   %s   %s\n", mp->var_name, mp->scope, mp->type, mp->size);
        mp = mp->next;
    }

    printf("\n### heap ###\n");
    // heap variables
    mp = heap_head;
    while (mp != NULL) {
        printf("   %s   %s   %s   %s\n", mp->var_name, mp->scope, mp->type, mp->size);
        mp = mp->next;
    }

    printf("\n####################\n### unused space ###\n####################\n");

    printf("\n### stack ###\n");
    // stack variables
    mp = stack_head;
    while (mp != NULL) {
        printf("   %s   %s   %s   %s\n", mp->var_name, mp->scope, mp->type, mp->size);
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
