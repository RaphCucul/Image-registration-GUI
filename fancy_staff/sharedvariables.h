#ifndef SHAREDVARIABLES_H
#define SHAREDVARIABLES_H

#include <QObject>
#include <QMap>
#include <QVector>

class SharedVariables
{
public:
    Q_DISABLE_COPY(SharedVariables)

    static SharedVariables *getSharedVariables();
    QString getPath(QString type) const;
    void setPath(QString type, QString path);
    void processFrangiParameters(QString path);
    QVector<double> getFrangiParameters() const;
    double getSpecificFrangiParameter(int parameter);
    void setSpecificFrangiParameter(int parameter,double value);
    void saveFrangiParameters();
private:
    SharedVariables();
    /**
     * @brief inicializace_frangi_opt
     * @param nactenyObjekt
     * @param parametr
     * @param nacteneParametry
     * @param pozice
     */
    void inicializace_frangi_opt(QJsonObject nactenyObjekt, QString parametr, QVector<double>& nacteneParametry,
                                 int &pozice);
    /**
     * @brief velikost_frangi_opt
     * @param velikost
     * @param nacteneParametry
     */
    void velikost_frangi_opt(int velikost, QVector<double>& nacteneParametry);

    /**
     * @brief data_z_frangi_opt
     * @param pozice
     * @param nacteneParametry
     * @return
     */
    double data_z_frangi_opt(int pozice, QVector<double>& nacteneParametry);
    static SharedVariables *g_sharedVariables;
    QMap<QString,QString> chosenActualPathes;
    QVector<double> FrangiParameters;
    QStringList FrangiParametersList = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};

};

#endif // SHAREDVARIABLES_H
