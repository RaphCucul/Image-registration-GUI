#include "util/vicevlaknovezpracovani.h"
#include "util/prace_s_vektory.h"
#include "dialogy/multiplevideoet.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "analyza_obrazu/entropie.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <QDebug>
#include <QVector>
#include <QThread>

using cv::Mat;
VicevlaknoveZpracovani::VicevlaknoveZpracovani(QStringList videaKanalyza, QObject *parent):QThread(parent)
{
    zpracujVidea = videaKanalyza;
    pocetVidei = double(videaKanalyza.count());
}

void VicevlaknoveZpracovani::run()
{
    /// Firstly, entropy and tennengrad are computed for each frame of each video
    /// This task is indicated by emiting 0 in typeOfMethod
    emit typeOfMethod(0);
    emit percentageCompleted(0);
    procento = 0;
    for (int kolikateVideo = 0; kolikateVideo < zpracujVidea.count(); kolikateVideo++)
    {
        QString fullPath = zpracujVidea.at(kolikateVideo);
        qDebug()<<"Processing: "<<fullPath;
        cv::VideoCapture cap = cv::VideoCapture(fullPath.toLocal8Bit().constData());
        int frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
        QVector<double> entropyActual,tennengradActual;
        entropyActual.fill(0.0,frameCount);
        tennengradActual.fill(0.0,frameCount);
        int uspech_analyzy = 0;
        if (cap.isOpened() == 0)
            continue;
        else
        {
            double pocet_snimku_videa = (cap.get(CV_CAP_PROP_FRAME_COUNT));
            qDebug()<< "Analyza videa: "<<kolikateVideo;
            for (double a = 0; a < pocet_snimku_videa; a++)
            {               
                //qDebug()<<(kolikateVideo/pocetVidei)*100;//+((a/pocet_snimku_videa)*100.0)/pocetVidei;
                    emit percentageCompleted(qRound((kolikateVideo/pocetVidei)*100+((a/pocet_snimku_videa)*100.0)/pocetVidei));
                //QCoreApplication::processEvents(); // tato funkce frčí v jiném vlákně - mohu sledovat
                cv::Mat snimek;
                double hodnota_entropie;
                cv::Scalar hodnota_tennengrad;
                cap.set(CV_CAP_PROP_POS_FRAMES,(a));
                if (!cap.read(snimek))
                        continue;
                else
                {
                    vypocet_entropie(snimek,hodnota_entropie,hodnota_tennengrad); /// výpočty proběhnou v pořádku
                    double pom = hodnota_tennengrad[0];
                    //qDebug()<<"Zpracovan snimek "<<a<<" s E: "<<hodnota_entropie<<" a T: "<<pom; // hodnoty v normě
                    entropyActual[int(a)] = (hodnota_entropie);
                    tennengradActual[int(a)] = (pom);
                    snimek.release();
                }
            }
            uspech_analyzy = 1;
            procento = qRound(100.0/kolikateVideo+2);
        }
        entropieKomplet.push_back(entropyActual);
        tennengradKomplet.push_back(tennengradActual);
        qDebug()<<entropieKomplet.size()<<tennengradKomplet.size();
        emit percentageCompleted(100);

        /// Secondly, it is necessary to select proper maximal value for later selection of bad images
        /// after this procedure, windows for detail analysis of entropy and tennengrad vectors are computed
        /// medians of values in the windows are computed
        emit percentageCompleted(0);
        emit typeOfMethod(1);
        double correctEntropyMax = kontrola_maxima(entropieKomplet[kolikateVideo]);
        double correctTennengradMax = kontrola_maxima(entropieKomplet[kolikateVideo]);
        QVector<double> windows_tennengrad,windows_entropy,windowsEntropy_medians,windowsTennengrad_medians;
        double restEntropy = 0.0,restTennengrad = 0.0;
        okna_vektoru(entropieKomplet[kolikateVideo],windows_entropy,restEntropy);
        okna_vektoru(tennengradKomplet[kolikateVideo],windows_tennengrad,restTennengrad);
        windowsEntropy_medians = mediany_vektoru(entropieKomplet[kolikateVideo],windows_entropy,restEntropy);
        windowsTennengrad_medians = mediany_vektoru(tennengradKomplet[kolikateVideo],windows_tennengrad,restTennengrad);
        emit percentageCompleted(100);

        /// Thirdly, values of entropy and tennengrad are evaluated and frames get mark good/bad, if they are
        /// or they are not suitable for image registration
        QVector<double> thresholdsEntropy(2,0);
        QVector<double> thresholdsTennengrad(2,0);
        thresholdsEntropy[0] = 0.01;thresholdsEntropy[1] = 0.01;
        thresholdsTennengrad[0] = 10;thresholdsTennengrad[1] = 10;
        double toleranceEntropy = 0.001;
        double toleranceTennengrad = 0.1;
        int dmin = 1;
        QVector<double> badFramesEntropy, badFramesTennengrad;
        QVector<double> nextAnalysisEntropy, nextAnalysisTennengrad;
        analyza_prubehu_funkce(entropieKomplet[kolikateVideo],windowsEntropy_medians,windows_entropy,
                               correctEntropyMax,thresholdsEntropy,toleranceEntropy,dmin,restEntropy,
                               badFramesEntropy,nextAnalysisEntropy);
        analyza_prubehu_funkce(tennengradKomplet[kolikateVideo],windowsTennengrad_medians,windows_tennengrad,
                               correctTennengradMax,thresholdsTennengrad,toleranceTennengrad,dmin,restTennengrad,
                               badFramesTennengrad,nextAnalysisTennengrad);
        int referencni_snimek = nalezeni_referencniho_snimku(correctEntropyMax,nextAnalysisEntropy,
                                                             entropieKomplet[kolikateVideo]);
        /// Fourth part - frangi filter is applied on the frame marked as the reference
        Mat obraz_temp, obraz_vyrez;
        cap.set(CV_CAP_PROP_POS_FRAMES,referencni_snimek);
        cv::Point3d pt_temp(0,0,0);
        cv::Point3d frangi_bod(0,0,0);
        if (hranice_x != 0)
        {
            frangi_bod = frangi_analyza(obraz_temp(vyrez_anomalie),1,1,0,"Frangi puvodniho",1,pritomnost_casove_znacky,pt_temp);
        }
        else
        {
            frangi_bod = frangi_analyza(obraz_temp,1,1,0,"Frangi puvodniho",1,pritomnost_casove_znacky,pt_temp);
        }

        if (frangi_bod.z == 0.0)
            emit hotovo();
        else
        {
            bool nutnost_zmenit_velikost = false;
            int rows = obraz_temp.rows;
            int cols = obraz_temp.cols;
            int radek_od = round(frangi_bod.y-0.8*frangi_bod.y);
            int radek_do = round(frangi_bod.y+0.8*(rows - frangi_bod.y));
            int sloupec_od = 0;
            int sloupec_do = 0;
            if (pritomnost_anomalie == true && hranice_x < (cols/2))
            {
                sloupec_od = hranice_x;
                nutnost_zmenit_velikost = true;
            }
            else
            {
                sloupec_od = round(frangi_bod.x-0.8*(frangi_bod.x));
            }
            if (pritomnost_anomalie == true && hranice_x > (cols/2))
            {
                sloupec_do = hranice_x;
                nutnost_zmenit_velikost = true;
            }
            else
            {
                sloupec_do = round(frangi_bod.x+0.9*(cols - frangi_bod.x));
            }
            int vyrez_sirka = sloupec_do-sloupec_od;
            int vyrez_vyska = radek_do - radek_od;
            Rect vyrez_korelace_extra(0,0,0,0);
            Rect vyrez_korelace_standard(0,0,0,0);
            Mat obraz;
            if ((vyrez_vyska > 480 || vyrez_sirka > 640) || nutnost_zmenit_velikost == true)
            {
                //vyrez_korelace2(sloupec_od,radek_od,vyrez_sirka,vyrez_vyska);
                vyrez_korelace_extra.x = sloupec_od;
                vyrez_korelace_extra.y = radek_od;
                vyrez_korelace_extra.width = vyrez_sirka;
                vyrez_korelace_extra.height = vyrez_vyska;
                /*****************************************************************************/
                obraz_temp(vyrez_korelace_extra).copyTo(obraz);
                obraz_temp.release();
                /*****************************************************************************/
                Point3d max_loc_frangi = frangi_analyza(obraz,1,1,0,"",1,false,pt_temp);//!
                rows = obraz.rows;
                cols = obraz.cols;
                radek_od = round(max_loc_frangi.y-0.9*max_loc_frangi.y);
                radek_do = round(max_loc_frangi.y+0.9*(rows - max_loc_frangi.y));
                sloupec_od = round(max_loc_frangi.x-0.9*(max_loc_frangi.x));
                sloupec_do = round(max_loc_frangi.x+0.9*(cols - max_loc_frangi.x));
                vyrez_sirka = sloupec_do-sloupec_od;
                vyrez_vyska = radek_do - radek_od;
                vyrez_korelace_standard.x = sloupec_od;
                vyrez_korelace_standard.y = radek_od;
                vyrez_korelace_standard.width = vyrez_sirka;
                vyrez_korelace_standard.height = vyrez_vyska;
                nutnost_zmenit_velikost = true;
            }
            else
            {
                vyrez_korelace_standard.x = round(frangi_bod.x-0.9*(frangi_bod.x));
                vyrez_korelace_standard.y = round(frangi_bod.y-0.9*frangi_bod.y);;
                radek_do = round(frangi_bod.y+0.9*(rows - frangi_bod.y));
                sloupec_do = round(frangi_bod.x+0.9*(cols - frangi_bod.x));
                vyrez_korelace_standard.width = sloupec_do-vyrez_korelace_standard.x;
                vyrez_korelace_standard.height = radek_do - vyrez_korelace_standard.y;
                /*vyrez_korelace_standard.x = sloupec_od;
                vyrez_korelace_standard.y = radek_od;
                vyrez_korelace_standard.width = vyrez_sirka;
                vyrez_korelace_standard.height = vyrez_vyska;*/
                obraz_temp.copyTo(obraz);
                obraz_temp.release();
            }

            /// Fifth part - average correlation coefficient and FWHM coefficient of the video are computed
            /// for decision algorithms
            double prumerny_korelacni_koeficient = 0.0;
            QVector<double> spatne_snimky_komplet = spojeni_vektoru(badFramesEntropy,badFramesTennengrad);
            kontrola_celistvosti(spatne_snimky_komplet);
            double prumerne_FWHM = analyza_FWHM(cap,referencni_snimek,frameCount,prumerny_korelacni_koeficient,
                                                nutnost_zmenit_velikost,vyrez_korelace_standard,vyrez_korelace_extra,
                                                spatne_snimky_komplet);


        }
    }





    emit hotovo();

}

QVector<QVector<double> > VicevlaknoveZpracovani::vypocitanaEntropie()
{
    return entropieKomplet;
}

QVector<QVector<double> > VicevlaknoveZpracovani::vypocitanyTennengrad()
{
    return tennengradKomplet;
}
