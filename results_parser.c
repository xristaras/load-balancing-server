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
    FILE* ifstream = fopen(argv[1], "r");
    FILE* ofstream = fopen(argv[2], "w");

    char line[1024];
    while (fgets(line, 1024, ifstream))
    {
        char* tmp = strdup(line);
        char* tmp2 = strdup(line);
        char* time = strcat(getfield(tmp, 2), "\n");
        char* status = getfield(tmp2, 4);
        // NOTE strtok clobbers tmp
        if(strcmp(status, "200")==0){
            printf("Time in miliseconds is %s and status is %s\n", time, status);
            fputs(time, ofstream);
        }
        free(tmp);
        free(tmp2);
    }
    fclose(ifstream);
    fclose(ofstream);
}

///mongodb examples
//
// in mongo shell: use rr_results
// mongo --eval 'db.rr_results.insert({"rr":"2001"})'
// mongo --eval 'db.rr_results.find()'
