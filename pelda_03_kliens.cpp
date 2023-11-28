#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma comment (lib, "Ws2_32.lib")

//Segédváltozók
#define BUFLEN 100000
char recvbuf[BUFLEN] = "";
char sendbuf[BUFLEN] = "";
//Főprogram
int __cdecl main(int argc, char** argv)
{
  WSADATA wsaData;
  SOCKET ConnectSocket = INVALID_SOCKET;

  strcpy_s(sendbuf, "Teszt keres");

  int Result_error;

  // Winsock inicializalasa
  Result_error = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (Result_error != 0) {
    printf("Inicializalasi hiba: %d\n", Result_error);
    return 1;
  }

  //Kapcsolat felepitese
  ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  sockaddr_in clientService;
  clientService.sin_family = AF_INET;
  clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
  clientService.sin_port = htons(80);
  Result_error = connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));


  if (ConnectSocket == INVALID_SOCKET) {
    printf("Kapcsolat nem jott letre!\n");
    WSACleanup();
    return 1;
  }

  // Adatkuldes
  Result_error = send(ConnectSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
  if (Result_error == SOCKET_ERROR) {
    printf("Adatkuldesi hiba: %d\n", WSAGetLastError());
    closesocket(ConnectSocket);
    WSACleanup();
    return 1;
  }
  printf("Elkuldott bajtok szama: %ld\n", Result_error);

  // Adatkuldes befejezese
  Result_error = shutdown(ConnectSocket, SD_SEND);
  if (Result_error == SOCKET_ERROR) {
    printf("Lezarasi hiba: %d\n", WSAGetLastError());
    closesocket(ConnectSocket);
    WSACleanup();
    return 1;
  }

  // Valasz osszeszedese
  do {

    Result_error = recv(ConnectSocket, recvbuf, BUFLEN, 0);
    if (Result_error > 0)
    {
      printf("Fogadott bajtok szama: %d\n", Result_error);
      printf("%s\n", recvbuf);
    }
    else if (Result_error == 0)
      printf("Kapcsolat lezarva\n");
    else
      printf("Adatfogadasi hiba: %d\n", WSAGetLastError());

  } while (Result_error > 0);

  // Befejezes
  closesocket(ConnectSocket);
  WSACleanup();

  return 0;
}
