#include "framework.h"
#include "pelda_05.h"
#include <WinInet.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include <string.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ws2_32.lib")
#include <commctrl.h>

HANDLE THANDLER;

#define HIBA_00 TEXT("Error:Program initialisation process.")
HINSTANCE hInstGlob;
int SajatiCmdShow;
char szClassName[] = "WindowsApp";
HWND Form1; //Ablak kezelője
int vege = 0;

WSADATA wsaData;
WORD wVersionRequested = MAKEWORD(2, 2);
BOOL serverisopen;
SOCKET clientsocket, serversocket;
sockaddr_in serveraddr, clientaddr;
FILE* flog;
char localinfo[128], htmlresp[5000];
int i, nLength = sizeof(SOCKADDR);

LRESULT CALLBACK WndProc0(HWND, UINT, WPARAM, LPARAM);
void varakozas(void* pParams);

//felugró üzenetablakok
void ShowMessage(LPCTSTR uzenet, LPCTSTR cim, HWND kuldo);

HWND Button1;
#define OBJ_ID100 100

void log(const char* puffer, FILE* fp);
void open_server(void);
void start_server_listening(void);
void close_server(void);

//*********************************
//A windows program belépési pontja
//*********************************
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("StdWinClassName");
    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass0;
    SajatiCmdShow = iCmdShow;
    hInstGlob = hInstance;
    LoadLibrary(L"COMCTL32.DLL");

    //*********************************
    //Ablak osztálypéldány előkészítése
    //*********************************
    wndclass0.style = CS_HREDRAW | CS_VREDRAW;
    wndclass0.lpfnWndProc = WndProc0;
    wndclass0.cbClsExtra = 0;
    wndclass0.cbWndExtra = 0;
    wndclass0.hInstance = hInstance;
    wndclass0.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass0.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass0.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);//LTGRAY_BRUSH
    wndclass0.lpszMenuName = NULL;
    wndclass0.lpszClassName = TEXT("WIN0");

    //*********************************
    //Ablak osztálypéldány regisztrációja
    //*********************************
    if (!RegisterClass(&wndclass0))
    {
        MessageBox(NULL, HIBA_00, TEXT("Program Start"), MB_ICONERROR); return 0;
    }

    //*********************************
    //Ablak létrehozása
    //*********************************
    Form1 = CreateWindow(TEXT("WIN0"),
        TEXT("Mini szerver"),
        (WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX),
        50,
        50,
        400,
        300,
        NULL,
        NULL,
        hInstance,
        NULL);

    //*********************************
    //Ablak megjelenítése
    //*********************************
    ShowWindow(Form1, SajatiCmdShow);
    UpdateWindow(Form1);

    //*********************************
    //Ablak üzenetkezelésének aktiválása
    //*********************************
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

//*********************************
//Az ablak callback függvénye: eseménykezelés
//*********************************
LRESULT CALLBACK WndProc0(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;

    switch (message)
    {
        //*********************************
        //Ablak létrehozásakor közvetlenül
        //*********************************
    case WM_CREATE:
        /*Init*/;
        Button1 = CreateWindow(TEXT("button"), TEXT("Szerver bekapcsolása")
            , WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_MULTILINE, 50, 50, 170, 30
            , hwnd, (HMENU)(OBJ_ID100), ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        fopen_s(&flog, "LOG.txt", "wt");
        fclose(flog);
        //open_server();
        return 0;
    //*********************************
    //vezérlőelem működtetése (többnyire kattintás)
    //*********************************
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case OBJ_ID100:
            THANDLER = (HANDLE)_beginthread(varakozas, 0, NULL);
            break;
        }

        return 0;
        //*********************************
//Ablak átméretezése
//*********************************
    case WM_SIZE:
        return 0;

        //*********************************
        //Ablak kliens területének újrarajzolása
        //*********************************
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        return 0;
        //*********************************
        //Ablak bezárása
        //*********************************
    case WM_CLOSE:
        vege = 1;
        DestroyWindow(hwnd);
        return 0;
        //*********************************
        //Ablak megsemmisítése
        //*********************************
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//*********************************
//Üzenetablak OK gombbal
//*********************************
void ShowMessage(LPCTSTR uzenet, LPCTSTR cim, HWND kuldo)
{
    MessageBox(kuldo, uzenet, cim, MB_OK);
}

void log(const char* puffer, FILE* fp)
{
    char logbuffer[2048];
    int meret = sizeof(&puffer);
    //strcpy_s(logbuffer, puffer);
    fopen_s(&flog, "LOG.txt", "at");
    //fwrite(puffer,sizeof(puffer),1,flog);
    fprintf_s(fp, puffer);
    fclose(flog);
}

void open_server(void)
{
    serverisopen = false;
    clientsocket = INVALID_SOCKET;
    serversocket = INVALID_SOCKET;

    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        WSACleanup();
        log("Winsock init error...\n", flog);
        return;
    }

    log("Winsock init OK.\n", flog);
    log(wsaData.szDescription, flog); log("\n", flog);

    gethostname(localinfo, sizeof(localinfo));
    log(localinfo, flog); log("\n", flog);
    log("PORT: 80\n", flog);

    serversocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serversocket == INVALID_SOCKET)
    {
        WSACleanup();
        log("Couldn't create server socket...\n", flog);
    }
    log("Server socket created...\n", flog);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(80);
    if (bind(serversocket, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
    {
        WSACleanup();
        serversocket = INVALID_SOCKET;
        log("Couldn't bind server socket...\n", flog);
        return;
    }
    else log("Binding server socket OK...\n", flog);
}

void start_server_listening(void)
{
    int i;
    int iResult;
    int iSendResult;
    char recvbuf[2048];
    int recvbuflen = 2048;

    for (i = 0; i < recvbuflen; ++i) recvbuf[i] = '\0';
    if (serversocket == INVALID_SOCKET) return;

    SetWindowTextA(Form1, "Bejövő kérések figyelése...");
    if (listen(serversocket, 1) == SOCKET_ERROR) //max 1 kapcsolat        
    {
        WSACleanup();
        serversocket = INVALID_SOCKET;
        log("Server socket FAILED to listen...\n", flog);
        return;
    }
    else
    {
        log("Server socket is listening...\n", flog);
        serverisopen = true;
    }

    clientsocket = accept(serversocket, (SOCKADDR*)&clientaddr, (LPINT)&nLength);
    //clientsocket = accept(serversocket, NULL, NULL);
    if (clientsocket == INVALID_SOCKET)
        log("Server socket failed to accept connection...\n", flog);
    else
    {
        log("CONNECTION OK...\n", flog);
        SetWindowTextA(Form1, "Kliens kapcsolódott!");

        

        log("\n*****Collecting request******\n", flog);
        do {

            iResult = recv(clientsocket, recvbuf, recvbuflen - 2, 0);
            if (iResult > 0) {
                log("Reading request...\n\n", flog);
                int meret = sizeof(recvbuf);
                log("Client request:\n", flog);
                log(recvbuf, flog);
                log("\n\n", flog);
                shutdown(clientsocket, SD_RECEIVE);
                strcpy_s(htmlresp, "<!doctype HTML>\n<html><h3>A szerverkapcsolat létrejött!</h3></html>\0");
                
                //connect(clientsocket,(SOCKADDR *)&clientaddr, sizeof(clientaddr));
                iResult = send(clientsocket, htmlresp, (int)strlen(htmlresp) + 1, 0);
                if (iResult == SOCKET_ERROR)
                    log("Sending response FAILED...\n", flog);
                else
                {
                    log("Response sent OK...\n", flog);
                    shutdown(clientsocket, SD_SEND);
                }
                closesocket(clientsocket);
                closesocket(serversocket);
                WSACleanup();
                break;
            }
            else if (iResult == 0)
                log("Connection closing...\n", flog);
            else {
                log("Reading request failed with error\n", flog);
                closesocket(clientsocket);
                WSACleanup();
                return;
            }

        } while (iResult > 0);        

        log("*****End of request******\n\n", flog);
    }
}

void close_server(void)
{
    log("\nEnding...\n", flog);

    log("WSA cleanup...\n", flog);
    shutdown(clientsocket, SD_SEND);
    if (serverisopen == true) closesocket(serversocket);
    if (clientsocket != INVALID_SOCKET) closesocket(clientsocket);
    WSACleanup();
    serverisopen = false;
    clientsocket = INVALID_SOCKET;
    serversocket = INVALID_SOCKET;
    log("Sockets closed...\n", flog);
}

void varakozas(void* pParams)
{
    ShowWindow(Button1, SW_HIDE);
    while (1)
    {
        if (vege == 1) break;
        open_server();
        if (vege == 1) break;
        start_server_listening();
        if (vege == 1) break;
        close_server();
    }
}