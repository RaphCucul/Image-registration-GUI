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

void analyzuj_jmena_souboru_avi(QString vybrana_cesta_k_souborum,
                                QStringList &seznam_jmen_souboru,
                                int &celkovy_pocet_souboru_s_koncovkou,
                                QString koncovka_co_hledam)
{
     QDir chosenDirectory(vybrana_cesta_k_souborum);
     seznam_jmen_souboru = chosenDirectory.entryList(QStringList() << "*."+koncovka_co_hledam << "*."+koncovka_co_hledam.toUpper(),QDir::Files);
     celkovy_pocet_souboru_s_koncovkou = seznam_jmen_souboru.size();
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
