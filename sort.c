#include"sort.h"
#include<math.h>
unsigned cacheline;
unsigned childK;
typedef struct Funnel{
    void* out;
    struct Funnel** child;
    size_t size;
    size_t nmemb;
    //static cmp_t cmp;
} Funnel;
cmp_t funnel_comparator;
Funnel* funnel_create(void *ptr, size_t nmemb, size_t size){
    struct Funnel* f = malloc(sizeof(Funnel));
    f->out = malloc(size*nmemb);
    f->size = size;
    f->nmemb = nmemb;
    if(nmemb <= cacheline || nmemb < childK){
        f->child = 0;
        memcpy(f->out, ptr, size*nmemb);
        qsort(f->out, nmemb, size, funnel_comparator);
        return f;
    }
    f->child = malloc(childK*sizeof(Funnel*));
    unsigned i;
    for(i = 0; i < childK; ++i){
        size_t child_n = nmemb/(childK-i);
        f->child[i] = funnel_create(ptr, child_n, size);
        ptr += child_n*size;
        nmemb -= child_n;
    }
    return f;
}
void funnel_fill(struct Funnel* f){
    if(f->child == 0)
        return;
    unsigned i;
    void *ptrout = f->out;
    void *ptr[childK];
    for(i = 0; i < childK; i++){
        ptr[i] = f->child[i]->out;
    }
    unsigned heap[childK+1], heap_n;
    for(heap_n = 1; heap_n <= childK; heap_n++){
        heap[heap_n] = heap_n-1;
        unsigned j = heap_n;
        while(j!=1 && funnel_comparator(ptr[heap[j]], ptr[heap[j>>1]]) < 0){
            unsigned tmp = heap[j];
            heap[j] = heap[j>>1];
            heap[j>>=1] = tmp;
        }
    }
    while(heap_n > 1){
        memcpy(ptrout, ptr[heap[1]], f->size);
        ptrout += f->size;
        ptr[heap[1]] += f->size;
        if(ptr[heap[1]] == f->child[heap[1]]->out + f->size * f->child[heap[1]]->nmemb)
            heap[1] = heap[--heap_n];
        unsigned j = 1;
        while((j << 1) < heap_n){
            unsigned minch = (j << 1);
            if(minch + 1 != heap_n && funnel_comparator(ptr[heap[minch+1]], ptr[heap[minch]]) < 0)
                minch++;
            if(funnel_comparator(ptr[heap[minch]], ptr[heap[j]]) < 0){
                unsigned tmp = heap[j];
                heap[j] = heap[minch];
                heap[minch] = tmp;
                j = minch;
            }
            else j = heap_n;
        }
    }
}
void funnel_push(Funnel *f){
    if(f->child){
        unsigned i;
        for(i = 0; i < childK; i++)
            funnel_push(f->child[i]);
        funnel_fill(f);
    }
}
void sort(void *ptr, size_t nmemb, size_t size, cmp_t cmp){
    cacheline = 64;
    childK = pow(nmemb, 1./3)+1;
    funnel_comparator = cmp;
    struct Funnel *f;
    f = funnel_create(ptr, nmemb, size);
    funnel_push(f);
    memcpy(ptr, f->out, size*nmemb);
}
