////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
//  Copyright 2014 Millenium-studio SARL
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _MEMORY_H
#define _MEMORY_H

#include "DetourAlloc.h"
#include <memory>
#include <type_traits>
#include <utility>

// memory management
inline void* dtCustomAlloc(int size, dtAllocHint /*hint*/)
{
    return (void*)new unsigned char[size];
}

inline void dtCustomFree(void* ptr)
{
    delete[](unsigned char*)ptr;
}

namespace Trinity
{
namespace Impl
{
template <typename T, typename Del>
struct stateful_unique_ptr_deleter
{
    using pointer = T;
    explicit stateful_unique_ptr_deleter(Del deleter) : _deleter(std::move(deleter)) { }
    void operator()(pointer ptr) const { (void)_deleter(ptr); }

private:
    Del _deleter;
};
}

template <typename Ptr, typename Del>
Impl::stateful_unique_ptr_deleter<Ptr, Del> unique_ptr_deleter(Del deleter)
{
    return Impl::stateful_unique_ptr_deleter<Ptr, Del>(std::move(deleter));
}

template <typename Ptr, typename Del>
std::unique_ptr<typename std::remove_pointer<Ptr>::type, Impl::stateful_unique_ptr_deleter<Ptr, Del>>
make_unique_ptr_with_deleter(Ptr ptr, Del deleter)
{
    return std::unique_ptr<typename std::remove_pointer<Ptr>::type, Impl::stateful_unique_ptr_deleter<Ptr, Del>>(ptr, std::move(deleter));
}

template <typename T, void (*Del)(T*)>
std::unique_ptr<T, Impl::stateful_unique_ptr_deleter<T*, void(*)(T*)>>
make_unique_ptr_with_deleter(T* ptr)
{
    return make_unique_ptr_with_deleter(ptr, Del);
}
}

#endif
