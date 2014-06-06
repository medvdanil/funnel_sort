#include"sort.h"
#include<math.h>
#define BUFFER_SIZE 64*1024
inline unsigned min(unsigned a, unsigned b){
    return a < b ? a : b;
}
typedef struct statFun{
    size_t size;
    cmp_t comparator;
    unsigned childK;
    unsigned buffer_size;
    char temp_filename[20];
    void *global_buff;
} statFun;
typedef struct Funnel{
    union{
        struct Funnel** child;
        FILE* fin;
    };
    unsigned nmemb;
    statFun *static_params;
    void* buffer;
    void* buffptr;
    unsigned buffn;
} Funnel;

Funnel* funnel_create(const char *filename, size_t offset, unsigned nmemb, statFun* fun_params){
    struct Funnel* f = malloc(sizeof(Funnel));
    f->nmemb = nmemb;
    f->static_params = fun_params;
    f->buffn = 0;
    if(nmemb <= f->static_params->buffer_size * f->static_params->childK){
        f->child = 0;
        f->buffer = 0;
        f->buffptr = 0;
        FILE* r = fopen(filename, "rb");
        fseek(r, offset, SEEK_SET);
        f->nmemb = fread(f->static_params->global_buff, f->static_params->size, nmemb, r);
        fclose(r);
        qsort(f->static_params->global_buff, f->nmemb, f->static_params->size, f->static_params->comparator);
        FILE* w = fopen(f->static_params->temp_filename, "ab+");
        fwrite(f->static_params->global_buff, f->static_params->size, f->nmemb, w);
        fclose(w);
        return f;
    }
    f->child = malloc(f->static_params->childK*sizeof(Funnel*));
    unsigned i;
    for(i = 0; i < f->static_params->childK; ++i){
        unsigned child_n = nmemb/(f->static_params->childK-i);
        f->child[i] = funnel_create(filename, offset, child_n, fun_params);
        offset += child_n * f->static_params->size;
        nmemb -= child_n;
    }
    f->buffer = malloc(f->static_params->buffer_size * f->static_params->size);
    f->buffptr = f->buffer + f->static_params->buffer_size * f->static_params->size;
    return f;
}

void funnel_openPump(struct Funnel* f, size_t offset){
    if(!f->buffer){
        f->fin = fopen(f->static_params->temp_filename, "rb");
        fseek(f->fin, offset, SEEK_SET);
    }
    else{
        unsigned i;
        unsigned nmemb = f->nmemb;
        for(i = 0; i < f->static_params->childK; ++i){
            unsigned child_n = nmemb/(f->static_params->childK-i);
            funnel_openPump(f->child[i], offset);
            offset += child_n * f->static_params->size;
            nmemb -= child_n;
        }
        return;
    }
}

unsigned funnel_fill(struct Funnel* f){
    if(f->buffer >= f->static_params->global_buff &&
            f->buffer < f->static_params->global_buff +
            f->static_params->size* f->static_params->buffer_size * f->static_params->childK){
        f->buffn = fread(f->buffer, f->static_params->size,
              min(f->static_params->buffer_size, f->nmemb), f->fin);
        f->buffptr = f->buffer;
        f->nmemb -= f->buffn;
        return f->buffn;
    }
    else{
        unsigned i;
        for(i = 0; i < f->static_params->childK; i++)
            if(!f->child[i]->buffn){
                if(!f->child[i]->buffer)
                    f->child[i]->buffer = f->static_params->global_buff + i * f->static_params->buffer_size * f->static_params->size;
                funnel_fill(f->child[i]);
            }
        unsigned heap[f->static_params->childK+1], heap_n = 1;
        for(i = 0; i < f->static_params->childK; i++)
            if(f->child[i]->buffn){
                heap[heap_n] = i;
                unsigned j = heap_n++;
                while(j!=1 && f->static_params->comparator(f->child[heap[j]]->buffptr, f->child[heap[j>>1]]->buffptr) < 0){
                    unsigned tmp = heap[j];
                    heap[j] = heap[j>>1];
                    heap[j>>=1] = tmp;
                }
            }
        f->buffptr = f->buffer;
        f->buffn = 0;
        while(heap_n > 1 && f->buffptr != f->buffer + f->static_params->buffer_size * f->static_params->size){
            memcpy(f->buffptr, f->child[heap[1]]->buffptr, f->static_params->size);
            f->buffptr += f->static_params->size;
            f->buffn++;
            f->child[heap[1]]->buffptr += f->static_params->size;
            f->child[heap[1]]->buffn--;
            if(f->child[heap[1]]->buffn == 0)
                heap[1] = heap[--heap_n];
            unsigned j = 1;
            while((j << 1) < heap_n){
                unsigned minch = (j << 1);
                if(minch + 1 != heap_n && f->static_params->comparator(f->child[heap[minch+1]]->buffptr, f->child[heap[minch]]->buffptr) < 0)
                    minch++;
                if(f->static_params->comparator(f->child[heap[minch]]->buffptr, f->child[heap[j]]->buffptr) < 0){
                    unsigned tmp = heap[j];
                    heap[j] = heap[minch];
                    heap[minch] = tmp;
                    j = minch;
                }
                else j = heap_n;
            }
        }
        for(i = 0; i < f->static_params->childK; i++)
            if(f->child[i]->buffn && (f->child[i]->buffer >= f->static_params->global_buff &&
                                       f->child[i]->buffer < f->static_params->global_buff +
                                       f->static_params->size* f->static_params->buffer_size * f->static_params->childK)){
                fseek(f->child[i]->fin, -f->child[i]->buffn * f->static_params->size, SEEK_CUR);
                f->child[i]->nmemb += f->child[i]->buffn;
                f->child[i]->buffn = 0;
                f->child[i]->buffer = 0;
            }
        f->buffptr = f->buffer;
        return f->buffn;
    }
}

void funnel_push(Funnel *f, const char* outfile){
    if(!f->buffer)
        f->buffer = f->static_params->global_buff;
    FILE* out = fopen(outfile, "wb");
    while(funnel_fill(f)){
        fwrite(f->buffer, f->static_params->size, f->buffn, out);
        f->buffptr = f->buffer;
        f->buffn = 0;
    }
    fclose(out);
}

void funnel_delete(Funnel *f){
    if(f->buffer == 0 || (f->buffer >= f->static_params->global_buff &&
            f->buffer < f->static_params->global_buff +
            f->static_params->size* f->static_params->buffer_size * f->static_params->childK)){
        fclose(f->fin);
    }
    else {
        unsigned i;
        for(i = 0; i < f->static_params->childK; i++)
            funnel_delete(f->child[i]);
        free(f->child);
        free(f->buffer);
    }
    free(f);
}

void sort(const char *filename, const char* outfile, size_t size, cmp_t cmp){
    FILE* fn = fopen(filename, "rb");
    fseek(fn, 0, SEEK_END);
    size_t nmemb = ftell(fn) / size;
    fclose(fn);

    statFun *fun_params = malloc(sizeof(statFun));
    fun_params->size = size;
    fun_params->buffer_size = BUFFER_SIZE / size;
    if( fun_params->buffer_size == 0)
        fun_params->buffer_size = 1;
    fun_params->childK = pow(nmemb, 1./3)+1;
    fun_params->comparator = cmp;
    fun_params->global_buff =  malloc(fun_params->size * fun_params->buffer_size * fun_params->childK);
    memcpy(fun_params->temp_filename, filename, 4);
    sprintf(fun_params->temp_filename+4, "_fstf0x%x.tmp", (fun_params - ((statFun*)0)) & 0xffff);

    struct Funnel *f;
    f = funnel_create(filename, 0, nmemb, fun_params);
    funnel_openPump(f, 0);

    funnel_push(f, outfile);

    funnel_delete(f);
    remove(fun_params->temp_filename);
    free(fun_params->global_buff);
    free(fun_params);
}
