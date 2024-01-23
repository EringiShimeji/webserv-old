#include <iostream>
#include "IOTask.hpp"
#include <unistd.h>
#include <sys/socket.h>

void IOTaskManager::add(IOTask *task) {
	std::pair<int, short> key = std::pair<int, short>(task->fd, task->events);
	TaskIndexMap::iterator it = task_index_map.find(key);
	if (it != task_index_map.end() && it->second != NOT_MONITORED)
		return; // TODO:例外を返す

	if (vacant_slots.empty()) {
		PollFd poll_fd = {task->fd, task->events, 0};

		poll_fds.push_back(poll_fd);
		tasks.push_back(task);
		task_index_map[key] = poll_fds.size() - 1;
	} else {
		unsigned long index = vacant_slots.top();
		vacant_slots.pop();

		poll_fds[index].fd = task->fd;
		poll_fds[index].events = task->events;
		tasks[index] = task;
		task_index_map[key] = index;
	}
}

void IOTaskManager::remove(IOTask *task) {
	std::pair<int, short> key = std::pair<int, short>(task->fd, task->events);
	TaskIndexMap::iterator it = task_index_map.find(key);
	if (it == task_index_map.end() || it->second == NOT_MONITORED)
		return;
	int *index = &it->second;

	poll_fds[*index].fd = NOT_MONITORED;
	poll_fds[*index].events = 0;
	poll_fds[*index].revents = 0;
	delete task;
	vacant_slots.push(*index);
	*index = NOT_MONITORED;
}

void IOTaskManager::executeTasks() {
	while (true) {
		poll(&poll_fds[0], poll_fds.size(), 0);

		for (unsigned long i = 0; i < poll_fds.size(); i++) {
			PollFd *poll_fd = &poll_fds[i];
			if (poll_fd->fd == NOT_MONITORED)
				continue;

			IOTask *task = tasks[i];
			if (poll_fd->revents & task->events) {
				if (task->execute() == REMOVE) {
					remove(task);
				}
			}
		}
	}
}

IOTask::IOTask(int fd, short events) : fd(fd), events(events) {}

IOTask::~IOTask() {}

IOCallback::~IOCallback() {}

ReadFile::ReadFile(int fd, ReadFileCallback *callback) : IOTask(fd, POLLIN), callback(callback) {
	memset(tmp_buf, 0, READ_FILE_READ_SIZE);
}

IOTaskResult ReadFile::execute() {
	ssize_t read_size = read(fd, tmp_buf, READ_FILE_READ_SIZE);

	if (read_size == 0) {
		if (callback != nullptr)
			callback->trigger(read_buf);
		return REMOVE;
	}
	read_buf.append((const char *) tmp_buf);
	return PAUSE;
}

ReadFile::~ReadFile() {
	delete callback;
}

WriteFile::WriteFile(int fd, const std::string &dataToWrite, WriteFileCallback *callback)
	: IOTask(fd, POLLOUT), callback(callback), dataToWrite(dataToWrite) {
	this->buf = this->dataToWrite.c_str();
	this->buf_len = dataToWrite.length();
}

IOTaskResult WriteFile::execute() {
	ssize_t written_size = write(fd, buf, std::min(WRITE_FILE_WRITE_SIZE, buf_len));
	buf += written_size;
	buf_len -= buf_len;

	if (buf_len == 0) {
		if (callback != nullptr)
			callback->trigger();
		return REMOVE;
	}
	return PAUSE;
}

Accept::Accept(int socket, AcceptCallback *callback)
	: IOTask(socket, POLLIN), callback(callback) {}

IOTaskResult Accept::execute() {
	int connection = accept(fd, (SockAddr *) &sock_addr, &sock_addr_len);
	if (callback != nullptr)
		callback->trigger(connection, sock_addr);
	return PAUSE;
}

Accept::~Accept() {
	delete callback;
}
