#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <fstream>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)

#define MAX_CONNECTIONS 100
#define MSG_MAX_SIZE 256
using namespace std;



//�������� ������� ������� (100 ����������)
SOCKET Connections[MAX_CONNECTIONS];
int Counter = 0;

//���������� �������� ����������
FILE* fb;

enum Packet {
	P_ChatMessage,
	P_GetFile
};

//������� ���������� ����� � ��� ��������� �������� �������
void Readf(FILE* fb, string fullname, int index) {
	bool msg_is_readed = true;
	int mir_size = sizeof(bool);

	char rmessage[100];
	byte smessage[4096]{ NULL };
	int Elements;

	//if (fread(smessage, sizeof(BYTE), 4096, fb) == NULL) {
	//	msg_is_readed = false;
	//	cout << "Can't read from a file: " << fullname << endl;
	//	send(Connections[index], (char*)&mir_size, sizeof(int), NULL); //7
	//	send(Connections[index], (char*)&msg_is_readed, sizeof(bool), NULL); //8
	//}	

	//else {
		send(Connections[index], (char*)&mir_size, sizeof(int), NULL); //7
		send(Connections[index], (char*)&msg_is_readed, sizeof(bool), NULL); //8

		while ((Elements = fread(smessage, sizeof(BYTE), 4096, fb)) != NULL)
		{
			cout << "Sending file: " << fullname << " which contains next text: " << smessage << endl;
			send(Connections[index], (char*)smessage, Elements, NULL); //9
		}
	//}
}

//������� ���������� ����� � �������
bool OpenFile(const char* filename, int index) {
	const char* folder = "./files/";
	string fullname(folder);
	fullname.append(filename);

	if ((fb = fopen(fullname.c_str(), "rb")) == NULL) {
		cout << "Can't open " << filename << endl;
		//fclose(fb);
		return false;
	}
	else {
		cout << "File " << fullname << " opened!\n";
		return true;
	}

	/*ifstream fin;
	fin.open(fullname);
	if (!fin.is_open()) {
		fin.close();
		return false;
	}
	else {
		fin.close();
		return true;
	}*/


}

//������� ��������� �������
bool ProccessPacket(int index, Packet packettype) {
	switch (packettype) {
	case(P_ChatMessage): {
		int msg_size; //������ ������������ ���������
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL); //2 ��������� ������ �-���
		char* msg = new char[msg_size + 1]; //�������� ������ ��� ������ char
		msg[msg_size] = '\0'; //���� ����������

		recv(Connections[index], msg, msg_size, NULL); //3 ��������� ���� ���������

		if (msg == "exit") {
			cout << "Client " << index + 1 << " disconnected!\n";
		}

		/*-----------------------SENDING MSG TO EVERY CLIENT-------------------*/
		for (int i = 0; i < Counter; i++) {
			if (i == index) {
				continue;
			}

			Packet msgtype = P_ChatMessage;
			send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL); //4

			send(Connections[i], (char*)&msg_size, sizeof(int), NULL); //5 ���������� ������ �-���

			send(Connections[i], msg, msg_size, NULL); //6 ���������� ���������
		}
		delete[] msg; //������� ������
		/*---------------------------------------------------------------------*/
		break;
	}

	case(P_GetFile): {
		int Fname_size; //������ ������������ ���������
		recv(Connections[index], (char*)&Fname_size, sizeof(int), NULL); //2 ��������� ������ �-���
		char* Fname = new char[Fname_size + 1]; //�������� ������ ��� ������ char
		Fname[Fname_size] = '\0'; //���� ����������

		recv(Connections[index], Fname, Fname_size, NULL); //3 ��������� ���� ���������

		Packet msgtype = P_GetFile;
		send(Connections[index], (char*)&msgtype, sizeof(Packet), NULL); //4

		/*-----------------SENDING MSG ABOUT FILE FOUND AND OPENED ON SERVER-------------------*/
		
		bool File_is_found = OpenFile(Fname, index); //file found/not found
		int FisF_size = sizeof(bool);

		send(Connections[index], (char*)&FisF_size, sizeof(int), NULL); //5
		send(Connections[index], (char*)&File_is_found, FisF_size, NULL); //6

		/*-----------------SENDING MSG ABOUT FILE READED AND DELIVERED TO CLIENT-------------------*/
		
		if (File_is_found) {
			Readf(fb, Fname, index);
		}

		cout << "Successfully send file to client " << index + 1 << endl;
		/*if (Fname == "exit") {
			cout << "Client " << index + 1 << " disconnected!\n";
		}*/

		break;
	}
	default: {
		cout << "Client disconnected!" << endl;
		return false;
	}
	}
	return true;
}

//������� ��������� ���������� ����� �������� � ��������
void ClientHandler(int index) {
	Packet packettype;
	while (true) {
		recv(Connections[index], (char*)&packettype, sizeof(Packet), NULL); //1
		if (!ProccessPacket(index, packettype)) {
			break;
		}
	}
	closesocket(Connections[index]); //��������� ����� ���� �� ����� ���������� �����
}

int main() {
	setlocale(LC_ALL, "Russian");

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

	cout << "Waiting for new connections\n\n";
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

			//�������� ���� ������
			Packet msgtype = P_ChatMessage;
			send(newConnection, (char*)&msgtype, sizeof(Packet), NULL); //1

			//�������� ��������������� ���������
			send(newConnection, (char*)&msg_size, sizeof(int), NULL); //2
			send(newConnection, msg.c_str(), msg_size, NULL); //3

			Connections[i] = newConnection; //������ ������ � ������
			Counter++;

			//�������� ������ ������ ��� ������ ����������� ����� ���������
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

			/*Packet testpacket = P_Test;
			send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);*/
		}
	}

	for (int index = 0; index < Counter; index++) {
		closesocket(Connections[index]);
	}

	WSACleanup();
	return 0;
}