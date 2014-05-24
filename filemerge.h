#include<stdio.h>
unsigned min(unsigned a, unsigned b);
typedef int elemtype;
#define CACHELINE_SIZE  8 / sizeof(elemtype)
typedef struct BufferedReader{
    FILE* in;
    unsigned elements_to_read;
    elemtype buff[CACHELINE_SIZE];
    unsigned elems_in_buff;
    elemtype* buffptr;
} BufferedReader;
int isempty(BufferedReader *m);
elemtype get(BufferedReader* m);
elemtype front(BufferedReader* m);
typedef struct BufferedWriter{
    FILE* out;
    elemtype buff[CACHELINE_SIZE];
    unsigned elems_in_buff;
} BufferedWriter;
void flush(BufferedWriter *w);
void put(BufferedWriter *w, elemtype v);
int initBR(BufferedWriter *r, char *s);
int initBW(BufferedWriter *w, char *s);
void close(void *m);
