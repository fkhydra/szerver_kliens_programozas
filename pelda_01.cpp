#include <Windows.h>
#include <iostream>
#include <WinInet.h>
#include <stdio.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "user32.lib")

char buffer[1000000];

void download_URL(char *eleresiut, char* fajlnev);

//Főprogram
int main(int argc, char *argv[])
{
  if (argc != 3) printf("Hasznalat: PROGAMNEV URL FAJLNEV\n");
  else download_URL(argv[1], argv[2]);
  return 0;
}

//Erőforrás letöltése
void download_URL(char* eleresiut, char* fajlnev)
{
  //A kapcsolat letrehozasa
  HINTERNET internet = InternetOpenA("Letolto", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
  if (internet)
  {
    HINTERNET file_handle = InternetOpenUrlA(internet, eleresiut, NULL, 0, INTERNET_FLAG_RAW_DATA, 0);
    if (file_handle)
    {
      DWORD bytes_read = 0;
      
      //A tavoli eroforras letoltese
      InternetReadFile(file_handle, buffer, 1000000, &bytes_read);

      //fájl kiírása
      FILE* fp;
      fopen_s(&fp, fajlnev, "wb");
      if (fp != NULL)
      {
        fwrite(buffer, bytes_read, 1, fp);
        fclose(fp);
      }
    }

    InternetCloseHandle(internet);
  }
}
