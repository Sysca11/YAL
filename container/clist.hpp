#pragma once
#include <type_traits>
namespace YAL
{
    template <typename T, bool ishead>
    class clistNode
    {
    public:
        clistNode<T, false> *prev, *next;
        std::conditional_t<ishead, int, T> inner;
        clistNode<T, false> *myPtr()
        {
            return (clistNode<T, false> *)this;
        }
        void operator=(clistNode const& t) {
            inner = t.inner;
            prev = next = myPtr();
        }
        void detach()
        {
            prev->next = next;
            next->prev = prev;
            prev = next = myPtr();
        }
        void insert_after(clistNode<T, false> *nd)
        {
            next = nd->next;
            prev = nd;
            nd->next = myPtr();
            next->prev = myPtr();
        }
        void insert_before(clistNode<T, false> *nd)
        {
            prev = nd->prev;
            next = nd;
            nd->prev = myPtr();
            prev->next = myPtr();
        }
        clistNode()
        {
            prev = next = myPtr();
        }
        clistNode(T &&x) : inner(std::forward<T>(x))
        {
            prev = next = myPtr();
        }
        clistNode(T const &x) : inner(x)
        {
            prev = next = myPtr();
        }
        ~clistNode()
        {
            //detach();
        }
    };
}; // namespace YAL
