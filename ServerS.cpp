#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)

#define MAX_CONNECTIONS 100
#define MSG_MAX_SIZE 256
using namespace std;



//�������� ������� ������� (100 ����������)
SOCKET Connections[MAX_CONNECTIONS];
int Counter = 0;


//�������� ������� ������ ����������� ����� ���������
void ClientHandler(int index) {
	int msg_size; //������ ������������ ���������
	while (true) {
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL); //��������� ������ �-���
		char* msg = new char[msg_size + 1]; //�������� ������ ��� ������ char
		msg[msg_size] = '\0'; //���� ����������

		recv(Connections[index], msg, msg_size, NULL); //��������� ���� ���������

		if (msg=="exit") {
			cout << "Client " << index + 1 << " disconnected!\n";
		}

		for (int i = 0; i < Counter; i++) {
			if (i == index) continue;

			send(Connections[i], (char*)&msg_size, sizeof(int), NULL); //���������� ������ �-���
			send(Connections[i], msg, msg_size, NULL); //���������� ���������
		}
		delete[] msg; //������� ������
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
			cout << "Client " << i + 1 << " Connected!\n";
			string msg = "***WELCOME TO THE SERVER***";
			int msg_size = msg.size();
			send(newConnection, (char*)&msg_size, sizeof(int), NULL);
			send(newConnection, msg.c_str(), msg_size, NULL); //�������� ������� ��������� msg

			Connections[i] = newConnection; //������ ������ � ������
			Counter++;
			//�������� ������ ������ ��� ������ ����������� ����� ���������
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL); 
		}
	}

	closesocket(newConnection);
	WSACleanup();
	return 0;
}

