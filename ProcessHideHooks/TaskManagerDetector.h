#pragma once
#include "stdafx.h"

class TaskManagerDetector
{
public:
    static std::unique_ptr<TaskManagerDetector> & GetInstance();
private:
    TaskManagerDetector() {}
    TaskManagerDetector(const TaskManagerDetector &) = delete;
    TaskManagerDetector(const TaskManagerDetector &&) = delete;
    TaskManagerDetector & operator=(const TaskManagerDetector &) = delete;
    TaskManagerDetector & operator=(const TaskManagerDetector &&) = delete;
private:
    static std::unique_ptr<TaskManagerDetector> m_instance;
};

