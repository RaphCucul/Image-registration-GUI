#ifndef SOUBOROVE_OPERACE_H_INCLUDED
#define SOUBOROVE_OPERACE_H_INCLUDED
#include <vector>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <shlobj.h>
#include <getopt.h>
#include <windows.h>
#include <iomanip>

#include <QString>
using namespace std;
void analyzuj_jmena_souboru_avi(std::string &vybrana_cesta_k_souborum,
                                std::vector<string> &seznam_jmen_souboru,
                                int &celkovy_pocet_souboru_s_koncovkou,
                                std::string koncovka_co_hledam,
                                std::string koncovka_kterou_nalezl);
/// funkce analyzujici obsah slozky a hledajici soubory se zadanou koncovkou, vyuziva nize uvedene funkce

std::string GetFileName( const std::string & prompt );
int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData);
string BrowseFolder(string saved_path);
string ExePath();
void zpracujJmeno(QString& celeJmeno,QString& slozka,QString& zkraceneJmeno,QString& koncovka);
/// funkce jsou zalozene na dokumentaci k dirent.h knihovne a z msdn.microsoft.com
void ZobrazPostup( char titulek[], int krok, int celkem );
/// nevyuziva se
#endif // SOUBOROVE_OPERACE_H_INCLUDED
