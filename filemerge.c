#include<stdio.h>
unsigned min(unsigned a, unsigned b){
    return a < b ? a : b;
}
typedef int elemtype;
#define CACHELINE_SIZE  8 / sizeof(elemtype)
typedef struct BufferedReader{
    FILE* in;
    unsigned elements_to_read;
    elemtype buff[CACHELINE_SIZE];
    unsigned elems_in_buff;
    elemtype* buffptr;
} BufferedReader;
int isempty(BufferedReader *m){
    return m->elems_in_buff == 0 && m->elements_to_read <= 0;
}
elemtype get(BufferedReader* m){
    if(!m->elems_in_buff){
        m->elems_in_buff = fread(m->buff, sizeof(elemtype), min(m->elements_to_read, CACHELINE_SIZE), m->in);
        m->elements_to_read -= m->elems_in_buff;
        m->buffptr = m->buff;
    }
    m->elems_in_buff--;
    return *m->buffptr++;
}
elemtype front(BufferedReader* m){
    if(!m->elems_in_buff){
        m->elems_in_buff = fread(m->buff, sizeof(elemtype), min(m->elements_to_read, CACHELINE_SIZE), m->in);
        m->elements_to_read -= m->elems_in_buff;
        m->buffptr = m->buff;
    }
    return *m->buffptr;
}
typedef struct BufferedWriter{
    FILE* out;
    elemtype buff[CACHELINE_SIZE];
    unsigned elems_in_buff;
} BufferedWriter;
void flush(BufferedWriter *w){
    fwrite(w->buff, sizeof(elemtype), w->elems_in_buff, w->out);
    w->elems_in_buff = 0;
}
void put(BufferedWriter *w, elemtype v){
    w->buff[w->elems_in_buff++] = v;
    if(w->elems_in_buff == CACHELINE_SIZE){
        flush(w);
    }
}
int initBR(BufferedWriter *r, char *s){
    r->in = fopen(s, "wb");
    w->elems_in_buff = 0;
    if(!w->out){
        printf("Can't open out");
        return 0;
    }
    return 1;
}
int initBW(BufferedWriter *w, char *s){
    w->out = fopen(s, "wb");
    w->elems_in_buff = 0;
    if(!w->out){
        printf("Can't open out");
        return 0;
    }
    return 1;
}
void close(void *m){
    fclose(*((FILE**)m));
}
int main(){
    int k = 3;
    BufferedReader m[k];
    unsigned insize[] = {4, 4, 4};
    int i;
    unsigned offset = 0;
    for(i = 0; i < k; i++){
        m[i].in = fopen("in", "rb");
        m[i].elements_to_read = insize[i];
        m[i].elems_in_buff = 0;
        if(!m[i].in){
            printf("Can't open in");
            return 0;
        }
        fseek(m[i].in, offset*sizeof(elemtype), SEEK_SET);
        offset += insize[i];
    }
    BufferedWriter wout;
    initBW(&wout, "out");
    unsigned j;
    for(j = 0; j < offset; j++){
        int mini = -1;
        elemtype next;
        for(i = 0; i < k; i++)
            if(!isempty(m+i) && (mini == -1 || front(m+i) < next))
                mini = i, next = front(m+i);
        put(&wout, get(m+mini));
    }
    flush(&wout);
    close(&wout);
    for(i = 0; i < k; i++){
        close(m+i);
    }
    return 0;
}
