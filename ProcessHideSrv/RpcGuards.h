#pragma once
#include "stdafx.h"

class noncopyable
{
protected:
    noncopyable() {}
    ~noncopyable() {}

private:  // emphasize the following members are private
    noncopyable(const noncopyable&);
    noncopyable& operator=(const noncopyable&);
};

class ServerIfHandleGuard : private noncopyable
{
public:
    explicit ServerIfHandleGuard(RPC_IF_HANDLE handle = NULL) :
        m_handle(handle)
    {
    }

    ~ServerIfHandleGuard()
    {
        if (m_handle != NULL)
        {
            ::RpcServerUnregisterIf(m_handle, NULL, TRUE);
        }
    }

    RPC_IF_HANDLE get()
    {
        return m_handle;
    }

    RPC_IF_HANDLE release()
    {
        RPC_IF_HANDLE handle = m_handle;
        m_handle = NULL;
        return handle;
    }

    void reset(RPC_IF_HANDLE handle = NULL)
    {
        if (m_handle != handle)
        {
            if (m_handle != NULL)
            {
                ::RpcServerUnregisterIf(m_handle, NULL, TRUE);
            }
            m_handle = handle;
        }
    }

private:
    RPC_IF_HANDLE m_handle;
};

class BindingHandleGuard : private noncopyable
{
public:
    explicit BindingHandleGuard(RPC_BINDING_HANDLE* handle = NULL) :
        m_handle(handle)
    {
    }

    ~BindingHandleGuard()
    {
        if (m_handle != NULL)
        {
            ::RpcBindingFree(m_handle);
        }
    }

    RPC_BINDING_HANDLE* get()
    {
        return m_handle;
    }

    RPC_BINDING_HANDLE* release()
    {
        RPC_BINDING_HANDLE* handle = m_handle;
        m_handle = NULL;
        return handle;
    }

    void reset(RPC_BINDING_HANDLE* handle = NULL)
    {
        if (m_handle != handle)
        {
            if (m_handle != NULL)
            {
                ::RpcBindingFree(m_handle);
            }
            m_handle = handle;
        }
    }

private:
    RPC_BINDING_HANDLE* m_handle;
};

class BindingVectorGuard : private noncopyable
{
public:
    explicit BindingVectorGuard(RPC_BINDING_VECTOR* bindingVector = NULL) :
        m_bindingVector(bindingVector)
    {
    }

    ~BindingVectorGuard()
    {
        if (m_bindingVector != NULL)
        {
            ::RpcBindingVectorFree(&m_bindingVector);
        }
    }

    RPC_BINDING_VECTOR* get()
    {
        return m_bindingVector;
    }

    RPC_BINDING_VECTOR* release()
    {
        RPC_BINDING_VECTOR* vector = m_bindingVector;
        m_bindingVector = NULL;
        return vector;
    }

    void reset(RPC_BINDING_VECTOR* vector = NULL)
    {
        if (m_bindingVector != vector)
        {
            if (m_bindingVector != NULL)
            {
                ::RpcBindingVectorFree(&m_bindingVector);
            }
            m_bindingVector = vector;
        }
    }

private:
    RPC_BINDING_VECTOR* m_bindingVector;
};

template <class StrT>
class StringGuard : private noncopyable
{
public:
    explicit StringGuard(StrT string = NULL) :
        m_string(string)
    {
    }

    ~StringGuard()
    {
        if (m_string != NULL)
        {
            ::RpcStringFree(&m_string);
        }
    }

    StrT get()
    {
        return m_string;
    }

    StrT release()
    {
        StrT str = m_string;
        m_string = NULL;
        return str;
    }

    void reset(StrT str = NULL)
    {
        if (m_string != str)
        {
            if (m_string != NULL)
            {
                ::RpcStringFree(&m_string);
            }
            m_string = str;
        }
    }

private:
    StrT m_string;
};
