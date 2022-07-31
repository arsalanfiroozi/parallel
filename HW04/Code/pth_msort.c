// Include your C header files here

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "pth_msort.h"

struct in
{
    int N;
    const int *arr;
    int *out_arr;
    int *t;
};

struct in_m
{
    struct in *s1;
    struct in *s2;
    int *out_arr;
};

struct in_m2
{
    struct in *s1;
    struct in *s2;
    int *out_arr;
    int *out_arr2;
};

struct binary_in
{
    int x;
    int *arr;
    int s;
    int e;
    int index;
};

struct merge_end
{
    int *arr1;
    int *arr2;
    int *result;
    int p1_e;
    int p2_e;
};

void *sort(void *s)
{
    struct in *sin = (struct in *)s;
    int N = sin->N;

    if (sin->N == 1)
    {
        sin->out_arr[0] = sin->arr[0];
        return;
    }

    // struct in s1 = {sin->N / 2, sin->arr, (int *)malloc(N / 2 * sizeof(int))};
    struct in s1 = {sin->N / 2, sin->arr, sin->out_arr, sin->t};
    sort((void *)&s1);
    // struct in s2 = {sin->N / 2, sin->arr + N / 2, (int *)malloc(N / 2 * sizeof(int))};
    struct in s2 = {sin->N / 2, sin->arr + N / 2, sin->out_arr + N / 2, sin->t + N / 2};
    sort((void *)&s2);

    int p1 = 0;
    int p2 = 0;
    // printf("%d\n", N);
    while (p1 + p2 < N)
    {
        if (p1 >= N / 2 || (p2 < N / 2 && s1.out_arr[p1] >= s2.out_arr[p2]))
        {
            sin->t[p1 + p2] = s2.out_arr[p2];
            p2++;
            // printf("%d %d \n", p1, p2);
        }
        else if (p2 >= N / 2 || (p1 < N / 2 && s1.out_arr[p1] < s2.out_arr[p2]))
        {
            sin->t[p1 + p2] = s1.out_arr[p1];
            p1++;
            // printf("%d %d \n", p1, p2);
        }
    }
    int i;
    for (i = 0; i < p1 + p2; i++)
    {
        sin->out_arr[i] = sin->t[i];
    }
    return;
}

void *merge(void *s)
{
    struct in_m *sin = (struct in_m *)s;
    struct in s1 = *(sin->s1);
    struct in s2 = *(sin->s2);
    int N = 2 * s1.N;

    int p1 = 0;
    int p2 = 0;
    // printf("S %d\n", N);
    while (p1 + p2 < N)
    {
        // printf("%d %d \n", s1.out_arr[p1], s2.out_arr[p2]);
        if (p1 >= N / 2 || (p2 < N / 2 && s1.out_arr[p1] >= s2.out_arr[p2]))
        {
            sin->out_arr[p1 + p2] = s2.out_arr[p2];
            p2++;
        }
        else if (p2 >= N / 2 || (p1 < N / 2 && s1.out_arr[p1] < s2.out_arr[p2]))
        {
            sin->out_arr[p1 + p2] = s1.out_arr[p1];
            p1++;
            // printf("%d %d \n", p1, p2);
        }
    }
    return;
}

void *merge2(void *s)
{
    struct in_m2 *sin = (struct in_m2 *)s;
    struct in s1 = *(sin->s1);
    struct in s2 = *(sin->s2);
    int N = 2 * s1.N;

    int p1 = 0;
    int p2 = 0;
    // printf("S %d\n", N);
    while (p1 + p2 < N)
    {
        // printf("%d %d \n", s1.out_arr[p1], s2.out_arr[p2]);
        if (p1 >= N / 2 || (p2 < N / 2 && s1.out_arr[p1] >= s2.out_arr[p2]))
        {
            sin->out_arr[p1 + p2] = p2;
            sin->out_arr2[p1 + p2] = 1;
            p2++;
        }
        else if (p2 >= N / 2 || (p1 < N / 2 && s1.out_arr[p1] < s2.out_arr[p2]))
        {
            sin->out_arr[p1 + p2] = p1;
            sin->out_arr2[p1 + p2] = 0;
            p1++;
            // printf("%d %d \n", p1, p2);
        }
    }
    return;
}

void *bin_search(void *s)
{
    struct binary_in *sin = (struct binary_in *)s;
    int m = (sin->s + sin->e) / 2;
    if ((sin->s == m) || sin->x == sin->arr[m])
    {
        sin->index = m;
        return;
    }
    if (sin->x > sin->arr[m])
    {
        sin->s = m;
        bin_search(s);
    }
    else
    {
        sin->e = m;
        bin_search(s);
    }

    return;
}

void *merge_endf(void *s)
{
    struct merge_end *sin = (struct merge_end *)s;
    int N = sin->p1_e + sin->p2_e + 2;

    int p1 = 0;
    int p2 = 0;
    // printf("S %d\n", N);
    while (p1 + p2 < N)
    {
        // printf("%d %d \n", s1.out_arr[p1], s2.out_arr[p2]);
        // if (p1 >= N / 2 || (p2 < N / 2 && s1.out_arr[p1] >= s2.out_arr[p2]))
        if (p1 > sin->p1_e || (p2 <= sin->p2_e && sin->arr1[p1] >= sin->arr2[p2]))
        {
            sin->result[p1 + p2] = sin->arr2[p2];
            p2++;
        }
        else if (p2 >= sin->p2_e || (p1 <= sin->p1_e && sin->arr1[p1] < sin->arr2[p2]))
        {
            sin->result[p1 + p2] = sin->arr1[p1];
            p1++;
        }
    }
    return;
}

void mergeSortParallel(const int *values, unsigned int N, int *sorted)
{
    int *temp = (int *)malloc(N * sizeof(int));

    // printf("0 \n");

    // Stage 1
    // struct in s1 = {N / 4, values, (int *)malloc(N / 4 * sizeof(int))};
    // struct in s2 = {N / 4, values + N / 4, (int *)malloc(N / 4 * sizeof(int))};
    // struct in s3 = {N / 4, values + N / 2, (int *)malloc(N / 4 * sizeof(int))};
    // struct in s4 = {N / 4, values + 3 * N / 4, (int *)malloc(N / 4 * sizeof(int))};
    struct in s1 = {N / 4, values, sorted, temp};
    struct in s2 = {N / 4, values + N / 4, sorted + N / 4, temp + N / 4};
    struct in s3 = {N / 4, values + N / 2, sorted + N / 2, temp + N / 2};
    struct in s4 = {N / 4, values + 3 * N / 4, sorted + 3 * N / 4, temp + 3 * N / 4};
    pthread_t *thread_handles;
    thread_handles = (pthread_t *)malloc(4 * sizeof(pthread_t));
    pthread_create(&thread_handles[0], (pthread_attr_t *)NULL, sort, (void *)&s1);
    pthread_create(&thread_handles[1], (pthread_attr_t *)NULL, sort, (void *)&s2);
    pthread_create(&thread_handles[2], (pthread_attr_t *)NULL, sort, (void *)&s3);
    pthread_create(&thread_handles[3], (pthread_attr_t *)NULL, sort, (void *)&s4);

    pthread_join(thread_handles[0], NULL);
    pthread_join(thread_handles[1], NULL);
    pthread_join(thread_handles[2], NULL);
    pthread_join(thread_handles[3], NULL);

    // printf("1 \n");

    // Stage 2
    struct in_m s5 = {&s1, &s2, temp};
    struct in_m s6 = {&s3, &s4, temp + N / 2};
    // struct in_m s5 = {&s1, &s2, sorted};
    // struct in_m s6 = {&s3, &s4, sorted + N / 2};
    pthread_create(&thread_handles[0], (pthread_attr_t *)NULL, merge, (void *)&s5);
    pthread_create(&thread_handles[1], (pthread_attr_t *)NULL, merge, (void *)&s6);

    pthread_join(thread_handles[0], NULL);
    pthread_join(thread_handles[1], NULL);

    // printf("2 \n");

    // Stage 3 => Old Approach
    // struct in s7 = {N / 2, values, s5.out_arr};
    // struct in s8 = {N / 2, values, s6.out_arr};
    // struct in_m s9 = {&s7, &s8, sorted};
    // pthread_create(&thread_handles[0], (pthread_attr_t *)NULL, merge, (void *)&s9);

    // pthread_join(thread_handles[0], NULL);

    // sorted[0] = 1000;

    // Parallelism of the Last Node
    int m = 10;
    int *ap = (int *)malloc((m / 2 + 1) * sizeof(int));
    int *bp = (int *)malloc((m / 2 + 1) * sizeof(int));
    int j = 0;
    int i;
    float remainder = 0;
    for (i = 0; i < N / 2; i = i + N / m)
    {
        remainder += ((float)N) / ((float)m) * ((float)j) - ((float)i);
        // printf("%f \n", remainder);
        if (remainder >= 1)
        {
            i += remainder;
            remainder = remainder - (float)((int)remainder);
        }
        // printf("p -> %d \n", i);
        ap[j] = s5.out_arr[i];
        bp[j] = s6.out_arr[i];
        j = j + 1;
    }
    ap[j] = s5.out_arr[N / 2 - 1];
    bp[j] = s6.out_arr[N / 2 - 1];
    j = j + 1;

    // for (i = 0; i < j; i++)
    //     printf("%d %d \n", ap[i], bp[i]);
    // printf("-------------\n");

    // struct in s10 = {j, (const int *)NULL, (int *)ap};
    // struct in s11 = {j, (const int *)NULL, (int *)bp};
    // struct in_m2 s12 = {&s10, &s11, (int *)malloc(2 * j * sizeof(int)), (int *)malloc(2 * j * sizeof(int))};
    // merge2((void *)&s12);

    // printf("%d \n", j);

    // Binary Search => Find indices by 4 threads
    struct binary_in *s13 = (struct binary_in *)malloc(4 * sizeof(struct binary_in));
    for (i = 0; i < 4; i++)
    {
        s13[i].x = ap[i + 1];
        s13[i].arr = s6.out_arr;
        s13[i].s = 0;
        s13[i].e = N / 2 - 1;
        s13[i].index = -1;
    }

    pthread_create(&thread_handles[0], (pthread_attr_t *)NULL, bin_search, (void *)&s13[0]);
    pthread_create(&thread_handles[1], (pthread_attr_t *)NULL, bin_search, (void *)&s13[1]);
    pthread_create(&thread_handles[2], (pthread_attr_t *)NULL, bin_search, (void *)&s13[2]);
    pthread_create(&thread_handles[3], (pthread_attr_t *)NULL, bin_search, (void *)&s13[3]);

    pthread_join(thread_handles[0], NULL);
    pthread_join(thread_handles[1], NULL);
    pthread_join(thread_handles[2], NULL);
    pthread_join(thread_handles[3], NULL);

    // Merge 4 Parts
    struct merge_end *s17 = (struct merge_end *)malloc(4 * sizeof(struct merge_end));
    // s17[1] = {s5.out_arr, s6.out_arr, sorted, p1_e, p2_e};

    j = 0;
    int jj=0;
    int p = 0;
    remainder = 0;
    for (i = 0; i < N / 2; i = i + N / m)
    {
        remainder += ((float)N) / ((float)m) * ((float)j) - ((float)i);
        if (remainder >= 1)
        {
            i += remainder;
            remainder = remainder - (float)((int)remainder);
        }
        if (i != 0)
        {
            // printf("p -> %d \n", p);
            s17[jj].arr1 = s5.out_arr + p;
            s17[jj].p1_e = i - p - 1;
            if (jj != 0)
            {
                s17[jj].arr2 = s6.out_arr + s13[jj - 1].index + 1;
                s17[jj].p2_e = s13[jj].index - s13[jj - 1].index - 1;
            }
            else
            {
                s17[jj].arr2 = s6.out_arr;
                s17[jj].p2_e = s13[jj].index;
            }
            jj++;
        }
        j++;
        if (jj < 4)
            p = i;
    }
    // printf("PPP %d \n", p);
    s17[3].p1_e = (N / 2) - p - 1;
    s17[3].p2_e = (N / 2 - 1) - s13[2].index - 1;

    s17[0].result = sorted;
    for (i = 1; i < 4; i++)
    {
        s17[i].result = s17[i - 1].result + s17[i - 1].p1_e + s17[i - 1].p2_e + 2;
    }

    // for (i = 0; i < N / 2; i++)
    //     printf("%d %d \n", s5.out_arr[i], s6.out_arr[i]);
    // printf("%d -------------\n", j);

    pthread_create(&thread_handles[0], (pthread_attr_t *)NULL, merge_endf, (void *)&s17[0]);
    pthread_create(&thread_handles[1], (pthread_attr_t *)NULL, merge_endf, (void *)&s17[1]);
    pthread_create(&thread_handles[2], (pthread_attr_t *)NULL, merge_endf, (void *)&s17[2]);
    pthread_create(&thread_handles[3], (pthread_attr_t *)NULL, merge_endf, (void *)&s17[3]);

    pthread_join(thread_handles[0], NULL);
    pthread_join(thread_handles[1], NULL);
    pthread_join(thread_handles[2], NULL);
    pthread_join(thread_handles[3], NULL);
    
    // printf("3 \n");

    // for (i = 0; i < 4; i++)
    // {
    //     printf("%d ===> ", i);
    //     for (j = 0; j <= s17[i].p1_e; j++)
    //     {
    //         printf("%d ", s17[i].arr1[j]);
    //     }
    //     printf("    ||||||   ");
    //     for (j = 0; j <= s17[i].p2_e; j++)
    //     {
    //         printf("%d ", s17[i].arr2[j]);
    //     }
    //     printf("\n");
    // }

    // merge_endf((void *)&s17[0]);
    // merge_endf((void *)&s17[1]);
    // printf("%d ===> ", i);
    // for (j = 0; j < N; j++)
    // {
    //     printf("%d ", sorted[j]);
    // }
    // printf("\n");

    // printf("%d -------------\n", j);
    // for (i = 0; i < 2 * j; i++)
    //     printf("%d %d\n", s12.out_arr[i], s12.out_arr2[i]);
    // printf("-------------\n");

    // Out
    // sorted = s9.out_arr;
    // sorted[0] = 0;
    // printf("-------------\n");
    // int i;
    // for (i = 0; i < N; i++)
    //     printf("%d \n", values[i]);
    // printf("-------------\n");

    // int i;
    // for (i = 0; i < N; i++)
    //     printf("%d \n", s9.out_arr[i]);
}