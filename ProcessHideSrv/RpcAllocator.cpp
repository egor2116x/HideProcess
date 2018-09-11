#include <Rpc.h>

extern "C"
void* __RPC_API MIDL_user_allocate(size_t size)
{
    return malloc(size);
}

extern "C"
void __RPC_API MIDL_user_free(void* ptr)
{
    free(ptr);
}
