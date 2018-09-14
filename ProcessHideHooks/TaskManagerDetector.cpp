#include "TaskManagerDetector.h"

std::unique_ptr<TaskManagerDetector> TaskManagerDetector::m_instance(nullptr);

std::unique_ptr<TaskManagerDetector> & TaskManagerDetector::GetInstance()
{
    if (m_instance.get() == nullptr)
    {
        m_instance.reset(new TaskManagerDetector);
    }
    return m_instance;
}
