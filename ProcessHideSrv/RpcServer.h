#pragma once
#include "RPCGuards.h"

class RpcServer
{
public:
    RpcServer() {}
    void Init();
    void Listen();
    void StopListening();
    void Unregistred();
    void Wait();
private:
    ServerIfHandleGuard m_ifHandleGuard;
};

