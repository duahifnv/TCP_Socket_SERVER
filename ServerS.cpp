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



//создание массива сокетов (100 соединений)
SOCKET Connections[MAX_CONNECTIONS];
int Counter = 0;

//глобальна€ файлова€ переменна€
FILE* fb;

enum Packet {
	P_ChatMessage,
	P_GetFile
};

//функци€ считывани€ файла и его побитовой отправки клиенту
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

//функци€ нахождени€ файла в системе
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

//функци€ обработки пакетов
bool ProccessPacket(int index, Packet packettype) {
	switch (packettype) {
	case(P_ChatMessage): {
		int msg_size; //размер принимаемого сообщени€
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL); //2 принимаем размер с-ни€
		char* msg = new char[msg_size + 1]; //выдел€ем пам€ть под массив char
		msg[msg_size] = '\0'; //нуль терминатор

		recv(Connections[index], msg, msg_size, NULL); //3 принимаем само сообщение

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

			send(Connections[i], (char*)&msg_size, sizeof(int), NULL); //5 отправл€ем размер с-ни€

			send(Connections[i], msg, msg_size, NULL); //6 отправл€ем сообщение
		}
		delete[] msg; //очищаем пам€ть
		/*---------------------------------------------------------------------*/
		break;
	}

	case(P_GetFile): {
		int Fname_size; //размер принимаемого сообщени€
		recv(Connections[index], (char*)&Fname_size, sizeof(int), NULL); //2 принимаем размер с-ни€
		char* Fname = new char[Fname_size + 1]; //выдел€ем пам€ть под массив char
		Fname[Fname_size] = '\0'; //нуль терминатор

		recv(Connections[index], Fname, Fname_size, NULL); //3 принимаем само сообщение

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

//функци€ удержани€ соединени€ между клиентом и сервером
void ClientHandler(int index) {
	Packet packettype;
	while (true) {
		recv(Connections[index], (char*)&packettype, sizeof(Packet), NULL); //1
		if (!ProccessPacket(index, packettype)) {
			break;
		}
	}
	closesocket(Connections[index]); //закрываем сокет если не можем обработать пакет
}

int main() {
	setlocale(LC_ALL, "Russian");

	WSAData wsadata;
	WORD DLLVersion = MAKEWORD(2, 1);

	//запуск протокола WSAStartup и проверка запуска
	if (WSAStartup(DLLVersion, &wsadata) != 0) {
		cerr << "no wsa sry :(" << endl;
		exit(1);
	}

	//настройка адреса сокета
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);

	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); //создание сокета
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); //прив€зка адреса к сокету
	listen(sListen, SOMAXCONN); //прослушивание порта дл€ подключени€ клиента к серверу

	SOCKET newConnection; //создание нового сокета дл€ удержани€ соединени€ с клиентом

	cout << "Waiting for new connections\n\n";
	//через цикл провер€ем подключени€ 100 клиентов
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		//проверка подключени€ клиента к серверу
		if (newConnection == 0) {
			cout << "cant connect new client!" << endl;

		}
		else {
			cout << "Client " << i + 1 << " Connected!\n";
			string msg = "***WELCOME TO THE SERVER***";
			int msg_size = msg.size();

			//отправка типа пакета
			Packet msgtype = P_ChatMessage;
			send(newConnection, (char*)&msgtype, sizeof(Packet), NULL); //1

			//отправка приветственного сообщени€
			send(newConnection, (char*)&msg_size, sizeof(int), NULL); //2
			send(newConnection, msg.c_str(), msg_size, NULL); //3

			Connections[i] = newConnection; //запись сокета в массив
			Counter++;

			//создание нового потока дл€ обмена сообщени€ми между клиентами
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