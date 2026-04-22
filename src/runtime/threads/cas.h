#pragma once

inline void cpu_relax(void) {
#if defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__("pause");
#elif defined(__aarch64__) || defined(__arm__)
    __asm__ __volatile__("yield");
#else
    __asm__ __volatile__("" ::: "memory");
#endif
}


inline void cas_sleep(int &backoff_us) {
    // if (backoff_us <= 16) {
    //     cpu_relax();
    //     backoff_us *= 2;
    // } else if (backoff_us <= 256) {
    //     sched_yield();
    //     backoff_us *= 2;
    // } else {
    //     struct timespec ts = {
    //         .tv_sec = 0,
    //         .tv_nsec = backoff_us * 1000
    //     };
    //     nanosleep(&ts, NULL);
    //     if (backoff_us < 1000) backoff_us *= 2;
    // }
    if (++backoff_us <= 4)
        cpu_relax();
    else if(backoff_us<8)
        sched_yield();
    else {
        struct timespec ts = {
            .tv_sec = 0,
            .tv_nsec = backoff_us * 1000
        };
        nanosleep(&ts, NULL);
        if (backoff_us < 1000) backoff_us *= 2;
    }
}


template<typename T>
void CAS_push(T *x, T *&ST) {
    T* loaded = __atomic_load_n(&ST, __ATOMIC_RELAXED);
    do {
        *(T**)x = loaded;
    } while (!__atomic_compare_exchange_n(
                &ST,
                &loaded,
                x,
                true,
                __ATOMIC_RELEASE,
                __ATOMIC_RELAXED
                ));
}
