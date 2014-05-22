#include "sort.h"
int cmp(const void* e1, const void* e2){
    return *((int*)e1) - *((int*)e2);
}
int main(int argc, char *argv[])
{
	(void) argc;
        (void) argv;
        int *a;
        int n;
        int i;

    freopen("in.txt", "r", stdin);
    freopen("out.txt", "w", stdout);

        scanf("%d", &n);
        a = malloc((sizeof(int))*n);
        srand((a-((int*)0))+n*3);
        for(i = 0; i < n; i++)
            a[i] = rand();
        sort(a, n, sizeof(int), cmp);
        for(i = 0; i < n; i++)
            printf("%d ", a[i]);

	return 0;
}
