#include "io_task_manager.hpp"
#include <algorithm>

IOTaskManager::IOTaskManager() {
}

IOTaskManager::~IOTaskManager() {
}

#include <iostream>
void IOTaskManager::executeTasks() {
    while (true) {
        for (unsigned long i = 0; i < tasks_.size(); i++) {
            IOTask *task = tasks_.at(i);
            if (task == NULL) {
                continue;
            }
            Result<IOTaskResult, std::string> r = task->execute();
            if (r.isErr()) {
                std::cerr << r.unwrapErr() << std::endl;
                delete task;
                continue;
            }
            if (r.unwrap() == kTaskComplete) {
                delete task;
            }
        }
    }
}

void IOTaskManager::removeTask(IOTask *task) {
    std::vector<IOTask *>::iterator it = std::find(tasks_.begin(), tasks_.end(), task);
    if (it != tasks_.end()) {
        *it = NULL;
    }
}

void IOTaskManager::addTask(IOTask *task) {
    tasks_.push_back(task);
}
