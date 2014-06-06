#include "sort.h"
int cmp(const void* e1, const void* e2){
    return *((int*)e1) - *((int*)e2);
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    if(argc > 2)
        sort(argv[1], argv[2], sizeof(int), cmp);
    else
        if(argc > 1)
            sort(argv[1], argv[1], sizeof(int), cmp);
        else
            printf("Usage: sort infile outfile\nor sort file - result in inputfile");
    return 0;
}
