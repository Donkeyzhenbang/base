### stl_construct
``` cpp
#include <new.h> 
// 欲使用 placement new，需先含入此檔
template <class T1, class T2>
inline void construct(T1* p, const T2& value) {
new (p) T1(value); // placement new; 喚起 T1::T1(value);
}
// 以㆘是 destroy() 第㆒版本，接受㆒個指標。
template <class T>
inline void destroy(T* pointer) {
pointer->~T(); // 喚起 dtor ~T()
}
// 以㆘是 destroy() 第㆓版本，接受兩個迭代器。此函式設法找出元素的數值型別，
// 進而利用 __type_traits<> 求取最適當措施。
template <class ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last) {
__destroy(first, last, value_type(first));
}
// 判斷元素的數值型別（ value type）是否有 trivial destructor
template <class ForwardIterator, class T>
inline void __destroy(ForwardIterator first, ForwardIterator last, T*)
{
typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
__destroy_aux(first, last, trivial_destructor());
}
// 如果元素的數值型別（ value type）有 non-trivial destructor…
template <class ForwardIterator>
inline void
__destroy_aux(ForwardIterator first, ForwardIterator last, __false_type) {
for ( ; first < last; ++first)
destroy(&*first);
}
// 如果元素的數值型別（ value type）有 trivial destructor…
template <class ForwardIterator>
inline void __destroy_aux(ForwardIterator, ForwardIterator, __true_type) {}
// 以㆘是 destroy() 第㆓版本針對迭代器為 char* 和 wchar_t* 的特化版
inline void destroy(char*, char*) {}
inline void destroy(wchar_t*, wchar_t*) {}
```

### defalloc
- 核心目的：封装底层内存操作，实现类型安全的内存管理
- 解耦目的：抽象统一内存接口，实现内存分配与容器的解耦（可替换性）
    - 将内存分配（allocate）、内存释放（deallocate）与上层容器（如 vector、list）解耦，容器无需关心底层内存是通过operator new、malloc还是内存池实现；
    - 提供统一的标准化接口（allocate分配 n 个 T 类型对象内存、deallocate释放内存、address获取对象地址等），上层容器只需依赖该接口，无需关心具体实现；
    - 实现 “开闭原则”：后续若需优化内存性能（如用内存池减少碎片、用共享内存实现跨进程分配），只需替换 allocator 的底层实现，无需修改任何容器代码。
- 泛型支撑：提供类型别名，保障泛型编程的兼容性与可移植性
    - 这些别名是泛型容器（如template<class T, class Alloc=allocator<T>> class vector）的 “通用语言”，让容器能够统一获取分配器的相关类型，无需关心T的具体类型；
    - 例如：容器需要知道 “元素类型”（value_type）、“指向元素的指针类型”（pointer）、“元素个数的类型”（size_type）、“两个指针的差值类型”（difference_type），这些别名提供了统一的访问方式；
    - 保证了泛型代码的可移植性：无论T是int、std::string还是自定义类型，容器都能通过相同的别名获取所需类型，无需适配。
- 异常处理：封装内存不足场景，实现可控的内存耗尽处理
    - 调用set_new_handler(0)：设置 “内存分配失败处理函数” 为空，此时::operator new分配失败时不会抛出异常（早期 C++ 特性），而是返回nullptr；
    - 检测tmp == 0：判断内存分配失败后，输出明确的错误提示（out of memory），并调用exit(1)优雅终止程序；
    - 对比直接使用::operator new（默认分配失败可能抛出异常或直接崩溃），该设计让内存耗尽的行为更可控，方便调试和问题定位。
- 性能优化：提供内存大小辅助方法，优化内存分配效率
    - init_page_size()：计算 “初始化页大小”，返回max(1, 4096/sizeof(T))
        4096 是操作系统典型的 “内存页大小”（操作系统管理内存的基本单位），让分配的内存大小对齐内存页，减少内存碎片，提高内存访问效率（减少缺页中断）；
        通过max(1, ...)保证至少分配 1 个 T 类型对象的内存（避免sizeof(T)过大时，4096/sizeof(T)小于 1）。
    - max_size()：返回分配器可分配的最大元素个数，即max(1, UINT_MAX/sizeof(T))
        给上层容器提供明确的分配上限，避免传入过大的n值导致内存分配失败或数值溢出，提前规避无效的分配请求。
- 兼容性适配：特化allocator<void>，解决void 类型的编译兼容性问题
    代码专门特化了allocator<void>，目的是支撑泛型编程中的特殊场景：
    - 普通模板allocator<T>对T=void无效：void类型无法计算sizeof(T)，也无法定义T&（void 引用不存在）；
    - 泛型编程中可能出现allocator<void>的推导场景（如模板参数自动推导），特化版本仅保留void* pointer别名，避免编译错误；
    - 保证分配器在泛型场景下的完整性，避免因void类型导致上层代码编译失败。

```cpp
#ifndef DEFALLOC_H
#define DEFALLOC_H

#include <new.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <iostream.h>
#include <algobase.h>

template<class T>
inline T* allocate(ptrdiff_t size, T*) {
    set_new_handler(0);
    T* tmp = (T*)(::operator new(size_t)(size * sizeof(T)));
    if(tmp == 0){
        cerr << "out of memory" << endl;
        exit(1);
    }
    return tmp;
}

template<class T>
inline void deallocate(T* buffer){
    ::operator delete(buffer);
}

template<class T>
inline void allocator{
public:
    // 值 引用 指针 const指针 const引用 size_type ptrdiff
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    pointer allocate(size_type n){
        return ::allocate((difference_type)n, (pointer)0);
    }

    void deallocate(pointer p) {::deallocate(p); }
    
    pointer address(reference x) {return (pointer)&x;}

    const_pointer const_address(const_reference x){
        return (const_pointer)&x;
    }

    size_type init_page_size(){
        return max(size_type(1), size_type(4096/sizeof(T)));
    }

    size_type max_size() const {
        return max(size_type(1), size_type(UINT_MAX/sizeof(T)));
    }

};

class allocator<void>
{
pulibc:
    typedef void* pointer;
};


#endif
```

### simple_alloc
```cpp
// ------------------------------ simple_alloc 类模板 ------------------------------
template<class T, class Alloc>
class simple_alloc {
public:
    static T *allocate(size_t n)
    { 
        return 0 == n ? 0 : (T*) Alloc::allocate(n * sizeof (T)); 
    }
    
    static T *allocate(void)
    { 
        return (T*) Alloc::allocate(sizeof (T)); 
    }
    
    static void deallocate(T *p, size_t n)
    { 
        if (0 != n) 
            Alloc::deallocate(p, n * sizeof (T)); 
    }
    
    static void deallocate(T *p)
    { 
        Alloc::deallocate(p, sizeof (T)); 
    }
};

// ------------------------------ vector 类模板 ------------------------------
template <class T, class Alloc = alloc> // 預設使用 alloc 為配置器
class vector {
protected:
    // 專屬之空間配置器，每次配置㆒個元素大小
    typedef simple_alloc<value_type, Alloc> data_allocator;
    
    void deallocate() {
        if (...)
            data_allocator::deallocate(start, end_of_storage - start);
    }
    // ... 其他成员省略
};

// ------------------------------ __malloc_alloc_template 相关代码 ------------------------------
#if 0
# include <new>
# define __THROW_BAD_ALLOC throw bad_alloc
#elif !defined(__THROW_BAD_ALLOC)
# include <iostream.h>
# define __THROW_BAD_ALLOC cerr << "out of memory" << endl; exit(1)
#endif

// ㆒般而言是 thread-safe，並且對於空間的運用比較高效（ efficient）。
// 以㆘是第㆒級配置器。
// 注意，無「 template 型別參數」。至於「非型別參數」 inst，完全沒派㆖用場。
template <int inst>
class __malloc_alloc_template {
private:
    // 以㆘都是函式指標，所代表的函式將用來處理記憶體不足的情況。
    // oom : out of memory.
    static void *oom_malloc(size_t);
    static void *oom_realloc(void *, size_t);
    static void (* __malloc_alloc_oom_handler)();

public:
    static void * allocate(size_t n)
    {
        void *result = malloc(n); // 第㆒級配置器直接使用 malloc()
        // 以㆘，無法滿足需求時，改用 oom_malloc()
        if (0 == result) 
            result = oom_malloc(n);
        return result;
    }

    static void deallocate(void *p, size_t /* n */)
    {
        free(p); // 第㆒級配置器直接使用 free()
    }

    static void * reallocate(void *p, size_t /* old_sz */, size_t new_sz)
    {
        void * result = realloc(p, new_sz); // 第㆒級配置器直接使用 realloc()
        // 以㆘，無法滿足需求時，改用 oom_realloc()
        if (0 == result) 
            result = oom_realloc(p, new_sz);
        return result;
    }

    // 以㆘模擬 C++ 的 set_new_handler(). 換句話說，你可以透過它，
    // 指定你自己的 out-of-memory handler
    static void (* set_malloc_handler(void (*f)()))()
    {
        void (* old)() = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = f;
        return(old);
    }
};

// malloc_alloc out-of-memory handling
// 初值為 0。有待客端設定。
template <int inst>
void (* __malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

template <int inst>
void * __malloc_alloc_template<inst>::oom_malloc(size_t n)
{
    void (* my_malloc_handler)();
    void *result;

    for (;;) { // 不斷嘗試釋放、配置、再釋放、再配置…
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)(); // 呼叫處理常式，企圖釋放記憶體。
        result = malloc(n); // 再次嘗試配置記憶體。
        if (result) 
            return(result);
    }
}

template <int inst>
void * __malloc_alloc_template<inst>::oom_realloc(void *p, size_t n)
{
    void (* my_malloc_handler)();
    void *result;

    for (;;) { // 不斷嘗試釋放、配置、再釋放、再配置…
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)(); // 呼叫處理常式，企圖釋放記憶體。
        result = realloc(p, n); // 再次嘗試配置記憶體。
        if (result) 
            return(result);
    }
}

// 注意，以㆘直接將參數 inst 指定為 0。
typedef __malloc_alloc_template<0> malloc_alloc;
```