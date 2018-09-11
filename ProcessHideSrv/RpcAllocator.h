#ifndef RpcAllocator_h__
#define RpcAllocator_h__

namespace ubRpc
{

    class RpcAllocator
    {
    public:
        template <class T>
        static T* Allocate(size_t count)
        {
            T* ptr = reinterpret_cast<T*>(MIDL_user_allocate(count * sizeof(T)));
            if (ptr == NULL)
            {
                throw std::bad_alloc();
            }
            return ptr;
        }

        template <class T>
        static void Free(T* ptr)
        {
            if (ptr != NULL)
            {
                MIDL_user_free(ptr);
            }
        }
    };

} 

#endif // RpcAllocator_h__
