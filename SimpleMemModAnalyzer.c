#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// struct to hold STATS information
typedef struct StatsStruct {
    char program_name[1024];
    int num_lines; // number of lines in source code
    int num_functions; // number of functions in source code
} Stats;

// struct to hold function information
typedef struct FunctionStruct {
    char function_name[1024];
    int num_lines;
    int num_variables;
} Function;

/**
 * Iterates and reads through lines of source file.
 * Returns 0 iff there is a valid file indicated by user
 */
int readFile(Stats *stats, int argc, char **argv) {
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
        char line[1024];
        while (fgets(line, sizeof(line), src_file) != NULL) {
            stats->num_lines++;
            
        }

        fclose(src_file); // close file
        return 0;
    }
}

/**
 * Prints the output given information read from source file
 */
void printOutput(Stats *stats) {
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
}

int main(int argc, char **argv) {
    Stats *stats = malloc(sizeof(Stats));
    // Initialize fields of stats
    if (argv[1] != NULL)
        strcpy(stats->program_name, argv[1]);
    stats->num_lines = 0;
    stats->num_functions = 0;

    // Call functions
    int validFile = readFile(stats, argc, argv);
    if (validFile != 0) {
        return validFile;
    }
    

    printOutput(stats);

    free(stats); // free struct
    return 0;
}
