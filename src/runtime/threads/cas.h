#pragma once

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
