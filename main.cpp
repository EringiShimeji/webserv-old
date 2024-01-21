#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include "IOTask.hpp"

using namespace std;

void handleHangUp() {
	cout << "-> disconnected" << endl;
}

void handleConnection(int fd, SockAddrIn addr, socklen_t addr_len, IOTaskManager &m) {
	char dst[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &addr, dst, addr_len);
	cout << "<- connection from " << dst << endl;

	CloseFactory hangUp(handleHangUp);
	m.createIOTask(hangUp, fd);
}

int main() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET; //アドレスファミリ(ipv4)
	addr.sin_port = htons(1234); //ポート番号,htons()関数は16bitホストバイトオーダーをネットワークバイトオーダーに変換
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //IPアドレス,inet_addr()関数はアドレスの翻訳

	bind(sock, (struct sockaddr *) &addr, sizeof(addr));
	listen(sock, SOMAXCONN);

	IOTaskManager taskManager;
	AcceptFactory acceptConnection(handleConnection);

	taskManager.createIOTask(acceptConnection, sock);

	taskManager.executeTasks();
}
