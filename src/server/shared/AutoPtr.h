////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _TRINITY_AUTO_PTR_H
#define _TRINITY_AUTO_PTR_H

#include <memory>


namespace Trinity
{
    template <class Pointer, class Lock>
    class AutoPtr : public std::shared_ptr<Pointer>
    {
        public:
            AutoPtr() : std::shared_ptr<Pointer>() {}

            AutoPtr(Pointer* x) : std::shared_ptr<Pointer>(x)
            {
            }

            operator bool() const
            {
                return std::shared_ptr<Pointer>::get() != nullptr;
            }

            bool operator !() const
            {
                return std::shared_ptr<Pointer>::get() == nullptr;
            }

            bool operator !=(Pointer* x) const
            {
                return std::shared_ptr<Pointer>::get() != x;
            }
    };
}

#endif
