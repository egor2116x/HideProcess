#pragma once
#include "stdafx.h"

class RpcServerError : public std::runtime_error
{
public:
    explicit RpcServerError(DWORD error = ::GetLastError()) :
        runtime_error("Rpc server logger error"),
        m_error(error)
    {
    }

    explicit RpcServerError(const char* message, DWORD error = ::GetLastError()) :
        runtime_error(message),
        m_error(error)
    {
    }

    DWORD GetError() const
    {
        return m_error;
    }

private:
    DWORD m_error;
};

class ServiceError : public std::runtime_error
{
public:
    explicit ServiceError(DWORD error = ::GetLastError()) :
        runtime_error("Service logger error"),
        m_error(error)
    {
    }

    explicit ServiceError(const char* message, DWORD error = ::GetLastError()) :
        runtime_error(message),
        m_error(error)
    {
    }

    DWORD GetError() const
    {
        return m_error;
    }

private:
    DWORD m_error;
};

class WriterLoggerError : public std::runtime_error
{
public:
    explicit WriterLoggerError(DWORD error = ::GetLastError()) :
        runtime_error("Writer logger error"),
        m_error(error)
    {
    }

    explicit WriterLoggerError(const char* message, DWORD error = ::GetLastError()) :
        runtime_error(message),
        m_error(error)
    {
    }

    DWORD GetError() const
    {
        return m_error;
    }

private:
    DWORD m_error;
};