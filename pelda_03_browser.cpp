#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 64000
#define DEFAULT_PORT "27015"

//Segédváltozók
int iResult;
int iSendResult;
char recvbuf[DEFAULT_BUFLEN];
int recvbuflen = DEFAULT_BUFLEN;

WSADATA wsaData;
SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ServerSocket = INVALID_SOCKET;

struct addrinfo* result = NULL;
struct addrinfo server_config;

//Főprogram
int __cdecl main(void)
{
 // Inicializalas
 iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
 if (iResult != 0) {
  printf("WSAStartup failed with error: %d\n", iResult);
  return 1;
 }

 ZeroMemory(&server_config, sizeof(server_config));
 server_config.ai_family = AF_INET;
 server_config.ai_socktype = SOCK_STREAM;
 server_config.ai_protocol = IPPROTO_TCP;
 server_config.ai_flags = AI_PASSIVE;

 // Adatlekerdezes a socket letrehozasahoz
 iResult = getaddrinfo(NULL, DEFAULT_PORT, &server_config, &result);
 if (iResult != 0) {
  printf("getaddrinfo hiba: %d\n", iResult);
  WSACleanup();
  return 1;
 }

 // socket létrehozása
 ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
 if (ListenSocket == INVALID_SOCKET) {
  printf("socket letrehozasi hiba: %ld\n", WSAGetLastError());
  freeaddrinfo(result);
  WSACleanup();
  return 1;
 }

 sockaddr_in serveraddr;
 serveraddr.sin_family = AF_INET;
 serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
 serveraddr.sin_port = htons(80);

 // A keresfigyelo socket letrehozasa
 iResult = bind(ListenSocket, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
 //iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
 if (iResult == SOCKET_ERROR) {
  printf("bind hiba: %d\n", WSAGetLastError());
  freeaddrinfo(result);
  closesocket(ListenSocket);
  WSACleanup();
  return 1;
 }

 freeaddrinfo(result);

 //Bejovo keresek figyelese
 printf("Listening...\n");
 iResult = listen(ListenSocket, SOMAXCONN);
 if (iResult == SOCKET_ERROR) {
  printf("Figyelesi hiba: %d\n", WSAGetLastError());
  closesocket(ListenSocket);
  WSACleanup();
  return 1;
 }

 // a keres feldolgozasanak kezdete
 ServerSocket = accept(ListenSocket, NULL, NULL);
 if (ServerSocket == INVALID_SOCKET) {
  printf("Fogadasi hiba: %d\n", WSAGetLastError());
  closesocket(ListenSocket);
  WSACleanup();
  return 1;
 }

 // a figyeles vege
 closesocket(ListenSocket);

 // Adatok fogadasa, amig a kuldo fel le nem zarja a folyamatot
 do {

  iResult = recv(ServerSocket, recvbuf, recvbuflen, 0);
  if (iResult > 0) {
   printf("Bajtok szama: %d\n", iResult);

   // Echo the buffer back to the sender
   iSendResult = send(ServerSocket, recvbuf, iResult, 0);
   if (iSendResult == SOCKET_ERROR) {
    printf("Kuldesi hiba: %d\n", WSAGetLastError());
    closesocket(ServerSocket);
    WSACleanup();
    return 1;
   }
   printf("Bajtok szama: %d\n", iSendResult);
   printf("%s\n",recvbuf);
   //shutdown(ServerSocket, SD_SEND);
  }
  else if (iResult == 0)
   printf("Kapcsolat lezarasa...\n");
  else {
   printf("Fogadasi hiba: %d\n", WSAGetLastError());
   closesocket(ServerSocket);
   WSACleanup();
   return 1;
  }

 } while (iResult > 0);

 // Kapcsolat lezarasa
 iResult = shutdown(ServerSocket, SD_SEND);
 if (iResult == SOCKET_ERROR) {
  printf("Leallasi hiba: %d\n", WSAGetLastError());
  closesocket(ServerSocket);
  WSACleanup();
  return 1;
 }

 // szerver megszüntetése
 closesocket(ServerSocket);
 WSACleanup();
 system("PAUSE");

 return 0;
}
