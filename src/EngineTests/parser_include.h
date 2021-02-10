#pragma once

#if defined(__REFLECTION_PARSER__)
#define rg_reflect(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#define rg_reflect(...)
#endif

template<typename T>
class meta_type {};


#define reflect(T) meta_type<T>();
