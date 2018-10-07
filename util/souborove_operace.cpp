#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <shlobj.h>
#include <getopt.h>
#include <windows.h>
#include <iomanip>
#include <stdio.h>
#include <math.h>
#include <QDir>
#include "souborove_operace.h"
using namespace std;

void analyzuj_jmena_souboru_avi(string &vybrana_cesta_k_souborum,
                                std::vector<string> &seznam_jmen_souboru,
                                int &celkovy_pocet_souboru_s_koncovkou,
                                string koncovka_co_hledam, string koncovka_kterou_nalezl)
{
    /// vyuziti dirent.h knihovny
    DIR *dir; //otevøení directory streamu
    struct dirent *ent; // struct dirent je definovaná struktura s prvky ino_t d_ino, char d_name[] a dalšími
    // vice na https://en.wikibooks.org/wiki/C_Programming/POSIX_Reference/dirent.h
    string jmeno_souboru;
    string koncovka_hledana = koncovka_co_hledam;
    string koncovka_nalezena = koncovka_kterou_nalezl;
    if ((dir = opendir(vybrana_cesta_k_souborum.c_str()))!=NULL)   //opendir otevírá directory stream korespondující
    {
        // se jménem, zde se kontroluje, zda lze vùbec složku otevøít - tedy jestli vùbec existuje, èi není pøístup
        // nìjak omezen
        ///opendir vrací ukazatel na objekt typu DIR
        while ((ent = readdir (dir)) != NULL)
        {
            ///readdir vrací ukazatel na objekt struktruy dirent ukazující na konkrétní vstup ve složce dir

            //printf ("%s\n", ent->d_name); // výpis všeho, co se ve složce nachází
            jmeno_souboru = string(ent->d_name); //jmeno aktualniho prvku ve složce
            koncovka_nalezena = jmeno_souboru;
            if (jmeno_souboru.length()>4) //ent->d_namelen
            {
                koncovka_nalezena = koncovka_nalezena.assign(jmeno_souboru,(jmeno_souboru.length())-3,3);
                if (koncovka_hledana == koncovka_nalezena)
                {
                    // segmentace pripony souboru
                    celkovy_pocet_souboru_s_koncovkou += 1;
                    size_t lastindex = jmeno_souboru.find_last_of(".");
                    jmeno_souboru = jmeno_souboru.substr(0, lastindex);
                    seznam_jmen_souboru.push_back(jmeno_souboru);
                }
                //cout << jmeno_souboru << endl;
            }
        }
        //closedir (dir);
    }
    else
    {
        cerr << "Slozku nelze otevrit" << endl;
    }
}
int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{ ///https://msdn.microsoft.com/en-us/library/windows/desktop/bb762598(v=vs.85).aspx
    if(uMsg == BFFM_INITIALIZED)
    {
        std::string tmp = (const char *) lpData;
        //std::cout << "path: " << tmp << std::endl;
        SendMessage(hwnd, BFFM_SETSELECTION, true, (LPARAM) lpData); // posílá zprávu do dialogového boxu, díky cemuz
        // lze kontrolovat OK tlacítko, zmenu velikosti okna a dalsi
    }
    return 0;
}
// funkce, ktera zjisti aktualne pouzivanou slozku - tedy tu, kde se prave nachazi pouzivany soubor
void ZobrazPostup( char titulek[], int krok, int celkem )
{
    HANDLE h = GetStdHandle ( STD_OUTPUT_HANDLE );
    WORD wOldColorAttrs;
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    GetConsoleScreenBufferInfo(h, &csbiInfo); // ulozeni aktulaniho nastaveni konzole
    wOldColorAttrs = csbiInfo.wAttributes;

    const int postup_sirka = 50; // jak siroky bude cely procentualni postup v cmd
    int sirka = postup_sirka - strlen(titulek); // uprava sirky odectenim titulku
    int pos = ( krok * sirka ) / celkem ;
    int procento = (krok/celkem)*100;
    cout << "\r" << procento << "% dokonceno";
    SetConsoleTextAttribute(h,FOREGROUND_GREEN);
    printf( "%s[", titulek );
    // tisknou se = symboly
    for ( int i = 0; i < pos; i++ )
    {printf( "%c", '=' );}

    // pridavani mezer
    printf( "% *c", sirka - pos + 1, ']' );
    printf( " %3d%%\r", procento );

    //reset text color, only on Windows
    SetConsoleTextAttribute ( h, wOldColorAttrs);
    //SetConsoleTextAttribute(  GetStdHandle( STD_OUTPUT_HANDLE ), 0x08 );
}

void zpracujJmeno(QString& celeJmeno,QString& slozka,QString& zkraceneJmeno,QString& koncovka)
{
    int lastindexSlash = celeJmeno.lastIndexOf("/");
    int lastIndexDot = celeJmeno.length() - celeJmeno.lastIndexOf(".");
    slozka = celeJmeno.left(lastindexSlash);
    zkraceneJmeno = celeJmeno.mid(lastindexSlash+1,
         (celeJmeno.length()-lastindexSlash-lastIndexDot-1));
    koncovka = celeJmeno.right(lastIndexDot-1);
}
