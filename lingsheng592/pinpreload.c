#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

/* isolcpus=domain does not migrate threads. A wide taskset leaves every
   ITK worker on one CPU. Pin each new thread to its own compute CPU.
   The last core of every 38-core NUMA node is a housekeeping core. */
static int cpus[592];
static int ncpus;
static atomic_int cursor;
static int (*real_pthread_create)(pthread_t *, const pthread_attr_t *,
                                  void *(*)(void *), void *);

struct pin_pack {
    void *(*fn)(void *);
    void *arg;
    int cpu;
};

static void build_cpus(void) {
    ncpus = 0;
    for (int start = 0; start < 608; start += 38) {
        for (int c = start; c < start + 37 && ncpus < 592; c++)
            cpus[ncpus++] = c;
    }
}

static void pin_self(int cpu) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
}

static void *tramp(void *raw) {
    struct pin_pack *pack = raw;
    void *(*fn)(void *) = pack->fn;
    void *arg = pack->arg;
    int cpu = pack->cpu;
    free(pack);
    pin_self(cpu);
    return fn(arg);
}

static void init_once(void) {
    if (ncpus > 0 && real_pthread_create) return;
    if (ncpus == 0) build_cpus();
    if (!real_pthread_create)
        real_pthread_create = dlsym(RTLD_NEXT, "pthread_create");
}

__attribute__((constructor)) static void pin_init(void) {
    init_once();
    if (ncpus > 0) {
        pin_self(cpus[0]);
        atomic_store(&cursor, 1);
        fprintf(stderr, "pinpreload: main on cpu %d, compute_cpus=%d\n",
                cpus[0], ncpus);
    }
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                    void *(*start)(void *), void *arg) {
    init_once();
    if (!real_pthread_create || ncpus == 0)
        return real_pthread_create ? real_pthread_create(thread, attr, start, arg) : 1;
    struct pin_pack *pack = malloc(sizeof(*pack));
    if (!pack) return real_pthread_create(thread, attr, start, arg);
    int idx = atomic_fetch_add(&cursor, 1);
    pack->fn = start;
    pack->arg = arg;
    pack->cpu = cpus[idx % ncpus];
    return real_pthread_create(thread, attr, tramp, pack);
}
