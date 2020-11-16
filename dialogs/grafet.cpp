#include "dialogs/grafet.h"
#include "image_analysis/entropy.h"
#include "util/files_folders_operations.h"
#include "util/vector_operations.h"
#include "shared_staff/sharedvariables.h"
#include "ui_grafet.h"
#include <algorithm>
#include <QVector>
#include <QWidget>
#include <QHBoxLayout>

GrafET::GrafET(QVector<double> i_entropy,
               QVector<double> i_tennengrad,
               QVector<double> i_thresholds,
               QVector<int> i_FirstEvalEntropy,
               QVector<int> i_FirstEvalTennengrad,
               QVector<int> i_FirstDecisionResults,
               QVector<int> i_SecondDecisionResults,
               QVector<int> i_CompleteEvaluation,
               QString i_videoName, QString i_suffix,
               QWidget *parent) : QDialog(parent),
    ui(new Ui::GrafET)
{
    ui->setupUi(this);

    QFile qssFile(":/images/style.qss");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);

    // preparing internal cap variable - load video path, request video size (width,height)
    videoName = i_videoName;
    QString videoPathFull = SharedVariables::getSharedVariables()->getPath("videosPath")+"/"+i_videoName+"."+i_suffix;
    qDebug()<<"Video path "<<videoPathFull;
    cap = cv::VideoCapture(videoPathFull.toLocal8Bit().constData());
    frameRows = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
    frameCols = int(cap.get(CV_CAP_PROP_FRAME_WIDTH));
    frameCount = int(cap.get(CV_CAP_PROP_FRAME_COUNT));

    // fill prepared layout with GraphicsView - this may contain a preview of the video frames
    framePreview = new QGraphicsView;
    framePreview->setFixedSize(frameCols/2+5,frameRows/2+5);
    scene = new QGraphicsScene;
    framePreview->setScene(scene);
    ui->horizontalLayout_3->addWidget(framePreview);
    //resize(sizeHint());

    // saving all provided informations into internal private variables
    initDataMaps(i_entropy,i_tennengrad,i_FirstEvalEntropy,i_FirstEvalTennengrad,i_FirstDecisionResults,
                 i_SecondDecisionResults,i_CompleteEvaluation);

    // standard GUI elements initialization
    ui->E_HPzobraz->setEnabled(false);
    ui->E_DPzobraz->setEnabled(false);
    ui->T_HPzobraz->setEnabled(false);
    ui->T_DPzobraz->setEnabled(false);

    if (ETparametersIntMap["entropyFirstEval"].length() == 0)
        ui->ohodnoceniEntropyCB->setEnabled(false);
    if (ETparametersIntMap["tennengradFirstEval"].length()==0)
        ui->ohodnoceniTennenCB->setEnabled(false);
    if (ETparametersIntMap["decisAlgorFirst"].length() == 0)
        ui->prvniRozhodCB->setEnabled(false);
    if (ETparametersIntMap["decisAlgorSecond"].length() == 0)
        ui->druheRozhodCB->setEnabled(false);

    // find extremes for entropy and tennengrad and apply correct thresholds
    findExtremesAndThresholds(i_thresholds);

    // because entropy and tennengrad can be displayed together in the one grahp and because different values
    // the standardization of both parameters must be done
    valueStandardization(ValueType::ENTROPY_STANDARD,extremes[ValueType::ENTROPY+ExtremeType::MAX],
                        extremes[ValueType::ENTROPY+ExtremeType::MIN]);
    valueStandardization(ValueType::TENNENGRAD_STANDARD,extremes[ValueType::TENNENGRAD+ExtremeType::MAX],
                        extremes[ValueType::TENNENGRAD+ExtremeType::MIN]);

    // creating an object of QCustomPlot class
    QWidget *WSCustomPlot = new QWidget();
    ActualGraphicalObject = new QCustomPlot(WSCustomPlot);
    ui->graphLayout->addWidget(ActualGraphicalObject);
    //change of visible value type
    connect(ui->zobrazGrafE,SIGNAL(stateChanged(int)),this,SLOT(onReactOnDisplayedParameterChange()));
    connect(ui->zobrazGrafT,SIGNAL(stateChanged(int)),this,SLOT(onReactOnDisplayedParameterChange()));
    // change of threshold (value or visibility) - checkboxes
    QObject::connect(ui->E_HPzobraz,&QCheckBox::stateChanged,[=](){
        onReactOnThresholdParameterChange(ValueType::ENTROPY,ThresholdType::UPPER,true);});
    QObject::connect(ui->E_DPzobraz,&QCheckBox::stateChanged,[=](){
        onReactOnThresholdParameterChange(ValueType::ENTROPY,ThresholdType::LOWER,true);});
    QObject::connect(ui->T_HPzobraz,&QCheckBox::stateChanged,[=](){
        onReactOnThresholdParameterChange(ValueType::TENNENGRAD,ThresholdType::UPPER,true);});
    QObject::connect(ui->T_DPzobraz,&QCheckBox::stateChanged,[=](){
        onReactOnThresholdParameterChange(ValueType::TENNENGRAD,ThresholdType::LOWER,true);});
    // change of threshold (value or visibility) - doublespinboxes
    QObject::connect(ui->E_HP,(void(QDoubleSpinBox::*)(double))&QDoubleSpinBox::valueChanged,[=](){
        onReactOnThresholdParameterChange(ValueType::ENTROPY,ThresholdType::UPPER,false);});
    connect(ui->E_DP,(void(QDoubleSpinBox::*)(double))&QDoubleSpinBox::valueChanged,[=](){
        onReactOnThresholdParameterChange(ValueType::ENTROPY,ThresholdType::LOWER,false);});
    connect(ui->T_HP,(void(QDoubleSpinBox::*)(double))&QDoubleSpinBox::valueChanged,[=](){
        onReactOnThresholdParameterChange(ValueType::TENNENGRAD,ThresholdType::UPPER,false);});
    connect(ui->T_DP,(void(QDoubleSpinBox::*)(double))&QDoubleSpinBox::valueChanged,[=](){
        onReactOnThresholdParameterChange(ValueType::TENNENGRAD,ThresholdType::LOWER,false);});
    // show/hide evaluation data
    connect(ui->ohodnoceniEntropyCB,&QCheckBox::stateChanged,[=](){
        showCorrespondingFrames(ValueType::ENTROPY,EvaluationFrames::ENTROPY_FIRST,
                                ui->ohodnoceniEntropyCB->isChecked(),true,0);
    });
    connect(ui->ohodnoceniTennenCB,&QCheckBox::stateChanged,[=](){
        showCorrespondingFrames(ValueType::TENNENGRAD,EvaluationFrames::TENNENGRAD_FIRST,
                                ui->ohodnoceniTennenCB->isChecked(),true,0);
    });
    connect(ui->prvniRozhodCB,&QCheckBox::stateChanged,[=](){
        showCorrespondingFrames(-1,EvaluationFrames::DECISION_FIRST,
                                ui->prvniRozhodCB->isChecked(),false,0);
    });
    connect(ui->druheRozhodCB,&QCheckBox::stateChanged,[=](){
        showCorrespondingFrames(-1,EvaluationFrames::DECISION_SECOND,
                                ui->druheRozhodCB->isChecked(),false,0);
    });
    connect(ui->ohodnocKomplet,&QCheckBox::stateChanged,[=](){
        showCorrespondingFrames(-1,EvaluationFrames::EVALUATION_COMPLETE,
                                ui->ohodnocKomplet->isChecked(),false,1);
    });
    connect(ui->IV1,&QCheckBox::stateChanged,[=](){
        showCorrespondingFrames(-1,EvaluationFrames::EVALUATION_1,
                                ui->IV1->isChecked(),false,2);
    });
    connect(ui->IV4,&QCheckBox::stateChanged,[=](){
        showCorrespondingFrames(-1,EvaluationFrames::EVALUATION_4,
                                ui->IV4->isChecked(),false,3);
    });
    connect(ui->IV5,&QCheckBox::stateChanged,[=](){
        showCorrespondingFrames(-1,EvaluationFrames::EVALUATION_5,
                                ui->IV5->isChecked(),false,4);
    });

    // prepating vectors of thresholds for entropy and tennengrad
    ETparametersDoubleMap.insert("entropyUT",thresholdLine(thresholds[ValueType::ENTROPY+ThresholdType::UPPER],frameCount));
    ETparametersDoubleMap.insert("entropyLT",thresholdLine(thresholds[ValueType::ENTROPY+ThresholdType::LOWER],frameCount));
    ETparametersDoubleMap.insert("tennengradUT",thresholdLine(thresholds[ValueType::TENNENGRAD+ThresholdType::UPPER],frameCount));
    ETparametersDoubleMap.insert("tennengradLT",thresholdLine(thresholds[ValueType::TENNENGRAD+ThresholdType::LOWER],frameCount));

    // creating vector from 0 to frameCount-1
    QVector<double> frames(frameCount);
    std::generate(frames.begin(),frames.end(),[n = 0] () mutable { return n++; });
    valueRange = frames;
    //************************************************************//
    // initialization of the QCustomPlot object - most important part
    //************************************************************//
    utilGraph = new util_GraphET(ActualGraphicalObject,ETparametersDoubleMap,valueRange);
    initAffectedFramesConnections();
    // set values into QDoubleSpinBoxes
    ui->zobrazGrafE->setChecked(true);
    ui->E_HP->setValue(thresholds[ValueType::ENTROPY+ThresholdType::UPPER]);
    ui->E_DP->setValue(thresholds[ValueType::ENTROPY+ThresholdType::LOWER]);
    ui->T_HP->setValue(thresholds[ValueType::TENNENGRAD+ThresholdType::UPPER]);
    ui->T_DP->setValue(thresholds[ValueType::TENNENGRAD+ThresholdType::LOWER]);
    // setup cursor object for better visual mouse movement effect
    cursor = new SW_VerticalQCPItemLine(ActualGraphicalObject,ColorTheme::Dark);
    connect(ActualGraphicalObject, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(showVals(QMouseEvent*)));
    connect(ActualGraphicalObject, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(showContextMenu(QMouseEvent*)));
}

GrafET::~GrafET()
{
    delete ui;
}

void GrafET::closeEvent(QCloseEvent *event){
    if (dataChanged || extremesChanged){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Save data"), tr("Data was changed. Do you want to save it?\n"
                                                                "Save - save data\n"
                                                                "Cancel - close the dialog\n"
                                                                "Close - close the dialog and the whole window without saving"),
                                        QMessageBox::Save|QMessageBox::Cancel|QMessageBox::Close,QMessageBox::Save);
        if (reply == QMessageBox::Save) {
            dataChanged ? saveData(SaveOption::ALL) : saveData(SaveOption::EXTREMES);
        }
        event->accept();
    }
    else
        event->accept();
}

void GrafET::initDataMaps(QVector<double> i_entropy, QVector<double> i_tennengrad,
                          QVector<int> i_firstEvalEntropy, QVector<int> i_firstEvalTennengrad,
                          QVector<int> i_firstDecisionResults, QVector<int> i_secondDecisionResults,
                          QVector<int> i_completeEvaluation){
    // init data maps
    ETparametersDoubleMap.insert("entropy",i_entropy);
    ETparametersDoubleMap.insert("tennengrad",i_tennengrad);
    ETparametersIntMap.insert("entropyFirstEval",i_firstEvalEntropy);
    ETparametersIntMap.insert("tennengradFirstEval",i_firstEvalTennengrad);
    ETparametersIntMap.insert("decisAlgorFirst",i_firstDecisionResults);
    ETparametersIntMap.insert("decisAlgorSecond",i_secondDecisionResults);
    ETparametersIntMap.insert("evalComplete",i_completeEvaluation);

    // init inner connections mapping map
    // connects enum classes with corresponding keys from parameters double and int map
    ETconnections.insert(ValueType::ENTROPY,"entropy");
    ETconnections.insert(ValueType::TENNENGRAD,"tennengrad");
    ETconnections.insert(ValueType::ENTROPY_STANDARD,"entropyStand");
    ETconnections.insert(ValueType::TENNENGRAD_STANDARD,"tennengradStand");
    ETconnections.insert(ValueType::ENTROPY+ThresholdType::UPPER,"entropyUT");
    ETconnections.insert(ValueType::ENTROPY+ThresholdType::LOWER,"entropyLT");
    ETconnections.insert(ValueType::TENNENGRAD+ThresholdType::UPPER,"tennengradUT");
    ETconnections.insert(ValueType::TENNENGRAD+ThresholdType::LOWER,"tennengradLT");

    EFconnections.insert(EvaluationFrames::ENTROPY_FIRST,"entropyFirstEval");
    EFconnections.insert(EvaluationFrames::TENNENGRAD_FIRST,"tennengradFirstEval");
    EFconnections.insert(EvaluationFrames::DECISION_FIRST,"decisAlgorFirst");
    EFconnections.insert(EvaluationFrames::DECISION_SECOND,"decisAlgorSecond");
    EFconnections.insert(EvaluationFrames::EVALUATION_COMPLETE,"evalComplete");
    EFconnections.insert(EvaluationFrames::EVALUATION_1,"eval1");
    EFconnections.insert(EvaluationFrames::EVALUATION_4,"eval4");
    EFconnections.insert(EvaluationFrames::EVALUATION_5,"eval5");

    DSBreferences.insert(ValueType::ENTROPY+ThresholdType::UPPER,ui->E_HP);
    DSBreferences.insert(ValueType::ENTROPY+ThresholdType::LOWER,ui->E_DP);
    DSBreferences.insert(ValueType::TENNENGRAD+ThresholdType::UPPER,ui->T_HP);
    DSBreferences.insert(ValueType::TENNENGRAD+ThresholdType::LOWER,ui->T_DP);
    CBreferences.insert(ValueType::ENTROPY+ThresholdType::UPPER,ui->E_HPzobraz);
    CBreferences.insert(ValueType::ENTROPY+ThresholdType::LOWER,ui->E_DPzobraz);
    CBreferences.insert(ValueType::TENNENGRAD+ThresholdType::UPPER,ui->T_HPzobraz);
    CBreferences.insert(ValueType::TENNENGRAD+ThresholdType::LOWER,ui->T_DPzobraz);
    interWidgetsReferences.insert(ui->E_HP,ui->E_HPzobraz);
    interWidgetsReferences.insert(ui->E_DP,ui->E_DPzobraz);
    interWidgetsReferences.insert(ui->T_HP,ui->T_HPzobraz);
    interWidgetsReferences.insert(ui->T_DP,ui->T_DPzobraz);
}

void GrafET::initAffectedFramesConnections() {
    affectedFramesConnections.insert(ValueType::ENTROPY+ThresholdType::UPPER,utilGraph->getNumberReference("framesUTE"));
    affectedFramesConnections.insert(ValueType::ENTROPY+ThresholdType::LOWER,utilGraph->getNumberReference("framesLTE"));
    affectedFramesConnections.insert(ValueType::TENNENGRAD+ThresholdType::UPPER,utilGraph->getNumberReference("framesUTT"));
    affectedFramesConnections.insert(ValueType::TENNENGRAD+ThresholdType::LOWER,utilGraph->getNumberReference("framesLTT"));
}

void GrafET::showVals(QMouseEvent *event)
{
    if (ActualGraphicalObject->axisRectCount() > 0)
    {
        QCPAxis* xAxis = ActualGraphicalObject->axisRect(0)->axis(QCPAxis::atBottom);
        QCPAxis* yAxis = ActualGraphicalObject->axisRect(0)->axis(QCPAxis::atLeft);
        if (xAxis != nullptr && yAxis != nullptr)
        {
            cursor->SetVisibility(true);
            //vertical
            double Start = yAxis->pixelToCoord(0);
            double End = yAxis->pixelToCoord(ActualGraphicalObject->size().height() - 15); //100 is volume rectangle
            double key;
            if (originFound)
                key = xAxis->pixelToCoord(event->x());
            else{
                key = cursor->start->coords().rx();
                originFound = true;
            }
            // define cursor line
            cursor->start->setCoords(key, Start);
            cursor->end->setCoords(key, End);
            //for my "cursor
            double mouseX = ActualGraphicalObject->xAxis->pixelToCoord(event->pos().x());
            //for the  graph values
            double frameIndex = getAxesValues(mouseX);
            if (_displayedGraphs == ValueType::ENTROPY){
                ui->entropyLabel->setText(QString::number(ETparametersDoubleMap[ETconnections[ValueType::ENTROPY]][int(frameIndex)]));
            }
            else if (_displayedGraphs == ValueType::TENNENGRAD){
                ui->tennengradLabel->setText(QString::number(ETparametersDoubleMap[ETconnections[ValueType::TENNENGRAD]][int(frameIndex)]));
            }
            else{
                ui->entropyLabel->setText(QString::number(ETparametersDoubleMap[ETconnections[ValueType::ENTROPY]][int(frameIndex)]));
                ui->tennengradLabel->setText(QString::number(ETparametersDoubleMap[ETconnections[ValueType::TENNENGRAD]][int(frameIndex)]));
            }
            ui->evalIndex_label->setText(QString::number(ETparametersIntMap["evalComplete"][int(frameIndex)]));
            // load frame preview if allowed
            if (showFrames){
                if (cap.isOpened()){
                    cv::Mat frame;
                    cap.set(CV_CAP_PROP_POS_FRAMES,frameIndex);
                    cap.read(frame);
                    transformMatTypeTo8C3(frame);

                    QImage* imageObject = new QImage(frame.data,
                                                     frame.cols,
                                                     frame.rows,
                                                     static_cast<int>(frame.step),
                                                     QImage::Format_RGB888);
                    QPixmap pixmap = QPixmap::fromImage(*imageObject);
                    pixmap = pixmap.scaled(frame.cols/2,frame.rows/2);
                    QGraphicsPixmapItem* image = new QGraphicsPixmapItem(pixmap);
                    framePreview->scene()->addItem(image);
                    delete imageObject;
                }
            }
            ActualGraphicalObject->replot();
        }
    }
}

double GrafET::getAxesValues(double i_mouseX_coordinates){
    if (_displayedGraphs == ValueType::ENTROPY || _displayedGraphs == ValueType::BOTH){
        int index = ActualGraphicalObject->graph(0)->findBegin(i_mouseX_coordinates);
        return ActualGraphicalObject->graph(0)->dataMainKey(index); //x
    }
    else{
        int index = ActualGraphicalObject->graph(0)->findBegin(i_mouseX_coordinates);
        return ActualGraphicalObject->graph(0)->dataMainKey(index);
    }
}

void GrafET::showContextMenu(QMouseEvent *i_point){
    if (i_point->button() == Qt::RightButton){
        double mouseX = ActualGraphicalObject->xAxis->pixelToCoord(i_point->pos().x());
        int index = ActualGraphicalObject->graph(0)->findBegin(mouseX);
        double frameIndex = ActualGraphicalObject->graph(0)->dataMainKey(index);
        // creating context menu
        QMenu contextMenu(tr("Context menu"), this);
        contextMenu.setAttribute(Qt::WA_DeleteOnClose);
        QAction referentialAction(tr("Make this frame referential"), this);
        QAction changeEvalIndexAction(tr("Change evaluation index"),this);
        // define available actions - if selected frame is included in the affected frames list,
        // the action "change evaluation index" will be active
        // otherwise "set frame as referential" will be active
        QStringList _keys = {"lesser","greater"};
        bool foundAffected = false;
        foreach (QString key,_keys){
            if (affectedFrames[key].contains(double(frameIndex))){
                foundAffected = true;
                break;
            }
        }
        if (foundAffected){
            contextMenu.addAction(&changeEvalIndexAction);
            connect(&changeEvalIndexAction, &QAction::triggered,[=](){
                changeEvalIndex(int(frameIndex));
            });
        }
        else{
            contextMenu.addAction(&referentialAction);
            connect(&referentialAction, &QAction::triggered,[=](){
                makeReferential(int(frameIndex));
            });
        }
        contextMenu.exec(ActualGraphicalObject->mapToGlobal(i_point->pos()));
    }
}

void GrafET::makeReferential(int i_frameIndex){
    int originalReferentialFrame = findReferentialFrame();
    ETparametersIntMap["evalComplete"][originalReferentialFrame] = 0;
    ETparametersIntMap["evalComplete"][i_frameIndex] = 2;
    dataChanged = true;
}

void GrafET::changeEvalIndex(int i_frameIndex){
    bool ok;
    QString headerText = tr("Assing new evaluation index for frame ")+QString::number(i_frameIndex);
    int newEvaluationIndex = QInputDialog::getInt(this, headerText, tr("Evaluation index:"), 0, 0, 5, 1, &ok);
    if (ok){
        ETparametersIntMap["evalComplete"][i_frameIndex] = newEvaluationIndex;
        dataChanged = true;
    }
}

int GrafET::findReferentialFrame(){
    bool found = false;
    int index;
    for (index = 0; index < ETparametersIntMap["evalComplete"].length(); index++){
        if (ETparametersIntMap["evalComplete"][index] == 2) {
            found = true;
            break;
        }
    }
    int output = found ? index : -1;
    return output;
}

void GrafET::on_showFrameOverview_stateChanged(int arg1)
{
    Q_UNUSED(arg1)
    if (ui->showFrameOverview->isChecked())
        showFrames = true;
    else{
        showFrames = false;
        scene->clear();
    }
    emit resizeWindow();
}

void GrafET::findExtremesAndThresholds(QVector<double> i_calculatedThresholds)
{
    extremes.insert(ValueType::ENTROPY+ExtremeType::MAX,findExtreme(ETparametersDoubleMap[ETconnections[ValueType::ENTROPY]],ExtremeType::MAX));
    extremes.insert(ValueType::ENTROPY+ExtremeType::MIN,findExtreme(ETparametersDoubleMap[ETconnections[ValueType::ENTROPY]],ExtremeType::MIN));
    extremes.insert(ValueType::TENNENGRAD+ExtremeType::MAX,findExtreme(ETparametersDoubleMap[ETconnections[ValueType::TENNENGRAD]],ExtremeType::MAX));
    extremes.insert(ValueType::TENNENGRAD+ExtremeType::MIN,findExtreme(ETparametersDoubleMap[ETconnections[ValueType::TENNENGRAD]],ExtremeType::MIN));

    if (i_calculatedThresholds.isEmpty() || i_calculatedThresholds.length() < 3 || vectorSum(i_calculatedThresholds) <= 0) {
        thresholds.insert(ValueType::ENTROPY+ThresholdType::UPPER,createThreshold(extremes[ValueType::ENTROPY+ExtremeType::MAX],ThresholdType::UPPER,ValueType::ENTROPY));
        thresholds.insert(ValueType::ENTROPY+ThresholdType::LOWER,createThreshold(extremes[ValueType::ENTROPY+ExtremeType::MIN],ThresholdType::LOWER,ValueType::ENTROPY));
        thresholds.insert(ValueType::TENNENGRAD+ThresholdType::UPPER,createThreshold(extremes[ValueType::TENNENGRAD+ExtremeType::MAX],ThresholdType::UPPER,ValueType::TENNENGRAD));
        thresholds.insert(ValueType::TENNENGRAD+ThresholdType::LOWER,createThreshold(extremes[ValueType::TENNENGRAD+ExtremeType::MIN],ThresholdType::LOWER,ValueType::TENNENGRAD));
    }
    else {
        thresholds.insert(ValueType::ENTROPY+ThresholdType::UPPER,i_calculatedThresholds[1]);
        thresholds.insert(ValueType::ENTROPY+ThresholdType::LOWER,i_calculatedThresholds[0]);
        thresholds.insert(ValueType::TENNENGRAD+ThresholdType::UPPER,i_calculatedThresholds[3]);
        thresholds.insert(ValueType::TENNENGRAD+ThresholdType::LOWER,i_calculatedThresholds[2]);
    }
    thresholds.insert(ValueType::ENTROPY_STANDARD+ThresholdType::UPPER,valueStandardization(thresholds[ValueType::ENTROPY+ThresholdType::UPPER],
                      extremes[ValueType::ENTROPY+ExtremeType::MAX],extremes[ValueType::ENTROPY+ExtremeType::MIN]));
    thresholds.insert(ValueType::ENTROPY_STANDARD+ThresholdType::LOWER,valueStandardization(thresholds[ValueType::ENTROPY+ThresholdType::LOWER],
                      extremes[ValueType::ENTROPY+ExtremeType::MAX],extremes[ValueType::ENTROPY+ExtremeType::MIN]));
    thresholds.insert(ValueType::TENNENGRAD_STANDARD+ThresholdType::UPPER,valueStandardization(thresholds[ValueType::TENNENGRAD+ThresholdType::UPPER],
                      extremes[ValueType::TENNENGRAD+ExtremeType::MAX],extremes[ValueType::TENNENGRAD+ExtremeType::MIN]));
    thresholds.insert(ValueType::TENNENGRAD_STANDARD+ThresholdType::LOWER,valueStandardization(thresholds[ValueType::TENNENGRAD+ThresholdType::LOWER],
                      extremes[ValueType::TENNENGRAD+ExtremeType::MAX],extremes[ValueType::TENNENGRAD+ExtremeType::MIN]));
}

double GrafET::findExtreme(QVector<double> i_vectorForSearch, ExtremeType extreme)
{
    if (extreme == ExtremeType::MAX)
    {
        QVector<double>::iterator maximumIterator = std::max_element(i_vectorForSearch.begin(),i_vectorForSearch.end());
        int maximalPosition = std::distance(i_vectorForSearch.begin(),maximumIterator);
        return i_vectorForSearch[maximalPosition];
    }
    else
    {
        QVector<double>::iterator minimumIterator = std::min_element(i_vectorForSearch.begin(),i_vectorForSearch.end());
        int minimalPosition = std::distance(i_vectorForSearch.begin(),minimumIterator);
        return i_vectorForSearch[minimalPosition];
    }
}

double GrafET::createThreshold(double i_extrem, ThresholdType threshold, ValueType type)
{
    double thresholdValue = 0.0;
    if (type == ValueType::ENTROPY)
    {
        if (threshold == ThresholdType::UPPER)
            thresholdValue = (i_extrem-0.01);
        if (threshold == ThresholdType::LOWER)
            thresholdValue = (i_extrem+0.01);
    }
    if (type == ValueType::TENNENGRAD)
    {
        if (threshold == ThresholdType::UPPER)
            thresholdValue = (i_extrem-5);
        if (threshold == ThresholdType::LOWER)
            thresholdValue = (i_extrem+5);
    }
    return thresholdValue;
}
void GrafET::valueStandardization(ValueType type, double i_max, double i_min)
{
    QVector<double> pom;
    pom.fill(0.0,frameCount);
    QVector<double> data = ETparametersDoubleMap[ETconnections[type-10]];
    for (int b = 0; b < frameCount; b++)
        pom[b] = (data[b]-i_min)/(i_max-i_min);
    ETparametersDoubleMap.insert(ETconnections[type],pom);
}

double GrafET::valueStandardization(double i_originalValue, double i_max, double i_min){
    return ((i_originalValue-i_min)/(i_max-i_min));
}

QVector<double> GrafET::thresholdLine(double i_originalValueForVector, int i_frameCount)
{
    QVector<double> pom;
    pom.fill(i_originalValueForVector,i_frameCount);
    return pom;
}

void GrafET::on_saveResults_clicked()
{
    saveData(SaveOption::ALL);
}

void GrafET::on_saveThresholds_clicked()
{
    saveData(SaveOption::EXTREMES);
}

void GrafET::saveData(SaveOption i_saveOption){
    QJsonObject _returnObject;
    if (i_saveOption == SaveOption::ALL){        
        prepareDataForSaving(ETparametersIntMap["evalComplete"],_returnObject,"evaluation");
        prepareDataForSaving(thresholdsToVector(),_returnObject,"thresholds");
    }
    else if (i_saveOption == SaveOption::EXTREMES) {
        prepareDataForSaving(thresholdsToVector(),_returnObject,"thresholds");
    }
    saveCalculatedData(videoName,_returnObject);
}

QVector<double> GrafET::thresholdsToVector() {
    QVector<double> returnVector;
    returnVector.push_back(thresholds[ValueType::ENTROPY+ThresholdType::LOWER]);
    returnVector.push_back(thresholds[ValueType::ENTROPY+ThresholdType::UPPER]);
    returnVector.push_back(thresholds[ValueType::TENNENGRAD+ThresholdType::LOWER]);
    returnVector.push_back(thresholds[ValueType::TENNENGRAD+ThresholdType::UPPER]);
    return returnVector;
}

void GrafET::onReactOnDisplayedParameterChange() {
    if(ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked()){
        _displayedGraphs = ValueType::ENTROPY;
        reactOnDisplayedParameterChange(ValueType::ENTROPY,false);
        entropyRelatedWidgetsController(true);
        tennengradRelatedWidgetsController(false);
    }
    else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()){
        _displayedGraphs = ValueType::TENNENGRAD;
        reactOnDisplayedParameterChange(ValueType::TENNENGRAD,false);
        entropyRelatedWidgetsController(false);
        tennengradRelatedWidgetsController(true);
    }
    else if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()){
        _displayedGraphs = ValueType::BOTH;
        ActualGraphicalObject->yAxis->setRange(0.0,1.0);
        reactOnDisplayedParameterChange(ValueType::ENTROPY,true);
        reactOnDisplayedParameterChange(ValueType::TENNENGRAD,true);
        entropyRelatedWidgetsController(true);
        tennengradRelatedWidgetsController(true);
    }
    else
        hideAll();
    ActualGraphicalObject->replot();
}

void GrafET::reactOnDisplayedParameterChange(ValueType i_valueType, bool standardizationRequested) {
    ActualGraphicalObject->xAxis->setRange(1, frameCount);
    if (!standardizationRequested)
        ActualGraphicalObject->yAxis->setRange(extremes[i_valueType+ExtremeType::MIN]-0.1,extremes[i_valueType+ExtremeType::MAX]+0.1);

    int correctData = standardizationRequested ? i_valueType+10 : i_valueType;
    int _identifier = utilGraph->getNumberReference(ETconnections[correctData]);
    ActualGraphicalObject->graph(_identifier)->setVisible(true);

    int upper = i_valueType+ThresholdType::UPPER;
    int lower = i_valueType+ThresholdType::LOWER;
    int upperThr = standardizationRequested ? upper+10 : upper;
    int lowerThr = standardizationRequested ? lower+10 : lower;
    /*ETparametersDoubleMap[ETconnections[upper]].fill(0.0,frameCount);
    ETparametersDoubleMap[ETconnections[upper]].fill(thresholds[upperThr],frameCount);
    _identifier = utilGraph->getNumberReference(ETconnections[i_valueType+ThresholdType::UPPER]);
    ActualGraphicalObject->graph(_identifier)->setData(valueRange,ETparametersDoubleMap[ETconnections[upper]]);*/
    adjustThresholdVectors(upper,upperThr,CBreferences[upper]->isChecked());
    if (CBreferences[upper]->isChecked() && i_valueType==ValueType::ENTROPY) {
        showAffectedFrames(ETparametersDoubleMap[ETconnections[i_valueType]],
                           thresholds[upperThr],
                           "greater",
                           affectedFramesConnections[upper]);
    }
    /*ETparametersDoubleMap[ETconnections[lower]].fill(0.0,frameCount);
    ETparametersDoubleMap[ETconnections[lower]].fill(thresholds[lowerThr],frameCount);
    _identifier = utilGraph->getNumberReference(ETconnections[i_valueType+ThresholdType::LOWER]);
    ActualGraphicalObject->graph(_identifier)->setData(valueRange,ETparametersDoubleMap[ETconnections[lower]]);*/
    adjustThresholdVectors(lower,lowerThr,CBreferences[lower]->isChecked());
    if (CBreferences[lower]->isChecked() && i_valueType==ValueType::ENTROPY) {
        showAffectedFrames(ETparametersDoubleMap[ETconnections[i_valueType]],
                           thresholds[lowerThr],
                           "lesser",
                           affectedFramesConnections[lower]);
    }
}

void GrafET::adjustThresholdVectors(int i_mapIdentifier, int i_thresholdIdentifier, bool i_setVisible) {
    ETparametersDoubleMap[ETconnections[i_mapIdentifier]].fill(0.0,frameCount);
    ETparametersDoubleMap[ETconnections[i_mapIdentifier]].fill(thresholds[i_thresholdIdentifier],frameCount);
    int _graphIdentifier = utilGraph->getNumberReference(ETconnections[i_mapIdentifier]);
    ActualGraphicalObject->graph(_graphIdentifier)->setData(valueRange,ETparametersDoubleMap[ETconnections[i_mapIdentifier]]);
    ActualGraphicalObject->graph(_graphIdentifier)->setVisible(i_setVisible);
}

void GrafET::entropyRelatedWidgetsController(bool status) {
    ui->E_HPzobraz->setEnabled(status);
    ui->E_DPzobraz->setEnabled(status);
    ui->ohodnoceniEntropyCB->setEnabled(status);
    ui->prvniRozhodCB->setEnabled(status);
    ui->druheRozhodCB->setEnabled(status);
    ui->IV1->setEnabled(status);
    ui->IV4->setEnabled(status);
    ui->IV5->setEnabled(status);

    if (!status){
        // if status indicates to disable all corresponding widgets,
        // checkboxes must be unchecked
        ui->E_HPzobraz->setChecked(status);
        ui->E_DPzobraz->setChecked(status);
        ui->ohodnoceniEntropyCB->setChecked(status);
        ui->prvniRozhodCB->setChecked(status);
        ui->druheRozhodCB->setChecked(status);
        ui->IV1->setChecked(status);
        ui->IV4->setChecked(status);
        ui->IV5->setChecked(status);
    }
}

void GrafET::tennengradRelatedWidgetsController(bool status) {
    ui->T_HPzobraz->setEnabled(status);
    ui->T_DPzobraz->setEnabled(status);
    ui->ohodnoceniTennenCB->setEnabled(status);

    if (!status){
        // if status indicates to disable all corresponding widgets,
        // checkboxes must be unchecked
        ui->T_HPzobraz->setChecked(status);
        ui->T_DPzobraz->setChecked(status);
        ui->ohodnoceniTennenCB->setChecked(status);
    }
}

/*void GrafET::showEntropy()
{
    if(ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
    {
        _displayedGraphs = DisplayedGraphs::ENTROPY;
        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(extremes["minEntropy"]-0.1,extremes["maxEntropy"]+0.1);
        ActualGraphicalObject->graph(0)->setVisible(true);
        ActualGraphicalObject->replot();

        UT_entropy.fill(0.0,frameCount);
        UT_entropy.fill(thresholds["upperE"],frameCount);
        ActualGraphicalObject->graph(4)->setData(valueRange,UT_entropy);
        LT_entropy.fill(0.0,frameCount);
        LT_entropy.fill(thresholds["lowerE"],frameCount);
        ActualGraphicalObject->graph(5)->setData(valueRange,LT_entropy);

        ui->E_HPzobraz->setEnabled(true);ui->E_DPzobraz->setEnabled(true);
        ui->T_HPzobraz->setEnabled(false);ui->T_HPzobraz->setChecked(false);
        ui->T_DPzobraz->setEnabled(false);ui->T_DPzobraz->setChecked(false);
        ui->ohodnoceniEntropyCB->setEnabled(true);
        ui->ohodnoceniTennenCB->setEnabled(false);ui->ohodnoceniTennenCB->setChecked(false);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);
    }
    else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
    {
        _displayedGraphs = DisplayedGraphs::TENNENGRAD;
        ActualGraphicalObject->graph(0)->setVisible(false);
        ActualGraphicalObject->graph(2)->setVisible(false);
        ActualGraphicalObject->graph(3)->setVisible(false);
        ui->E_HPzobraz->setChecked(false);
        ui->E_DPzobraz->setChecked(false);

        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(extremes["minTennengrad"]-1,extremes["maxTennengrad"]+1);
        ActualGraphicalObject->graph(1)->setVisible(true);

        //if (ui->T_HPzobraz->isChecked()){
            HP_tennengrad.fill(0.0,frameCount);
            HP_tennengrad.fill(thresholds["upperT"],frameCount);
            ActualGraphicalObject->graph(6)->setData(valueRange,HP_tennengrad);
        //}
        //if (ui->T_DPzobraz->isChecked()){
            DP_tennengrad.fill(0.0,frameCount);
            DP_tennengrad.fill(thresholds["lowerT"],frameCount);
            ActualGraphicalObject->graph(7)->setData(valueRange,DP_tennengrad);
        //}

        ActualGraphicalObject->replot();
        ui->E_HPzobraz->setEnabled(false);
        ui->E_DPzobraz->setEnabled(false);
        ui->T_HPzobraz->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
        ui->ohodnoceniEntropyCB->setEnabled(false);
        ui->ohodnoceniTennenCB->setEnabled(true);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);
    }
    else if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
    {
        _displayedGraphs = DisplayedGraphs::BOTH;
        ActualGraphicalObject->graph(0)->setVisible(false);
        ActualGraphicalObject->graph(1)->setVisible(false);

        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(0,1);
        ActualGraphicalObject->graph(2)->setVisible(true);
        ActualGraphicalObject->graph(3)->setVisible(true);

        //if (ui->T_HPzobraz->isChecked())
        //{
            HP_tennengrad.fill(0.0,frameCount);
            HP_tennengrad.fill(thresholds["upperTS"],frameCount);
            ActualGraphicalObject->graph(6)->setData(valueRange,HP_tennengrad);
        //}
        //if (ui->T_DPzobraz->isChecked())
        //{
            DP_tennengrad.fill(0.0,frameCount);
            DP_tennengrad.fill(thresholds["lowerTS"],frameCount);
            ActualGraphicalObject->graph(7)->setData(valueRange,DP_tennengrad);
        //}
        ActualGraphicalObject->replot();

        ui->E_HPzobraz->setEnabled(true);
        ui->E_DPzobraz->setEnabled(true);
        ui->ohodnoceniEntropyCB->setEnabled(true);
        ui->ohodnoceniTennenCB->setEnabled(true);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);
    }
    else{
        hideAll();
    }
}

void GrafET::showTennengrad()
{
    if(ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
    {
        _displayedGraphs = DisplayedGraphs::ENTROPY;
        ActualGraphicalObject->graph(2)->setVisible(false);
        ActualGraphicalObject->graph(3)->setVisible(false);
        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(extremes["minEntropy"]-0.1,extremes["maxEntropy"]+0.1);
        ActualGraphicalObject->graph(0)->setVisible(true);
        ui->ohodnoceniTennenCB->setEnabled(false);
        //if (ui->E_HPzobraz->isChecked())
        //{
            UT_entropy.fill(0.0,frameCount);
            UT_entropy.fill(thresholds["upperE"],frameCount);
            ActualGraphicalObject->graph(4)->setData(valueRange,UT_entropy);
        //}
        //if (ui->E_DPzobraz->isChecked())
        //{
            LT_entropy.fill(0.0,frameCount);
            LT_entropy.fill(thresholds["lowerE"],frameCount);
            ActualGraphicalObject->graph(5)->setData(valueRange,LT_entropy);
        //}
        ActualGraphicalObject->replot();

        ui->T_HPzobraz->setEnabled(false);ui->T_HPzobraz->setChecked(false);
        ui->T_DPzobraz->setEnabled(false);ui->T_DPzobraz->setChecked(false);
        ui->ohodnoceniEntropyCB->setEnabled(true);
        ui->ohodnoceniTennenCB->setEnabled(false);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);
    }
    else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
    {
        _displayedGraphs = DisplayedGraphs::TENNENGRAD;
        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(extremes["minTennengrad"]-1,extremes["maxTennengrad"]+1);
        ActualGraphicalObject->graph(1)->setVisible(true);

        //if (ui->T_HPzobraz->isChecked())
        //{
            HP_tennengrad.fill(0.0,frameCount);
            HP_tennengrad.fill(thresholds["upperT"],frameCount);
            ActualGraphicalObject->graph(6)->setData(valueRange,HP_tennengrad);
        //}
        //if (ui->T_DPzobraz->isChecked())
        //{
            DP_tennengrad.fill(0.0,frameCount);
            DP_tennengrad.fill(thresholds["lowerT"],frameCount);
            ActualGraphicalObject->graph(7)->setData(valueRange,DP_tennengrad);
        //}

        ActualGraphicalObject->replot();

        ui->T_HPzobraz->setEnabled(true);ui->ohodnoceniTennenCB->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
        ui->ohodnoceniEntropyCB->setEnabled(false);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);

    }
    else if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
    {
        _displayedGraphs = DisplayedGraphs::BOTH;
        ActualGraphicalObject->xAxis->setRange(1, frameCount);
        ActualGraphicalObject->yAxis->setRange(0,1);
        ActualGraphicalObject->graph(2)->setVisible(true);
        ActualGraphicalObject->graph(3)->setVisible(true);
        ActualGraphicalObject->graph(0)->setVisible(false);
        //if (ui->E_HPzobraz->isChecked() == true)
        //{
            UT_entropy.fill(0.0,frameCount);
            UT_entropy.fill(thresholds["upperES"],frameCount);
            ActualGraphicalObject->graph(4)->setData(valueRange,UT_entropy);
            ActualGraphicalObject->graph(4)->setVisible(true);
        //}
        //if (ui->E_DPzobraz->isChecked() == true)
        //{
            LT_entropy.fill(0.0,frameCount);
            LT_entropy.fill(thresholds["lowerES"],frameCount);
            ActualGraphicalObject->graph(5)->setData(valueRange,LT_entropy);
            ActualGraphicalObject->graph(5)->setVisible(true);
        //}

        ActualGraphicalObject->replot();

        ui->T_HPzobraz->setEnabled(true);
        ui->T_DPzobraz->setEnabled(true);
        ui->ohodnoceniEntropyCB->setEnabled(true);
        ui->ohodnoceniTennenCB->setEnabled(true);
        ui->prvniRozhodCB->setEnabled(true);
        ui->druheRozhodCB->setEnabled(true);
        ui->IV1->setEnabled(true);ui->IV4->setEnabled(true);ui->IV5->setEnabled(true);

    }
    else{
        hideAll();
    }
}*/

void GrafET::onReactOnThresholdParameterChange(ValueType i_type,
                                               ThresholdType i_thresholdType,
                                               bool isCheckbox) {
    // check if the threshold visibility is checked <- graph should be visible
    bool isChecked = false;
    if (isCheckbox)
        isChecked = CBreferences[i_type+i_thresholdType]->isChecked();
    else
        isChecked = interWidgetsReferences.value(DSBreferences[i_type+i_thresholdType])->isChecked();

    // get value from corresponding double spin box
    double thresholdValue = DSBreferences[i_type+i_thresholdType]->value();

    // decide if to show or hide the graph
    if (isChecked) {
        ValueType whatToDisplay = i_type;//_displayedGraphs==ValueType::BOTH ? ValueType::ENTROPY : i_type;
        bool standardisation = _displayedGraphs==ValueType::BOTH;
        reactOnThresholdParameterChange(whatToDisplay,
                                        i_thresholdType,
                                        thresholdValue,
                                        whatToDisplay==ValueType::ENTROPY ? true : false,
                                        standardisation);
    }
    else
        hideSpecificGraph(i_type,i_thresholdType,thresholdValue);

    /*if (_displayedGraphs == ValueType::ENTROPY && isCheckbox)
        reactOnThresholdParameterChange(ValueType::ENTROPY,i_thresholdType,thresholdValue,true,false);
    else if (_displayedGraphs == ValueType::BOTH && isCheckbox)
        reactOnThresholdParameterChange(ValueType::ENTROPY,i_thresholdType,thresholdValue,true,true); // 4?
    else if (_displayedGraphs == ValueType::TENNENGRAD && isCheckbox)
        reactOnThresholdParameterChange(ValueType::TENNENGRAD,i_thresholdType,thresholdValue,false,false);
    else if (_displayedGraphs == ValueType::BOTH && isCheckbox)
        reactOnThresholdParameterChange(ValueType::TENNENGRAD,i_thresholdType,thresholdValue,false,true);
    else
        hideSpecificGraph(i_type,i_thresholdType,thresholdValue);*/
}

void GrafET::reactOnThresholdParameterChange(ValueType i_affectedType,
                                             ThresholdType i_actuallyDisplayed,
                                             double i_thresholdValue,
                                             bool i_showAffectedFrames,
                                             bool i_standardizationRequested) {

    // prepare vectors with thresholds - it depends on standardization if requested or not
    int identifierMap = i_affectedType+i_actuallyDisplayed;
    int identifierThr = i_standardizationRequested ? identifierMap+10 : identifierMap;
    if (i_standardizationRequested) {
        thresholds[identifierThr] = valueStandardization(i_thresholdValue,
                                                         extremes[i_affectedType+ExtremeType::MAX],
                                                         extremes[i_affectedType+ExtremeType::MIN]);
    }
    else
        thresholds[identifierThr] = i_thresholdValue;

    adjustThresholdVectors(identifierMap,identifierThr,CBreferences[identifierMap]->isChecked());
    if (i_showAffectedFrames) {
        showAffectedFrames(ETparametersDoubleMap[ETconnections[identifierThr - i_actuallyDisplayed]],
                           thresholds[identifierThr],
                           i_actuallyDisplayed==ThresholdType::UPPER?"greater":"lesser",
                           affectedFramesConnections[identifierMap]);
    }
    ActualGraphicalObject->replot();
    extremesChanged = true;
}

/*void GrafET::showEntropyUpperThreshold()
{
    double actualValueEUT = ui->E_HP->value();
    if (_displayedGraphs == DisplayedGraphs::ENTROPY && ui->E_HPzobraz->isChecked())
    {
            UT_entropy.fill(0.0,frameCount);
            thresholds["upperE"] = actualValueEUT;
            UT_entropy.fill(thresholds["upperE"],frameCount);
            ActualGraphicalObject->graph(4)->setData(valueRange,UT_entropy);
            ActualGraphicalObject->graph(4)->setVisible(true);
            showAffectedFrames(entropy,thresholds["upperE"],"greater",16);
            ActualGraphicalObject->replot();
            extremesChanged = true;
    }
    else if (_displayedGraphs == DisplayedGraphs::BOTH && ui->E_HPzobraz->isChecked())
    {
        if (std::abs(actualValueEUT-thresholds["upperE"])>0.005)
        {
            UT_entropy.fill(0.0,frameCount);
            thresholds["upperES"] = (actualValueEUT-extremes["minEntropy"])/(extremes["maxEntropy"]-extremes["minEntropy"]);
            UT_entropy.fill(thresholds["upperES"],frameCount);
            ActualGraphicalObject->graph(4)->setData(valueRange,UT_entropy);
            ActualGraphicalObject->graph(4)->setVisible(true);
            showAffectedFrames(entropyStandard,thresholds["upperES"],"greater",16);
            ActualGraphicalObject->replot();
            extremesChanged = true;
        }
        else
        {
            UT_entropy.fill(0.0,frameCount);
            UT_entropy.fill(thresholds["upperES"],frameCount);
            ActualGraphicalObject->graph(4)->setData(valueRange,UT_entropy);
            ActualGraphicalObject->graph(4)->setVisible(true);
            ActualGraphicalObject->graph(16)->setVisible(false);
            ActualGraphicalObject->replot();
        }
    }
    else if (!ui->E_HPzobraz->isChecked())
    {
        thresholds["upperE"] = actualValueEUT;
        ActualGraphicalObject->graph(4)->setVisible(false);
        ActualGraphicalObject->graph(16)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::showEntropyLowerThreshold()
{
    double aktualniHodnotaEDP = ui->E_DP->value();
    if (_displayedGraphs == DisplayedGraphs::ENTROPY && ui->E_DPzobraz->isChecked())
    {
        if (std::abs(aktualniHodnotaEDP-thresholds["lowerE"])>0.005)
        {
            LT_entropy.fill(0.0,frameCount);
            thresholds["lowerE"] = aktualniHodnotaEDP;
            LT_entropy.fill(thresholds["lowerE"],frameCount);
            ActualGraphicalObject->graph(5)->setData(valueRange,LT_entropy);
            ActualGraphicalObject->graph(5)->setVisible(true);
            showAffectedFrames(entropy,thresholds["lowerE"],"lesser",17);
            ActualGraphicalObject->replot();
            extremesChanged = true;
        }
        else
        {
            ActualGraphicalObject->graph(5)->setVisible(true);
            ActualGraphicalObject->graph(17)->setVisible(false);
            ActualGraphicalObject->replot();
        }

    }
    else if (_displayedGraphs == DisplayedGraphs::BOTH && ui->E_DPzobraz->isChecked())
    {
        if (std::abs(aktualniHodnotaEDP-thresholds["lowerE"])>0.005)
        {
            LT_entropy.fill(0.0,frameCount);
            thresholds["lowerES"] = (aktualniHodnotaEDP-extremes["minEntropy"])/(extremes["maxEntropy"]-extremes["minEntropy"]);
            LT_entropy.fill(thresholds["lowerES"],frameCount);
            ActualGraphicalObject->graph(5)->setData(valueRange,LT_entropy);
            ActualGraphicalObject->graph(5)->setVisible(true);
            showAffectedFrames(entropyStandard,thresholds["lowerES"],"lesser",17);
            ActualGraphicalObject->replot();
            extremesChanged = true;
        }
        else
        {
            LT_entropy.fill(0.0,frameCount);
            LT_entropy.fill(thresholds["lowerES"],frameCount);
            ActualGraphicalObject->graph(5)->setData(valueRange,LT_entropy);
            ActualGraphicalObject->graph(5)->setVisible(true);
            ActualGraphicalObject->graph(17)->setVisible(false);
            ActualGraphicalObject->replot();
        }
    }
    else if(!ui->E_DPzobraz->isChecked())
    {
        thresholds["lowerE"] = aktualniHodnotaEDP;
        ActualGraphicalObject->graph(5)->setVisible(false);
        ActualGraphicalObject->graph(17)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::showTennengradUpperThreshold()
{
    double aktualHodnotaTHP = ui->T_HP->value();
    //qDebug()<<"Aktualni hodnota THP: "<<aktualHodnotaTHP;
    if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked() &&
            ui->T_HPzobraz->isChecked())
    {
        //qDebug()<<"Zobrazuji graf horního prahu tennengradu.";
        if (std::abs(aktualHodnotaTHP-thresholds["upperT"])>0.005)
        {

            HP_tennengrad.fill(0.0,frameCount);
            thresholds["upperTS"] = (aktualHodnotaTHP-extremes["minTennengrad"])/(extremes["maxTennengrad"]-extremes["minTennengrad"]);
            //qDebug()<<"THP Standardized: "<<thresholds["upperT"]Standardized;
            HP_tennengrad.fill(thresholds["upperTS"],frameCount);
            ActualGraphicalObject->graph(6)->setData(valueRange,HP_tennengrad);
            ActualGraphicalObject->graph(6)->setVisible(true);
            ActualGraphicalObject->replot();
            extremesChanged = true;
        }
        else
        {
            HP_tennengrad.fill(0.0,frameCount);
            HP_tennengrad.fill(thresholds["upperTS"],frameCount);
            ActualGraphicalObject->graph(6)->setData(valueRange,HP_tennengrad);
            ActualGraphicalObject->graph(6)->setVisible(true);
            ActualGraphicalObject->replot();
        }
    }
    else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked() &&
             ui->T_HPzobraz->isChecked())
    {
        if (std::abs(aktualHodnotaTHP-thresholds["upperT"])>0.005)
        {
            HP_tennengrad.fill(0.0,frameCount);
            thresholds["upperT"] = aktualHodnotaTHP;
            HP_tennengrad.fill(thresholds["upperT"],frameCount);
            ActualGraphicalObject->graph(6)->setData(valueRange,HP_tennengrad);
            ActualGraphicalObject->graph(6)->setVisible(true);
            ActualGraphicalObject->replot();
            extremesChanged = true;
        }
        else
        {
            ActualGraphicalObject->graph(6)->setVisible(true);
            ActualGraphicalObject->replot();
        }
    }
    else if (!ui->T_HPzobraz->isChecked())
    {
        thresholds["upperT"] = aktualHodnotaTHP;
        ActualGraphicalObject->graph(6)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::showTennengradLowerThreshold()
{
    double aktualniHodnotaTDP = ui->T_DP->value();
    if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked() &&
            ui->T_DPzobraz->isChecked())
    {
        if (std::abs(aktualniHodnotaTDP-thresholds["lowerT"])>0.005)
        {
            DP_tennengrad.fill(0.0,frameCount);
            thresholds["lowerTS"] = (aktualniHodnotaTDP-extremes["minTennengrad"])/(extremes["maxTennengrad"]-extremes["minTennengrad"]);
            DP_tennengrad.fill(thresholds["lowerTS"],frameCount);
            ActualGraphicalObject->graph(7)->setData(valueRange,DP_tennengrad);
            ActualGraphicalObject->graph(7)->setVisible(true);
            ActualGraphicalObject->replot();
            extremesChanged = true;
        }
        else
        {
            DP_tennengrad.fill(0.0,frameCount);
            DP_tennengrad.fill(thresholds["lowerTS"],frameCount);
            ActualGraphicalObject->graph(7)->setData(valueRange,DP_tennengrad);
            ActualGraphicalObject->graph(7)->setVisible(true);
            ActualGraphicalObject->replot();
        }
    }
    else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked() &&
             ui->T_DPzobraz->isChecked())
    {
        if (std::abs(aktualniHodnotaTDP-thresholds["lowerT"])>0.005)
        {
            DP_tennengrad.fill(0.0,frameCount);
            thresholds["lowerT"] = aktualniHodnotaTDP;
            DP_tennengrad.fill(thresholds["lowerT"],frameCount);
            ActualGraphicalObject->graph(7)->setData(valueRange,DP_tennengrad);
            ActualGraphicalObject->graph(7)->setVisible(true);
            ActualGraphicalObject->replot();
            extremesChanged = true;
        }
        else
        {
            ActualGraphicalObject->graph(7)->setVisible(true);
            ActualGraphicalObject->replot();
        }
    }
    else if(!ui->T_DPzobraz->isChecked())
    {
        thresholds["lowerT"] = aktualniHodnotaTDP;
        ActualGraphicalObject->graph(7)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}*/

void GrafET::showCorrespondingFrames(int i_valueType, int i_evaluationType,
                                     bool showFramesInGraph, bool ETdependency, int i_evaluatioTypeIndex) {
    int graphIdentifier = utilGraph->getNumberReference(EFconnections[i_evaluationType]);
    if (showFramesInGraph && ETdependency) {
        if (_displayedGraphs == ValueType::BOTH) {
            populateGraph(graphIdentifier,
                          ETparametersIntMap[EFconnections[i_evaluationType]],
                          ETparametersDoubleMap[ETconnections[i_valueType+10]],i_evaluatioTypeIndex);
        }
        else {
            populateGraph(graphIdentifier,
                          ETparametersIntMap[EFconnections[i_evaluationType]],
                          ETparametersDoubleMap[ETconnections[i_valueType]],i_evaluatioTypeIndex);
        }
    }
    else if (showFramesInGraph && !ETdependency) {
        int _pomEvalType = i_evaluationType > 12 ? 12 : i_evaluationType;
        if (_displayedGraphs == ValueType::BOTH) {
            populateGraph(graphIdentifier,
                          ETparametersIntMap[EFconnections[_pomEvalType]],
                          ETparametersDoubleMap[ETconnections[ValueType::ENTROPY_STANDARD]],
                          i_evaluatioTypeIndex);
        }
        else if (_displayedGraphs == ValueType::ENTROPY) {
            populateGraph(graphIdentifier,
                          ETparametersIntMap[EFconnections[_pomEvalType]],
                          ETparametersDoubleMap[ETconnections[ValueType::ENTROPY]],
                          i_evaluatioTypeIndex);
        }
        else if (_displayedGraphs == ValueType::TENNENGRAD) {
            populateGraph(graphIdentifier,
                          ETparametersIntMap[EFconnections[_pomEvalType]],
                          ETparametersDoubleMap[ETconnections[ValueType::TENNENGRAD]],
                          i_evaluatioTypeIndex);
        }
        else {
            ActualGraphicalObject->graph(graphIdentifier)->setVisible(false);
            ActualGraphicalObject->replot();
        }
    }
    else {
        ActualGraphicalObject->graph(graphIdentifier)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

/*void GrafET::firstEvaluationEntropy_f()
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (ui->ohodnoceniEntropyCB->isChecked())
    {
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            populateGraph(8,framesEntropyFirstEvalComplete,entropyStandard,0);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            populateGraph(8,framesEntropyFirstEvalComplete,entropy,0);
        }
    }
    else
    {
        ActualGraphicalObject->graph(8)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}
void GrafET::firstEvaluationTennengrad_f()
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (ui->ohodnoceniTennenCB->isChecked())
    {
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            populateGraph(9,framesTennengradFirstEvalComplete,tennengradStandard,0);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            populateGraph(9,framesTennengradFirstEvalComplete,tennengrad,0);
        }
    }
    else
    {
        ActualGraphicalObject->graph(9)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}
void GrafET::firstEvaluation_f()
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (ui->prvniRozhodCB->isChecked())
    {
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            populateGraph(10,framesFirstEvalComplete,entropyStandard,0);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            populateGraph(10,framesFirstEvalComplete,entropy,0);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            populateGraph(10,framesFirstEvalComplete,tennengrad,0);
        }
    }
    else
    {
        ActualGraphicalObject->graph(10)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}
void GrafET::secondEvaluation_f()
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (ui->druheRozhodCB->isChecked())
    {
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            populateGraph(11,framesSecondEvalComplete,entropyStandard,0);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            populateGraph(11,framesSecondEvalComplete,entropy,0);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            populateGraph(11,framesSecondEvalComplete,tennengrad,0);
        }
    }
    else
    {
        ActualGraphicalObject->graph(11)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::on_ohodnocKomplet_stateChanged(int arg1)
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (arg1 == 2){
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            populateGraph(12,framesEvalComplete,entropyStandard,1);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            populateGraph(12,framesEvalComplete,entropy,1);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            populateGraph(12,framesEvalComplete,tennengrad,1);
        }
    }
    if (arg1 == 0){
        ActualGraphicalObject->graph(12)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::on_IV1_stateChanged(int arg1)
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (arg1 == 2){
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            populateGraph(13,framesEvalComplete,entropyStandard,2);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            populateGraph(13,framesEvalComplete,entropy,2);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            populateGraph(13,framesEvalComplete,tennengrad,2);
        }
    }
    if (arg1 == 0){
        ActualGraphicalObject->graph(13)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::on_IV4_stateChanged(int arg1)
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (arg1 == 2){
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
           populateGraph(14,framesEvalComplete,tennengrad,3);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            populateGraph(14,framesEvalComplete,entropy,3);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            populateGraph(14,framesEvalComplete,tennengrad,3);
        }
    }
    if (arg1 == 0){
        ActualGraphicalObject->graph(14)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}

void GrafET::on_IV5_stateChanged(int arg1)
{
    QVector<double> vybraneHodnotyY,souradniceDouble;
    if (arg1 == 2){
        if (ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked())
        {
            populateGraph(15,framesEvalComplete,tennengrad,4);
        }
        else if (ui->zobrazGrafE->isChecked() && !ui->zobrazGrafT->isChecked())
        {
            populateGraph(15,framesEvalComplete,entropy,4);
        }
        else if (!ui->zobrazGrafE->isChecked() && ui->zobrazGrafT->isChecked()) {
            populateGraph(15,framesEvalComplete,tennengrad,4);
        }
    }
    if (arg1 == 0){
        ActualGraphicalObject->graph(15)->setVisible(false);
        ActualGraphicalObject->replot();
    }
}*/

void GrafET::populateGraph(int i_graphLine,
                           QVector<int> &i_y_coords, QVector<double> &i_x_coords,
                           int i_evalType){
    QVector<double> chosenYcoordinates,coordinatesInDouble;
    chosenYcoordinates = utilGraph->chooseFrames(i_y_coords,i_x_coords,i_evalType);
    coordinatesInDouble = utilGraph->convertInt2Double(i_y_coords,i_evalType);
    ActualGraphicalObject->graph(i_graphLine)->setData(coordinatesInDouble,chosenYcoordinates);
    ActualGraphicalObject->graph(i_graphLine)->setVisible(true);
    ActualGraphicalObject->replot();
}

void GrafET::showAffectedFrames(QVector<double> i_affectedValues, double i_valueToCompareWith,
                                QString i_criterion, int i_graphLine){
    QVector<double> selectedData,correspondingFrames;
    for (int index = 0; index < frameCount; index++) {
        if (i_criterion == "greater"){
            if (i_affectedValues[index] > i_valueToCompareWith){
                selectedData.push_back(i_affectedValues[index]);
                correspondingFrames.push_back(double(index));
            }
        }
        else if (i_criterion == "lesser"){
            if (i_affectedValues[index] < i_valueToCompareWith){
                selectedData.push_back(i_affectedValues[index]);
                correspondingFrames.push_back(double(index));
            }
        }
    }
    if (affectedFrames.contains(i_criterion))
        affectedFrames.remove(i_criterion);
    affectedFrames.insert(i_criterion,correspondingFrames);
    ActualGraphicalObject->graph(i_graphLine)->setData(correspondingFrames,selectedData);
    ActualGraphicalObject->graph(i_graphLine)->setVisible(true);
}

void GrafET::hideAll(){
    ActualGraphicalObject->graph(0)->setVisible(false);
    ActualGraphicalObject->graph(1)->setVisible(false);
    ActualGraphicalObject->graph(2)->setVisible(false);
    ActualGraphicalObject->graph(3)->setVisible(false);
    ActualGraphicalObject->graph(4)->setVisible(false);
    ActualGraphicalObject->graph(5)->setVisible(false);
    ActualGraphicalObject->graph(6)->setVisible(false);
    ActualGraphicalObject->graph(7)->setVisible(false);
    ActualGraphicalObject->replot();

    ui->E_HPzobraz->setEnabled(false);ui->E_DPzobraz->setEnabled(false);
    ui->T_HPzobraz->setEnabled(false);ui->T_DPzobraz->setEnabled(false);
    ui->E_DPzobraz->setChecked(false);ui->E_HPzobraz->setChecked(false);
    ui->T_HPzobraz->setChecked(false);ui->T_DPzobraz->setChecked(false);
    ui->ohodnoceniEntropyCB->setEnabled(false);ui->ohodnoceniEntropyCB->setChecked(false);
    ui->ohodnoceniTennenCB->setEnabled(false);ui->ohodnoceniTennenCB->setChecked(false);
    ui->prvniRozhodCB->setEnabled(false);ui->prvniRozhodCB->setChecked(false);
    ui->druheRozhodCB->setEnabled(false);ui->druheRozhodCB->setChecked(false);
    ui->IV1->setEnabled(false);ui->IV1->setChecked(false);
    ui->IV4->setEnabled(false);ui->IV4->setChecked(false);
    ui->IV5->setEnabled(false);ui->IV5->setChecked(false);
}

void GrafET::hideSpecificGraph(int i_identificatorValueType, int i_identificatorThreshold, double newThresholdValue) {
    thresholds[i_identificatorValueType+i_identificatorThreshold] = newThresholdValue;
    int graphIdentificator = utilGraph->getNumberReference(ETconnections[i_identificatorValueType+i_identificatorThreshold]);
    ActualGraphicalObject->graph(graphIdentificator)->setVisible(false);
    ActualGraphicalObject->graph(affectedFramesConnections[i_identificatorValueType+i_identificatorThreshold])->setVisible(false);
    ActualGraphicalObject->replot();
}

SW_VerticalQCPItemLine::SW_VerticalQCPItemLine(QCustomPlot *parentPlot, ColorTheme color)
    : QCPItemLine(parentPlot)
{
    m_lineLabel = new QCPItemText(parentPlot);
    m_linePixmap = new QCPItemPixmap(parentPlot);
    m_lineLabel->setText("");

    if (color == ColorTheme::Light)
    {
        this->setPen(QPen(Qt::lightGray));
        m_lineLabel->setColor(Qt::darkGray);
    }
    else
    {
        this->setPen(QPen(QColor(100, 100, 100)));
        m_lineLabel->setColor(Qt::lightGray);
    }
}

SW_VerticalQCPItemLine::~SW_VerticalQCPItemLine()
{
}

void SW_VerticalQCPItemLine::UpdateLabel(double x, double y, QPixmap i_framePixmap)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(i_framePixmap)
    /*m_lineLabel->setText("test");
    m_lineLabel->position->setType(QCPItemPosition::ptAbsolute);
    m_lineLabel->position->setCoords(x, y);*/

    /*entropyLabel->setText(QString::number(i_E));
    tennengradLabel->setText(QString::number(i_T));
    evalIndex->setText(QString::number(i_EI));*/

    /*m_linePixmap->setPixmap(i_framePixmap);
    m_linePixmap->position(QString(QCPItemPosition::ptAbsolute));
    m_lineLabel->position->setCoords(x, y);*/
}

void SW_VerticalQCPItemLine::SetVisibility(bool i_status){
    //m_lineLabel->setVisible(i_status);
    //m_linePixmap->setVisible(i_status);
    this->setVisible(i_status);
}
