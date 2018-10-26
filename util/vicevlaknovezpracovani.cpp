#include "util/vicevlaknovezpracovani.h"
#include "util/prace_s_vektory.h"
#include "dialogy/multiplevideoet.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "analyza_obrazu/entropie.h"
#include "analyza_obrazu/korelacni_koeficient.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "licovani/fazova_korelace_funkce.h"
#include "licovani/rozhodovaci_algoritmy.h"
#include "licovani/multiPOC_Ai1.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <QDebug>
#include <QVector>
#include <QThread>

using cv::Mat;
VicevlaknoveZpracovani::VicevlaknoveZpracovani(QStringList videaKanalyza, cv::Point2f souradniceAnomalie,
                                               cv::Point2f souradniceCasZnac, QVector<double> PF,
                                               bool An, bool CZ, QObject *parent):QThread(parent)
{
    zpracujVidea = videaKanalyza;
    pocetVidei = double(videaKanalyza.count());
    ziskane_hranice_anomalie = souradniceAnomalie;
    ziskane_hranice_CasZnac = souradniceCasZnac;
    parametryFrangi = PF;
    AnomalieBool = An;
    CasZnacBool = CZ;
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
        QVector<double> pom(frameCount,0);

        QVector<double> frangi_x,frangi_y,frangi_euklid,
        POC_x,POC_y,uhel;
        frangi_x = pom;frangi_y=pom;frangi_euklid=pom;
        POC_x=pom;POC_y=pom;uhel=pom;

        double sirka = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        double vyska = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        cv::Rect vyrez_anomalie(0,0,0,0);
        cv::Point2f hranice_anomalie;
        cv::Point2f hranice_CasZnac;
        if (ziskane_hranice_anomalie.x >0.0f && ziskane_hranice_anomalie.x < float(sirka))
        {
            hranice_anomalie.x = ziskane_hranice_anomalie.x;
            hranice_anomalie.y = ziskane_hranice_anomalie.y;
        }
        else
        {
            hranice_anomalie.x = 0.0f;
            hranice_anomalie.y = 0.0f;
        }
        if (ziskane_hranice_anomalie.y > 0.0f && ziskane_hranice_anomalie.y < float(vyska))
        {
            hranice_CasZnac.y = ziskane_hranice_anomalie.y;
            hranice_CasZnac.x = ziskane_hranice_anomalie.x;
        }
        else
        {
            hranice_CasZnac.x = 0.0f;
            hranice_CasZnac.y = 0.0f;
        }
        if (hranice_anomalie.x != 0.0f)
        {
            if (hranice_anomalie.x < float(sirka/2))
            {
                vyrez_anomalie.x = 0;
                vyrez_anomalie.y = int(hranice_anomalie.y);
                vyrez_anomalie.width = int(sirka)-int(hranice_anomalie.x)-1;
                vyrez_anomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
            }
            if (hranice_anomalie.x > float(sirka/2))
            {
                vyrez_anomalie.x = 0;
                vyrez_anomalie.y = 0;
                vyrez_anomalie.width = int(hranice_anomalie.x);
                vyrez_anomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
            }
        }
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
                /// Původní výpočet pro průběh zpracování všech videí
                //emit percentageCompleted(qRound((kolikateVideo/pocetVidei)*100+((a/pocet_snimku_videa)*100.0)/pocetVidei));
                /// Nový vyýpočet průběhu čistě pro jedno video
                emit percentageCompleted(qRound(((a/pocet_snimku_videa)*100.0)));
                cv::Mat snimek;
                double hodnota_entropie;
                cv::Scalar hodnota_tennengrad;
                cap.set(CV_CAP_PROP_POS_FRAMES,(a));
                if (!cap.read(snimek))
                    continue;
                else
                {
                    kontrola_typu_snimku_8C3(snimek);
                    vypocet_entropie(snimek,hodnota_entropie,hodnota_tennengrad); /// výpočty proběhnou v pořádku
                    double pom = hodnota_tennengrad[0];
                    //qDebug()<<"Zpracovan snimek "<<a<<" s E: "<<hodnota_entropie<<" a T: "<<pom; // hodnoty v normě
                    entropyActual[int(a)] = (hodnota_entropie);
                    tennengradActual[int(a)] = (pom);
                    snimek.release();
                }
            }
            uspech_analyzy = 1;
            //procento = qRound(100.0/kolikateVideo+2);
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
        emit typeOfMethod(2);
        emit percentageCompleted(0);
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
        emit percentageCompleted(50);
        analyza_prubehu_funkce(tennengradKomplet[kolikateVideo],windowsTennengrad_medians,windows_tennengrad,
                               correctTennengradMax,thresholdsTennengrad,toleranceTennengrad,dmin,restTennengrad,
                               badFramesTennengrad,nextAnalysisTennengrad);
        int referencni_snimek = nalezeni_referencniho_snimku(correctEntropyMax,nextAnalysisEntropy,
                                                             entropieKomplet[kolikateVideo]);
        emit percentageCompleted(100);

        /// Fourth part - frangi filter is applied on the frame marked as the reference
        emit typeOfMethod(3);
        emit percentageCompleted(0);
        Mat reference, reference_vyrez;
        cap.set(CV_CAP_PROP_POS_FRAMES,referencni_snimek);
        cap.read(reference);

        cv::Rect vyrez_korelace_extra(0,0,0,0);
        cv::Rect vyrez_korelace_standard(0,0,0,0);
        cv::Point3d pt_temp(0.0,0.0,0.0);
        cv::Mat obraz;
        cv::Point3d frangi_bod(0,0,0);
        bool zmenaMeritka;
        predzpracovaniKompletnihoLicovani(reference,
                                          obraz,
                                          parametryFrangi,
                                          hranice_anomalie,
                                          hranice_CasZnac,
                                          frangi_bod,
                                          vyrez_anomalie,
                                          vyrez_korelace_extra,
                                          vyrez_korelace_standard,
                                          cap,
                                          AnomalieBool,
                                          CasZnacBool,
                                          zmenaMeritka);

        /// Fifth part - average correlation coefficient and FWHM coefficient of the video are computed
        /// for decision algorithms
        double prumerny_korelacni_koeficient = 0.0;
        QVector<double> spatne_snimky_komplet = spojeni_vektoru(badFramesEntropy,badFramesTennengrad);
        kontrola_celistvosti(spatne_snimky_komplet);
        double prumerne_FWHM = analyza_FWHM(cap,referencni_snimek,frameCount,prumerny_korelacni_koeficient,
                                            zmenaMeritka,vyrez_korelace_standard,vyrez_korelace_extra,
                                            spatne_snimky_komplet);
        QVector<double> hodnoceniSnimku;
        hodnoceniSnimku = pom;
        for (int a = 0; a < spatne_snimky_komplet.size(); a++)
        {
            hodnoceniSnimku[int(spatne_snimky_komplet[a])] = 1;
        }
        if (restEntropy == 1.0)
        {
            spatne_snimky_komplet.push_back(frameCount-1);
        }
        int do_zbytku = int(restEntropy-1.0);
        if (restEntropy > 1)
        {
            while (do_zbytku >= 0)
            {
                spatne_snimky_komplet.push_back(frameCount-1-do_zbytku);
                do_zbytku -= 1;
            }
            spatne_snimky_komplet.push_back(frameCount-1);
        }
        emit percentageCompleted(100);
        /// Sixth part - first decision algorithm
        emit percentageCompleted(0);
        emit typeOfMethod(4);
        QVector<double> vypoctena_R,vypoctena_FWHM,snimkyKprovereniPrvni;
        rozhodovani_prvni(spatne_snimky_komplet,
                          hodnoceniSnimku,
                          POC_x,
                          POC_y,
                          uhel,
                          frangi_x,
                          frangi_y,
                          frangi_euklid,
                          prumerny_korelacni_koeficient,
                          prumerne_FWHM,
                          cap,
                          reference,
                          vyrez_korelace_standard,
                          vyrez_korelace_extra,
                          zmenaMeritka,
                          snimkyKprovereniPrvni,
                          vypoctena_R,
                          vypoctena_FWHM);
        emit percentageCompleted(100);
        if (snimkyKprovereniPrvni.empty() != true)
        {
            emit typeOfMethod(5);
            emit percentageCompleted(0);
            QVector<double> snimkyKprovereniDruhy;
            /// Seventh part - second decision algorithm
            rozhodovani_druhe(spatne_snimky_komplet,
                              hodnoceniSnimku,
                              vypoctena_R,
                              vypoctena_FWHM,
                              POC_x,
                              POC_y,
                              uhel,
                              frangi_x,
                              frangi_y,
                              frangi_euklid,
                              prumerny_korelacni_koeficient,
                              prumerne_FWHM,
                              snimkyKprovereniDruhy);
            emit percentageCompleted(100);
            if (snimkyKprovereniDruhy.empty() != true)
            {
                emit typeOfMethod(6);
                emit percentageCompleted(0);
                /// Eigth part - third decision algorithm
                rozhodovani_treti(obraz,
                                  vyrez_korelace_extra,
                                  vyrez_korelace_standard,
                                  frangi_x,
                                  frangi_y,
                                  frangi_euklid,
                                  POC_x,
                                  POC_y,
                                  uhel,
                                  zmenaMeritka,
                                  CasZnacBool,
                                  cap,
                                  spatne_snimky_komplet,
                                  snimkyKprovereniDruhy,
                                  parametryFrangi);
                emit percentageCompleted(100);
            }
            else
                continue;
        }
        else
            continue;
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

double VicevlaknoveZpracovani::analyzaFWHM(cv::VideoCapture& capture,
                                           int referencni_snimek_cislo,
                                           int pocet_snimku_videa,
                                           double& R,
                                           bool zmena_meritka,
                                           cv::Rect& vyrez_oblasti_standardni,
                                           cv::Rect& vyrez_oblasti_navic,
                                           QVector<double>& spatne_snimky_komplet)
{
    //std::ostream_iterator<double> out_it (std::cout," ");
    int velikost_spojeneho_vektoru = spatne_snimky_komplet.size();
    QVector<double> snimky_pro_sigma((pocet_snimku_videa-velikost_spojeneho_vektoru-10),0);
    QVector<double> cisla_pro_generator(pocet_snimku_videa,0);
    std::iota(cisla_pro_generator.begin(),cisla_pro_generator.end(),0);

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, pocet_snimku_videa-1);
    QVector<double>::iterator it;
    for (int i = 0; i < snimky_pro_sigma.size(); i++)
    {
        int kontrola_ulozeni = 0;
        while (kontrola_ulozeni == 0)
        {
            int vygenerovane_cislo = distr(eng);
            while (cisla_pro_generator[vygenerovane_cislo] == 0.0)
            {
                vygenerovane_cislo = distr(eng);
            }
            it = std::find(spatne_snimky_komplet.begin(), spatne_snimky_komplet.end(), cisla_pro_generator[vygenerovane_cislo]);
            if (it != spatne_snimky_komplet.end())
            {kontrola_ulozeni = 0;}
            else
            {snimky_pro_sigma[i] = vygenerovane_cislo;
                cisla_pro_generator[vygenerovane_cislo] = 0;
                kontrola_ulozeni = 1;}
        }
    }
    //std::copy ( snimky_pro_sigma.begin(), snimky_pro_sigma.end(), out_it );
    //cout<<endl;
    if (capture.isOpened() == 0)
    {
        qWarning()<<"Video nelze pouzit pro analyzu entropie a tennengrada!";
        return 0.0;
    }
    cv::Mat referencni_snimek_temp,referencni_snimek,referencni_snimek32f,referencni_vyrez;
    capture.set(CV_CAP_PROP_POS_FRAMES,referencni_snimek_cislo);
    capture.read(referencni_snimek_temp);
    int rows = 0;
    int cols = 0;
    if (zmena_meritka == true)
    {
        referencni_snimek_temp(vyrez_oblasti_navic).copyTo(referencni_snimek);
        rows = referencni_snimek.rows;
        cols = referencni_snimek.cols;
        referencni_snimek(vyrez_oblasti_standardni).copyTo(referencni_vyrez);
        referencni_snimek_temp.release();
    }
    else
    {
        referencni_snimek_temp.copyTo(referencni_snimek);
        rows = referencni_snimek.rows;
        cols = referencni_snimek.cols;
        referencni_snimek(vyrez_oblasti_standardni).copyTo(referencni_vyrez);
        referencni_snimek_temp.release();
    }
    QVector<double> zaznamenane_FWHM(snimky_pro_sigma.size(),0.0);
    QVector<double> zaznamenane_R(snimky_pro_sigma.size(),0.0);
    //referencni_snimek.copyTo(referencni_snimek32f);
    //kontrola_typu_snimku_32(referencni_snimek32f);
    //cout << snimky_pro_sigma.size()<<" "<<zaznamenane_FWHM.size()<<endl;
    qDebug()<< "Analyza snimku pro urceni prumerneho korelacniho koeficientu a hodnoty FWHM";
    for (int j = 0; j < snimky_pro_sigma.size(); j++)
    {
        Mat posunuty_temp,posunuty,posunuty_vyrez;
        capture.set(CV_CAP_PROP_POS_FRAMES,snimky_pro_sigma[j]);
        if (capture.read(posunuty_temp) != 1)
        {
            qWarning()<<"Snimek "<<j<<" nelze slicovat!";
            continue;
        }
        else
        {
            if (zmena_meritka == true)
            {
                posunuty_temp(vyrez_oblasti_navic).copyTo(posunuty);
                posunuty(vyrez_oblasti_standardni).copyTo(posunuty_vyrez);
                posunuty_temp.release();
            }
            else
            {
                posunuty_temp.copyTo(posunuty);
                posunuty(vyrez_oblasti_standardni).copyTo(posunuty_vyrez);
                posunuty_temp.release();
            }
            emit percentageCompleted(qRound((j*100.0)/snimky_pro_sigma.size()));
            //cout << "\r" << procento << "%";
            cv::Point3d pt(0,0,0);
            if (zmena_meritka == true)
            {
                pt = fk_translace_hann(referencni_snimek,posunuty,5);
                if (std::abs(pt.x)>=290 || std::abs(pt.y)>=290)
                {
                    pt = fk_translace(referencni_snimek,posunuty,5);
                }
            }
            if (zmena_meritka == false){pt = fk_translace_hann(referencni_snimek,posunuty,5);}
            Mat slicovany,slicovany_vyrez;
            slicovany = translace_snimku(posunuty,pt,rows,cols);
            posunuty.release();
            slicovany(vyrez_oblasti_standardni).copyTo(slicovany_vyrez);
            double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt.z);
            double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
            zaznamenane_FWHM[j] = FWHM;
            double R = vypocet_KK(referencni_snimek,slicovany,vyrez_oblasti_standardni);
            slicovany.release();
            slicovany_vyrez.release();
            zaznamenane_R[j] = R;
        }

    }
    //cout << endl;
    //std::copy ( zaznamenane_R.begin(), zaznamenane_R.end(), out_it );
    //cout<<endl;
    double charakteristicke_FWHM = median_vektoru_cisel(zaznamenane_FWHM);
    R = median_vektoru_cisel(zaznamenane_R);
    qDebug()<<"Zaznamenany R: "<<R;
    return charakteristicke_FWHM;
}
