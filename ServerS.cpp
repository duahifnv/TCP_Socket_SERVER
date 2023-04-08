#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#pragma warning(disable: 4996)

#define MAX_CONNECTIONS 100
#define MSG_MAX_SIZE 256
using namespace std;
//�������� ������� ������� (100 ����������)
SOCKET Connections[MAX_CONNECTIONS];
int Counter = 0;

//�������� ������� ������ ����������� ����� ���������
void ClientHandler(int index) {
	char msg[MAX_CONNECTIONS];
	while (true) {
		recv(Connections[index], msg, sizeof(msg), NULL);
		for (int i = 0; i < Counter; i++) {
			if (i == index) continue;
			send(Connections[i], msg, sizeof(msg), NULL);
		}
	}
}

int main() {
	WSAData wsadata;
	WORD DLLVersion = MAKEWORD(2, 1);

	//������ ��������� WSAStartup � �������� �������
	if (WSAStartup(DLLVersion, &wsadata) != 0) {
		cerr << "no wsa sry :(" << endl;
		exit(1);
	}

	//��������� ������ ������
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);

	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); //�������� ������
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); //�������� ������ � ������
	listen(sListen, SOMAXCONN); //������������� ����� ��� ����������� ������� � �������

	SOCKET newConnection; //�������� ������ ������ ��� ��������� ���������� � ��������

	//����� ���� ��������� ����������� 100 ��������
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		//�������� ����������� ������� � �������
		if (newConnection == 0) {
			cout << "cant connect new client!" << endl;

		}
		else {
			cout << "Client Connected!\n";
			char msg[MAX_CONNECTIONS / 2] = "Privetstvuyu smotryashih!";
			send(newConnection, msg, sizeof(msg), NULL); //�������� ������� ��������� msg
			Connections[i] = newConnection; //������ ������ � ������
			Counter++;
			//�������� ������ ������ ��� ������ ����������� ����� ���������
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL); 
		}
	}

	return 0;
}

