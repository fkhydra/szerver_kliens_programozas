#include <Windows.h>
#include <iostream>
#include <WinInet.h>
#include <string.h>
#include <stdio.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "user32.lib")

//Segédváltozók
unsigned char buffer[1024 * 1024 ];
unsigned char s_token[128];
unsigned int szoveg_poz=0;
unsigned char kimenet[1024 * 1024];

void download_URL(char* eleresiut);
unsigned int search_token(unsigned char* szoveg, unsigned char* token, unsigned int startpos);
unsigned int parse_text(unsigned char* szoveg, unsigned char* token, unsigned int startpos);

//Főprogram
int main(int argc, char* argv[])
{
 if (argc != 3) printf("Hasznalat: PROGAMNEV URL TOKEN\n");
 else
 {
  strcpy((char*)s_token, argv[2]);
  download_URL(argv[1]);
  //printf("%s\n",buffer);
  do
  {//kereses vegrehajtasa a letoltott eroforrasban
   szoveg_poz = search_token(buffer, s_token, szoveg_poz + strlen((const char*)s_token));
   if (szoveg_poz != 0) parse_text2(buffer, s_token, szoveg_poz);
  } while (szoveg_poz != 0);
 } 
 
 return 0;
}

//Erőforrás letöltése
void download_URL(char* eleresiut)
{
 HINTERNET internet = InternetOpenA("Letolto_kereso", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
 if (internet)
 {
  //A kapcsolat letrehozasa
  HINTERNET file_handle = InternetOpenUrlA(internet, eleresiut, NULL, 0, INTERNET_FLAG_RAW_DATA, 0);
  if (file_handle)
  {
   DWORD bytes_read = 0;

   //A tavoli eroforras letoltese
   InternetReadFile(file_handle, buffer, 1000000, &bytes_read);
  }

  InternetCloseHandle(internet);
 }
}

//Tag keresése az erőforrásban
unsigned int search_token(unsigned char* szoveg, unsigned char* token, unsigned int startpos)
{
 int i, j,token_talalat;//0-nincs,1-van

 for (i = startpos; i < strlen((const char*)szoveg); ++i)
 {
  token_talalat = 1;
  for (j = 0; j < strlen((const char*)token); ++j)
  {
   if (szoveg[i + j] != token[j])
   {
    token_talalat = 0;
    break;
   }
  }

  //talalat
  if (token_talalat == 1)
  {
   return i;
  }
 }

 return 0;
}

//Szöveg kikeresése tag-ből
unsigned int parse_text2(unsigned char* szoveg, unsigned char* token, unsigned int startpos)
{
 int i, j = 0;//0-nincs,1-van 

 for (i = 0; i < 1024 * 1024; ++i) kimenet[i] = 0;
 for (i = startpos; i < strlen((const char*)szoveg); ++i)
 {
  if (szoveg[i] == '>' or szoveg[i] == '\n' or szoveg[i] == 0) break;
  else kimenet[j++] = szoveg[i];
  if (j == (1024 * 1024) - 1) break;
 }

 printf("%s\n", kimenet);
}
