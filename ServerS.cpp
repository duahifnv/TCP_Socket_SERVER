#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)

#define MAX_CONNECTIONS 100
#define MSG_MAX_SIZE 256
using namespace std;



//создание массива сокетов (100 соединений)
SOCKET Connections[MAX_CONNECTIONS];
int Counter = 0;

enum Packet {
	P_ChatMessage,
	P_Test
};

//функция обработки пакетов
bool ProccessPacket(int index, Packet packettype) {
	switch (packettype) {
	case(P_ChatMessage): {
		int msg_size; //размер принимаемого сообщения
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL); //принимаем размер с-ния
		char* msg = new char[msg_size + 1]; //выделяем память под массив char
		msg[msg_size] = '\0'; //нуль терминатор

		recv(Connections[index], msg, msg_size, NULL); //принимаем само сообщение

		if (msg == "exit") {
			cout << "Client " << index + 1 << " disconnected!\n";
		}

		for (int i = 0; i < Counter; i++) {
			if (i == index) {
				continue;
			}

			Packet msgtype = P_ChatMessage;
			send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL);
			send(Connections[i], (char*)&msg_size, sizeof(int), NULL); //отправляем размер с-ния
			send(Connections[i], msg, msg_size, NULL); //отправляем сообщение
		}
		delete[] msg; //очищаем память

		break;
	}
	default: {
		cout << "Unrecognized packet: " << packettype << endl;
		break;
	}
	}
	return true;
}

//функция удержания соединения между клиентом и сервером
void ClientHandler(int index) {
	Packet packettype;
	while (true) {
		recv(Connections[index], (char*)&packettype, sizeof(Packet), NULL);
		if (!ProccessPacket(index, packettype)) {
			break;
		}
	}
	closesocket(Connections[index]); //закрываем сокет если не можем обработать пакет
}

int main() {
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
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); //привязка адреса к сокету
	listen(sListen, SOMAXCONN); //прослушивание порта для подключения клиента к серверу

	SOCKET newConnection; //создание нового сокета для удержания соединения с клиентом

	//через цикл проверяем подключения 100 клиентов
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		//проверка подключения клиента к серверу
		if (newConnection == 0) {
			cout << "cant connect new client!" << endl;

		}
		else {
			cout << "Client " << i + 1 << " Connected!\n";
			string msg = "***WELCOME TO THE SERVER***";
			int msg_size = msg.size();

			//отправка типа пакета
			Packet msgtype = P_ChatMessage;
			send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);

			//отправка приветственного сообщения
			send(newConnection, (char*)&msg_size, sizeof(int), NULL);
			send(newConnection, msg.c_str(), msg_size, NULL);

			Connections[i] = newConnection; //запись сокета в массив
			Counter++;

			//создание нового потока для обмена сообщениями между клиентами
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

			Packet testpacket = P_Test;
			send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);
		}
	}

	/*closesocket(newConnection);
	WSACleanup();*/
	return 0;
}

