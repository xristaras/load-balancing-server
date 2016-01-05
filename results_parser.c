#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    FILE* stream = fopen(argv[1], "r");

    char line[1024];
    while (fgets(line, 1024, stream))
    {
        char* tmp = strdup(line);
        printf("Time in miliseconds is %s\n", getfield(tmp, 2));
        // NOTE strtok clobbers tmp
        free(tmp);
    }
}

///mongodb examples
//
// in mongo shell: use rr_results
// mongo --eval 'db.rr_results.insert({"rr":"2001"})'
// mongo --eval 'db.rr_results.find()'
