#include "qthreadfourthpart.h"
#include "util/souborove_operace.h"

#include <QDebug>
#include <QThread>
#include <QObject>
#include <QStringList>
#include <QVector>

qThreadFourthPart::qThreadFourthPart(QStringList &sV,
                                     QVector<QVector<int> > &sPrvniRozhodovaniK,
                                     QVector<QVector<int> > &ohodKomplet,
                                     QVector<QVector<double> > &Rproblematickych,
                                     QVector<QVector<double> > &FWHMproblematickych,
                                     QVector<QVector<double> > &POCX,
                                     QVector<QVector<double> > &POCY,
                                     QVector<QVector<double> > &U,
                                     QVector<QVector<double> > &F_X,
                                     QVector<QVector<double> > &F_Y,
                                     QVector<QVector<double> > &F_E,
                                     QVector<double> &prumeryR,
                                     QVector<double> &prumeryFWHM,
                                     QObject* parent):QThread(parent)
{
    seznamVidei = sV;
    snimky_k_provereni_prvni = sPrvniRozhodovaniK;
    ohodnoceniSnimkuKomplet = ohodKomplet;
    vypoctene_hodnoty_R = Rproblematickych;
    vypoctene_hodnoty_FWHM = FWHMproblematickych;
    POC_x = POCX;
    POC_y = POCY;
    uhel = U;
    frangi_x = F_X;
    frangi_y = F_Y;
    frangi_euklid = F_E;
    prumerny_korelacni_koeficient = prumeryR;
    prumerne_FWHM = prumeryFWHM;
}

void qThreadFourthPart::run()
{
    emit typeOfMethod(3);
    emit percentageCompleted(0);
    pocetVidei = double(seznamVidei.count());
    for (int indexVidea = 0; indexVidea < seznamVidei.count(); indexVidea++)
    {
        QVector<int> snimky_k_provereni_druhy;
        pocetSnimku = double(snimky_k_provereni_prvni[indexVidea].length());
        QString fullPath = seznamVidei.at(indexVidea);
        QString slozka,jmeno,koncovka;
        zpracujJmeno(fullPath,slozka,jmeno,koncovka);
        emit actualVideo(indexVidea);
        for (int b = 0; b < snimky_k_provereni_prvni[indexVidea].length(); b++)
        {
            emit percentageCompleted(qRound((double(indexVidea)/pocetVidei)*100.0+((double(b)/pocetSnimku)*100.0)/pocetVidei));
            if ((prumerny_korelacni_koeficient[indexVidea] - vypoctene_hodnoty_R[indexVidea][b]) <= 0.01)
            {
                if (vypoctene_hodnoty_FWHM[indexVidea][b] < (prumerne_FWHM[indexVidea] + 2))
                {
                    qDebug()<< "Snimek "<< snimky_k_provereni_prvni[indexVidea][b]<< " je vhodny ke slicovani.";
                    ohodnoceniSnimkuKomplet[indexVidea][snimky_k_provereni_prvni[indexVidea][b]] = 0;
                }
                else
                {
                    qDebug()<< "Snimek "<< snimky_k_provereni_prvni[indexVidea][b]<< " bude proveren.";
                    snimky_k_provereni_druhy.push_back(snimky_k_provereni_prvni[indexVidea][b]);
                }
                continue;
            }
            else if ((prumerny_korelacni_koeficient[indexVidea] - vypoctene_hodnoty_R[indexVidea][b]) >0.01)
            {
                if (vypoctene_hodnoty_FWHM[indexVidea][b] < (prumerne_FWHM[indexVidea] + 2.5))
                {
                    qDebug()<< "Snimek "<< snimky_k_provereni_prvni[indexVidea][b]<< " je vhodny ke slicovani.";
                    ohodnoceniSnimkuKomplet[indexVidea][snimky_k_provereni_prvni[indexVidea][b]] = 0;
                }
                else if (vypoctene_hodnoty_FWHM[indexVidea][b] > (prumerne_FWHM[indexVidea] + 10))
                {
                    qDebug() << "Snimek "<< snimky_k_provereni_prvni[indexVidea][b]<< " nepripusten k analyze.";
                    ohodnoceniSnimkuKomplet[indexVidea][snimky_k_provereni_prvni[indexVidea][b]] = 5.0;
                    POC_x[indexVidea][snimky_k_provereni_prvni[indexVidea][b]] = 999.0;
                    POC_y[indexVidea][snimky_k_provereni_prvni[indexVidea][b]] = 999.0;
                    uhel[indexVidea][snimky_k_provereni_prvni[indexVidea][b]] = 999.0;
                    frangi_x[indexVidea][snimky_k_provereni_prvni[indexVidea][b]] = 999.0;
                    frangi_y[indexVidea][snimky_k_provereni_prvni[indexVidea][b]] = 999.0;
                    frangi_euklid[indexVidea][snimky_k_provereni_prvni[indexVidea][b]] = 999.0;
                }
                else
                {
                    qDebug()<< "Snimek "<< snimky_k_provereni_prvni[indexVidea][b]<< " bude proveren.";
                    snimky_k_provereni_druhy.push_back(snimky_k_provereni_prvni[indexVidea][b]);
                }
                continue;
            }
            else if ((prumerny_korelacni_koeficient[indexVidea] - vypoctene_hodnoty_R[indexVidea][b]) >= 0.05)
            {
                if (vypoctene_hodnoty_FWHM[indexVidea][b] <= (prumerne_FWHM[indexVidea] + 3))
                {
                    qDebug()<< "Snimek "<< snimky_k_provereni_prvni[indexVidea][b]<< " je vhodny ke slicovani.";
                    ohodnoceniSnimkuKomplet[indexVidea][snimky_k_provereni_prvni[indexVidea][b]] = 0;
                }
                else
                {
                    qDebug()<< "Snimek "<< snimky_k_provereni_prvni[indexVidea][b]<< " bude proveren.";
                    snimky_k_provereni_druhy.push_back(snimky_k_provereni_prvni[indexVidea][b]);
                }
                continue;
            }
        }
        snimkyProvereniDruheKomplet.append(snimky_k_provereni_druhy);
    }
    emit hotovo(4);
}

QVector<QVector<int>> qThreadFourthPart::snimkyUpdateOhodnoceniKomplet()
{
    return ohodnoceniSnimkuKomplet;
}
QVector<QVector<int>> qThreadFourthPart::snimkyRozhodovaniDruheKomplet()
{
    return snimkyProvereniDruheKomplet;
}
QVector<QVector<double>> qThreadFourthPart::snimkyFrangiXestimated()
{
    return frangi_x;
}
QVector<QVector<double>> qThreadFourthPart::snimkyFrangiYestimated()
{
    return frangi_y;
}
QVector<QVector<double>> qThreadFourthPart::snimkyFrangiEuklidestimated()
{
    return frangi_euklid;
}
QVector<QVector<double>> qThreadFourthPart::snimkyPOCXestimated()
{
    return POC_x;
}
QVector<QVector<double>> qThreadFourthPart::snimkyPOCYestimated()
{
    return POC_y;
}
QVector<QVector<double>> qThreadFourthPart::snimkyUhelestimated()
{
    return uhel;
}
