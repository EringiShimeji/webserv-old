#include <iostream>
#include "IOTask.hpp"
#include <unistd.h>
#include <stdnoreturn.h>
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
	}
//	std::cout << "added task: fd=" << task->fd << ", events=" << task->events << std::endl;
//	for (unsigned long i = 0; i < poll_fds.size(); i++) {
//		std::cout << "[" << i << "] fd=" << poll_fds[i].fd << ", events=" << poll_fds[i].events
//				  << ", revents=" << poll_fds[i].revents << std::endl;
//	}
//	std::cout << "vacant_slots: size=" << vacant_slots.size();
//	if (!vacant_slots.empty())
//		std::cout << ", top=" << vacant_slots.top();
//	std::cout << "\n" << std::endl;
}

void IOTaskManager::remove(int fd, short events) {
	std::pair<int, short> key = std::pair<int, short>(fd, events);
	TaskIndexMap::iterator it = task_index_map.find(key);
	if (it == task_index_map.end() || it->second == NOT_MONITORED)
		return;
	int *index = &it->second;

	std::cout << "\n" << std::endl;
	poll_fds[*index].fd = NOT_MONITORED;
	poll_fds[*index].events = 0;
	poll_fds[*index].revents = 0;
	delete tasks[*index];
	vacant_slots.push(*index);
	*index = NOT_MONITORED;
//	std::cout << "removed task: fd=" << fd << ", events=" << events << std::endl;
//	for (unsigned long i = 0; i < poll_fds.size(); i++) {
//		std::cout << "[" << i << "] fd=" << poll_fds[i].fd << ", events=" << poll_fds[i].events
//				  << ", revents=" << poll_fds[i].revents << std::endl;
//	}
//	std::cout << "vacant_slots: size=" << vacant_slots.size();
//	if (!vacant_slots.empty())
//		std::cout << ", top=" << vacant_slots.top();
//	std::cout << "\n" << std::endl;
}

noreturn void IOTaskManager::executeTasks() {
	while (true) {
		if (poll(&poll_fds[0], poll_fds.size(), 0) == -1)
			perror("poll");

		for (unsigned long i = 0; i < poll_fds.size(); i++) {
			PollFd *poll_fd = &poll_fds[i];
			if (poll_fd->fd == NOT_MONITORED)
				continue;

			IOTask *task = tasks[i];
			if (poll_fd->revents & task->events)
				task->execute(*this);
		}
	}
}

IOTask::IOTask(int fd, short events) : fd(fd), events(events) {}

IOTask::~IOTask() {}

ReadFileCallback::ReadFileCallback(ReadFileCallback::Callback callback) : callback(callback) {}

void ReadFileCallback::trigger(const std::string &fileData, IOTaskManager &m) {
	callback(fileData, m);
}

ReadFile::ReadFile(int fd, ReadFileCallback *callback) : IOTask(fd, POLLIN), callback(callback) {
	memset(tmp_buf, 0, READ_FILE_READ_SIZE);
}

void ReadFile::execute(IOTaskManager &m) {
	ssize_t read_size = read(fd, tmp_buf, READ_FILE_READ_SIZE);
	if (read_size == -1) {
		perror("read");
		exit(1);
	}

	if (read_size == 0) {
		if (callback != nullptr)
			callback->trigger(read_buf, m);
		m.remove(fd, events);
		return;
	}
	read_buf.append((const char *) tmp_buf);
}

ReadFile::~ReadFile() {
	delete callback;
}

WriteFile::WriteFile(int fd, const std::string &dataToWrite, Callback callback)
	: IOTask(fd, POLLOUT), callback(callback) {
	this->buf = dataToWrite.c_str();
	this->buf_len = dataToWrite.length();
}

void WriteFile::execute(IOTaskManager &m) {
	ssize_t written_size = write(fd, buf, std::min(WRITE_FILE_WRITE_SIZE, buf_len));
	buf += written_size;
	buf_len -= buf_len;

	if (buf_len == 0) {
		callback();
		m.remove(fd, events);
	}
}

AcceptCallback::AcceptCallback(AcceptCallback::Callback callback) : callback(callback) {}

void
AcceptCallback::trigger(int connection, SockAddrIn addr, socklen_t addr_len, IOTaskManager &m) {
	callback(connection, addr, addr_len, m);
}

Accept::Accept(int socket, AcceptCallback *callback) : IOTask(socket, POLLIN), callback(callback) {}

void Accept::execute(IOTaskManager &m) {
	int connection = accept(fd, (SockAddr *) &sock_addr, &sock_addr_len);
	if (callback != nullptr)
		callback->trigger(connection, sock_addr, sock_addr_len, m);
}

Accept::~Accept() {
	delete callback;
}

CloseCallback::CloseCallback(CloseCallback::Callback callback, SockAddrIn sock_addr,
							 socklen_t sock_addr_len)
	: callback(callback), sock_addr(sock_addr), sock_addr_len(sock_addr_len) {}

void CloseCallback::trigger() {
	callback(sock_addr, sock_addr_len);
}

Close::Close(int fd, CloseCallback *callback) : IOTask(fd, POLLHUP), callback(callback) {}

Close::~Close() {
	delete callback;
}

void Close::execute(IOTaskManager &m) {
	close(fd);
	if (callback != nullptr)
		callback->trigger();
	m.remove(fd, events);
}
