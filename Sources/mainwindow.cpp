#include <QDateTime>
#include "mainwindow.h"
#include "qprocessingdialog.h"
//------------------------------------------------------------------------------------

#define FRAME_MARGIN 5
#define MS_INTERVAL 1000

//------------------------------------------------------------------------------------
const char * MainWindow::QPlotDialogName[]=
{
    "Signal vs frame",
    "Amplitude spectrum",
    "Frame time vs frame",
    "PCA 1-st projection",
    "Filter output vs frame",
    "Signal phase diagram"
};
//------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent)
{
    setWindowTitle(APP_NAME);
    setMinimumSize(320, 240);

    pt_centralWidget = new QBackgroundWidget(NULL, palette().color(backgroundRole()));
    pt_centralWidgetLayout = new QVBoxLayout();
    this->setCentralWidget(pt_centralWidget);
    pt_centralWidget->setLayout(pt_centralWidgetLayout);
    pt_centralWidgetLayout->setMargin(FRAME_MARGIN);

    //--------------------------------------------------------------
    pt_display = new QImageWidget(); // Widgets without a parent are “top level” (independent) widgets. All other widgets are drawn within their parent
    pt_mainLayout = new QVBoxLayout();
    pt_display->setLayout(pt_mainLayout);
    pt_centralWidgetLayout->addWidget(pt_display);

    //--------------------------------------------------------------
    pt_infoLabel = new QLabel(tr("<i>Start new measurement session</i>"));
    pt_infoLabel->setFrameStyle(QFrame::Box | QFrame::Sunken);
    pt_infoLabel->setAlignment(Qt::AlignCenter);
    pt_infoLabel->setWordWrap( true );
    pt_infoLabel->setFont( QFont("MS Shell Dlg 2", 14, QFont::Normal) );
    pt_mainLayout->addWidget(pt_infoLabel);

    //--------------------------------------------------------------
    createActions();
    createMenus();
    createThreads();

    //--------------------------------------------------------------
    m_dialogSetCounter = 0;
    m_sessionsCounter = 0;
    pt_videoSlider = NULL;

    //--------------------------------------------------------------
    m_timer.setTimerType(Qt::PreciseTimer);
    m_timer.setInterval(MS_INTERVAL);
    m_timer.stop();

    //--------------------------------------------------------------
    resize(570, 480);
    statusBar()->showMessage(tr("A context menu is available by right-clicking"));
}
//------------------------------------------------------------------------------------

void MainWindow::createActions()
{
    pt_openSessionAct = new QAction(tr("New &session"),this);
    pt_openSessionAct->setStatusTip(tr("Open new measurement session"));
    connect(pt_openSessionAct, SIGNAL(triggered()), this, SLOT(configure_and_start_session()));

    pt_pauseAct = new QAction(tr("&Pause"), this);
    pt_pauseAct->setStatusTip(tr("Stop a measurement session"));
    connect(pt_pauseAct, SIGNAL(triggered()), this, SLOT(onpause()));

    pt_resumeAct = new QAction(tr("&Resume"), this);
    pt_resumeAct->setStatusTip(tr("Resume a measurement session"));
    connect(pt_resumeAct, SIGNAL(triggered()), this, SLOT(onresume()));

    pt_exitAct = new QAction(tr("E&xit"), this);
    pt_exitAct->setStatusTip(tr("See You next time ;)"));
    connect(pt_exitAct, SIGNAL(triggered()), this, SLOT(close()));

    pt_aboutAct = new QAction(tr("&About"), this);
    pt_aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(pt_aboutAct, SIGNAL(triggered()), this, SLOT(show_about()));

    pt_helpAct = new QAction(tr("&Help"), this);
    pt_helpAct->setStatusTip(tr("Show the application's Help"));
    connect(pt_helpAct, SIGNAL(triggered()), this, SLOT(show_help()));

    pt_deviceResAct = new QAction(tr("&Resolution"), this);
    pt_deviceResAct->setStatusTip(tr("Open a video device resolution dialog"));
    connect(pt_deviceResAct, SIGNAL(triggered()), this, SLOT(opendeviceresolutiondialog()));

    pt_deviceSetAct = new QAction(tr("&Preset"), this);
    pt_deviceSetAct->setStatusTip(tr("Open a video device settings dialog"));
    connect(pt_deviceSetAct, SIGNAL(triggered()), this, SLOT(opendevicesettingsdialog()));

    pt_DirectShowAct = new QAction(tr("&DSpreset"), this);
    pt_DirectShowAct->setStatusTip(tr("Open a device-driver embedded settings dialog"));
    connect(pt_DirectShowAct, SIGNAL(triggered()), this, SLOT(callDirectShowSdialog()));

    pt_fastVisualizationAct = new QAction(tr("Co&ntour"), this);
    pt_fastVisualizationAct->setStatusTip(tr("Switch between contoured or uncontoured style of text on the screen"));
    pt_fastVisualizationAct->setCheckable(true);
    pt_fastVisualizationAct->setChecked(true);
    connect(pt_fastVisualizationAct, SIGNAL(triggered(bool)),pt_display, SLOT(toggle_advancedvisualization(bool)));

    pt_changeColorsAct = new QAction(tr("TextCo&lor"), this);
    pt_changeColorsAct->setStatusTip(tr("Switch between black or white color of text on the screen"));
    connect(pt_changeColorsAct, SIGNAL(triggered()), pt_display, SLOT(switchColorScheme()));

    pt_openPlotDialog = new QAction(tr("&New plot"), this);
    pt_openPlotDialog->setStatusTip(tr("Create a new window for the visualization of appropriate process"));
    connect(pt_openPlotDialog, SIGNAL(triggered()), this, SLOT(createPlotDialog()));

    pt_recordAct = new QAction(tr("&Record"), this);
    pt_recordAct->setStatusTip(tr("Start to record current measurement ssession in to output text file"));
    connect(pt_recordAct, SIGNAL(triggered()), this, SLOT(startRecord()));
    pt_recordAct->setCheckable(true);

    pt_colorActGroup = new QActionGroup(this);
    pt_colorMapper = new QSignalMapper(this);
    pt_redAct = new QAction(tr("Red"), pt_colorActGroup);
    pt_redAct->setStatusTip(tr("Enroll only red channel"));
    pt_redAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_redAct,0);
    connect(pt_redAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_greenAct = new QAction(tr("Green"), pt_colorActGroup);
    pt_greenAct->setStatusTip(tr("Enroll only green channel"));
    pt_greenAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_greenAct,1);
    connect(pt_greenAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_blueAct = new QAction(tr("Blue"), pt_colorActGroup);
    pt_blueAct->setStatusTip(tr("Enroll only blue channel"));
    pt_blueAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_blueAct,2);
    connect(pt_blueAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_allAct = new QAction(tr("RGB"), pt_colorActGroup);
    pt_allAct->setStatusTip(tr("Enroll all channels"));
    pt_allAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_allAct,3);
    connect(pt_allAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_experimentalAct = new QAction(tr("Experimental"), pt_colorActGroup);
    pt_experimentalAct->setStatusTip(tr("Experimental color enrollment scheme"));
    pt_experimentalAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_experimentalAct, 4);
    connect(pt_experimentalAct,SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_greenAct->setChecked(true);

    pt_pcaAct = new QAction(tr("PCA align"), this);
    pt_pcaAct->setStatusTip(tr("Control PCA alignment, affects on result only in harmonic analysis mode"));
    pt_pcaAct->setCheckable(true);

    pt_mapAct = new QAction(tr("Mapping"), this);
    pt_mapAct->setStatusTip(tr("Map clarity of a pulse signal on image"));
    pt_mapAct->setCheckable(true);
    connect(pt_mapAct, SIGNAL(triggered()), this, SLOT(openMapDialog()));

    pt_selectAllAct = new QAction(tr("&Select all"), this);
    pt_selectAllAct->setStatusTip(tr("Select whole image"));
    connect(pt_selectAllAct, SIGNAL(triggered()), pt_display, SLOT(selectWholeImage()));

    pt_skinAct = new QAction(tr("&Only skin"), this);
    pt_skinAct->setStatusTip(tr("Enroll pixels wih color close to skin only"));
    pt_skinAct->setCheckable(true);
    pt_skinAct->setChecked(true);

    pt_adjustAct = new QAction(tr("&Timing"), this);
    pt_adjustAct->setStatusTip(tr("Allows to adjust time between frequency evaluations & data normalization interval"));
    connect(pt_adjustAct, SIGNAL(triggered()), this, SLOT(openProcessingDialog()));
}

//------------------------------------------------------------------------------------

void MainWindow::createMenus()
{
    pt_fileMenu = this->menuBar()->addMenu(tr("&Session"));
    pt_fileMenu->addAction(pt_openSessionAct);
    pt_fileMenu->addSeparator();
    pt_fileMenu->addAction(pt_exitAct);

    //------------------------------------------------
    pt_optionsMenu = menuBar()->addMenu(tr("&Options"));
    pt_optionsMenu->addAction(pt_openPlotDialog);
    pt_optionsMenu->addAction(pt_recordAct);
    pt_optionsMenu->addAction(pt_mapAct);
    pt_optionsMenu->addSeparator();
    pt_colormodeMenu = pt_optionsMenu->addMenu(tr("&Light"));
    pt_colormodeMenu->addActions(pt_colorActGroup->actions());
    pt_modeMenu = pt_optionsMenu->addMenu(tr("&Mode"));
    pt_modeMenu->addAction(pt_pcaAct);
    pt_modeMenu->addSeparator();
    pt_modeMenu->addAction(pt_skinAct);
    pt_optionsMenu->setEnabled(false);

    pt_appearenceMenu = menuBar()->addMenu(tr("&Appearence"));
    pt_appearenceMenu->addAction(pt_fastVisualizationAct);
    pt_appearenceMenu->addAction(pt_changeColorsAct);

    //-------------------------------------------------
    pt_deviceMenu = menuBar()->addMenu(tr("&Device"));
    pt_deviceMenu->addAction(pt_deviceSetAct);
    pt_deviceMenu->addAction(pt_deviceResAct);
    pt_deviceMenu->addSeparator();
    pt_deviceMenu->addAction(pt_DirectShowAct);

    //--------------------------------------------------
    pt_helpMenu = menuBar()->addMenu(tr("&Help"));
    pt_helpMenu->addAction(pt_helpAct);
    pt_helpMenu->addAction(pt_aboutAct);
}

//------------------------------------------------------------------------------------

void MainWindow::createThreads()
{
    //-------------------Pointers for objects------------------------
    pt_improcThread = new QThread(this); // Make an own QThread for opencv interface
    pt_opencvProcessor = new QOpencvProcessor();
    pt_opencvProcessor->moveToThread( pt_improcThread );
    connect(pt_improcThread, SIGNAL(finished()), pt_opencvProcessor, SLOT(deleteLater()));
    connect(pt_skinAct, SIGNAL(triggered(bool)), pt_opencvProcessor, SLOT(setSkinSearchingFlag(bool)));
    //---------------------------------------------------------------

    pt_harmonicProcessor = NULL;
    pt_harmonicThread = NULL;
    pt_map = NULL;
    pt_mapThread = NULL;

    //--------------------QVideoCapture------------------------------
    pt_videoThread = new QThread(this);
    pt_videoCapture = new QVideoCapture();
    pt_videoCapture->moveToThread(pt_videoThread);
    connect(pt_videoThread, &QThread::started, pt_videoCapture, &QVideoCapture::initiallizeTimer);
    connect(pt_videoThread, &QThread::finished, pt_videoCapture, &QVideoCapture::deleteLater);

    //----------Register openCV types in Qt meta-type system---------
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<cv::Rect>("cv::Rect");

    //----------------------Connections------------------------------
    connect(pt_opencvProcessor, SIGNAL(frameProcessed(cv::Mat,double,quint32)), pt_display, SLOT(updateImage(cv::Mat,double,quint32)));
    connect(pt_display, SIGNAL(rect_was_entered(cv::Rect)), pt_opencvProcessor, SLOT(setRect(cv::Rect)));
    connect(pt_opencvProcessor, SIGNAL(selectRegion(const char*)), pt_display, SLOT(set_warning_status(const char*)));
    connect(pt_opencvProcessor, SIGNAL(mapRegionUpdated(cv::Rect)), pt_display, SLOT(updadeMapRegion(cv::Rect)));
    connect(this, &MainWindow::pauseVideo, pt_videoCapture, &QVideoCapture::pause);
    connect(this, &MainWindow::resumeVideo, pt_videoCapture, &QVideoCapture::resume);
    connect(this, &MainWindow::closeVideo, pt_videoCapture, &QVideoCapture::close);
    connect(this, &MainWindow::updateTimer, pt_opencvProcessor, &QOpencvProcessor::updateTime);
    //----------------------Thread start-----------------------------
    pt_improcThread->start();
    pt_videoThread->start();
}

//------------------------------------------------------------------------------------

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(pt_selectAllAct);
    //menu.addAction(pt_openSessionAct);
    menu.addSeparator();
    menu.addAction(pt_pauseAct);
    menu.addAction(pt_resumeAct);
    menu.addSeparator();
    menu.addAction(pt_adjustAct);
    menu.addSeparator();   
    menu.exec(event->globalPos());
}

//------------------------------------------------------------------------------------

bool MainWindow::openvideofile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open video file"), "/video", tr("Video (*.avi *.mp4 *.wmv)"));
    while( !pt_videoCapture->openfile(fileName) ) {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open video file!"), QMessageBox::Open | QMessageBox::Cancel, this, Qt::Dialog);
        if( msgBox.exec() == QMessageBox::Open )
        {
            fileName = QFileDialog::getOpenFileName(this, tr("Open video file"), "/video", tr("Video (*.avi *.mp4 *.wmv)"));
        } else {
            return false;
        }
    }
    if ( pt_infoLabel ) {
        pt_mainLayout->removeWidget(pt_infoLabel);
        delete pt_infoLabel;
        pt_infoLabel = NULL;
    }

    delete pt_videoSlider;
    pt_videoSlider = NULL;
    pt_videoSlider = new QVideoSlider(this);
    pt_videoSlider->setRange(0, (int)pt_videoCapture->getFrameCounts());
    pt_videoSlider->setOrientation(Qt::Horizontal);
    pt_videoSlider->setTickPosition(QSlider::TicksBothSides);
    pt_videoSlider->setTickInterval(32);
    pt_centralWidgetLayout->addWidget(pt_videoSlider);
    connect(pt_videoCapture, SIGNAL(capturedFrameNumber(int)), pt_videoSlider, SLOT(setValue(int)));
    connect(pt_videoSlider, SIGNAL(sliderPressed()), this, SLOT(onpause()));
    connect(pt_videoSlider, SIGNAL(sliderReleased(int)), pt_videoCapture, SLOT(setFrameNumber(int)));
    connect(pt_videoSlider, SIGNAL(sliderReleased(int)), this, SLOT(onresume()));
    return true;
}


//------------------------------------------------------------------------------------

bool MainWindow::opendevice()
{
    pt_videoCapture->open_deviceSelectDialog();
    while( !pt_videoCapture->opendevice() )
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open device!"), QMessageBox::Open | QMessageBox::Cancel, this, Qt::Dialog);
        if( msgBox.exec() == QMessageBox::Open )
        {
            pt_videoCapture->open_deviceSelectDialog();
        } else {
            return false;
        }
    }
    if ( pt_infoLabel ) {
        pt_mainLayout->removeWidget(pt_infoLabel);
        delete pt_infoLabel;
        pt_infoLabel = NULL;
    }

    delete pt_videoSlider;
    pt_videoSlider = NULL;
    return true;
}

//------------------------------------------------------------------------------------

void MainWindow::show_about()
{
   QDialog *aboutdialog = new QDialog();
   aboutdialog->setWindowTitle("About dialog");
   aboutdialog->setFixedSize(232,128);

   QVBoxLayout *templayout = new QVBoxLayout();
   templayout->setMargin(5);

   QLabel *projectname = new QLabel( QString(APP_NAME) + " " + QString(APP_VERSION) );
   projectname->setFrameStyle(QFrame::Box | QFrame::Raised);
   projectname->setAlignment(Qt::AlignCenter);
   QLabel *projectauthors = new QLabel( QString(APP_AUTHOR) + "\n\n" + QString(APP_COMPANY) + "\n\n" + QString(APP_RELEASE_DATE) );
   projectauthors->setAlignment(Qt::AlignCenter);
   QLabel *hyperlink = new QLabel( APP_EMAIL );
   hyperlink->setToolTip("Tap here to send an email");
   hyperlink->setOpenExternalLinks(true);
   hyperlink->setAlignment(Qt::AlignCenter);

   templayout->addWidget(projectname);
   templayout->addWidget(projectauthors);
   templayout->addWidget(hyperlink);

   aboutdialog->setLayout(templayout);
   aboutdialog->exec();

   delete hyperlink;
   delete projectauthors;
   delete projectname;
   delete templayout;
   delete aboutdialog;
}

//------------------------------------------------------------------------------------

void MainWindow::show_help()
{
    if (!QDesktopServices::openUrl(QUrl("https://github.com/pi-null-mezon/qpulsecapture.git", QUrl::TolerantMode))) // runs the ShellExecute function on Windows
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open help file"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}

//------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
    emit closeVideo();

     if(m_saveFile.isOpen())
    {
        m_saveFile.close();
    }

    pt_videoThread->quit();
    pt_videoThread->wait();

    pt_improcThread->quit();
    pt_improcThread->wait();

    if(pt_map)
    {
        pt_mapThread->quit();
        pt_mapThread->wait();
    }

    if(pt_harmonicThread)
    {
        pt_harmonicThread->quit();
        pt_harmonicThread->wait();
    }
}

//------------------------------------------------------------------------------------

void MainWindow::onpause()
{
    emit pauseVideo();
    m_timer.stop();
}

//------------------------------------------------------------------------------------

void MainWindow::onresume()
{
    emit resumeVideo();
    emit updateTimer();
    m_timer.start();
}

//-----------------------------------------------------------------------------------

void MainWindow::opendeviceresolutiondialog()
{
    onpause();
    if( !pt_videoCapture->open_resolutionDialog() )
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open device resolution dialog!"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
    onresume();
}

//-----------------------------------------------------------------------------------

void MainWindow::opendevicesettingsdialog()
{
    if( !pt_videoCapture->open_settingsDialog() )
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open device settings dialog!"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}

//-----------------------------------------------------------------------------------

void MainWindow::callDirectShowSdialog()
{   
    if (!QProcess::startDetached(QString("WVCF_utility.exe"),QStringList("-l -c")))// runs the ShellExecute function on Windows
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open utility!"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}

//----------------------------------------------------------------------------------------

void MainWindow::configure_and_start_session()
{
    this->onpause();
    if(m_settingsDialog.exec() == QDialog::Accepted)
    {     
        closeAllDialogs();
        if(pt_harmonicProcessor)
        {
            pt_harmonicThread->quit();
            pt_harmonicThread->wait();
        }       
        //---------------------Harmonic processor------------------------
        pt_harmonicThread = new QThread(this);
        pt_harmonicProcessor = new QHarmonicProcessor(NULL, m_settingsDialog.get_datalength(), m_settingsDialog.get_bufferlength());
        pt_harmonicProcessor->moveToThread(pt_harmonicThread);
        connect(pt_harmonicThread, SIGNAL(finished()),pt_harmonicProcessor, SLOT(deleteLater()));
        connect(pt_harmonicThread, SIGNAL(finished()),pt_harmonicThread, SLOT(deleteLater()));
        //---------------------------------------------------------------
        if(m_saveFile.isOpen()) {
            m_saveFile.close();
            pt_recordAct->setChecked(false);
        }
        //---------------------------------------------------------------
        if(m_settingsDialog.get_customPatientFlag())
        {
            if(pt_harmonicProcessor->loadWarningRates(m_settingsDialog.get_stringDistribution().toLocal8Bit().constData(),(QHarmonicProcessor::SexID)m_settingsDialog.get_patientSex(),m_settingsDialog.get_patientAge(),(QHarmonicProcessor::TwoSideAlpha)m_settingsDialog.get_patientPercentile()) == QHarmonicProcessor::FileExistanceError)
            {
                QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not find population distribution file"), QMessageBox::Ok, this, Qt::Dialog);
                msgBox.exec();
            }
        }
        //---------------------------------------------------------------
        disconnect(pt_videoCapture,SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(faceProcess(cv::Mat)));
        disconnect(pt_videoCapture,SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(rectProcess(cv::Mat)));
        if(m_settingsDialog.get_flagCascade())
        {
            QString filename = m_settingsDialog.get_stringCascade();
            while(!pt_opencvProcessor->loadClassifier(filename.toStdString()))
            {
                QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not load classifier file"), QMessageBox::Ok | QMessageBox::Open, this, Qt::Dialog);
                if(msgBox.exec() == QMessageBox::Open)
                {
                    QString temp_filename = QFileDialog::getOpenFileName(this, tr("Open file"), "haarcascades/", tr("Cascade (*.xml)"));
                    if(!temp_filename.isEmpty())
                    {
                        filename = temp_filename;
                    }
                }
                else
                {
                    break;
                }
            }
            connect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(faceProcess(cv::Mat)), Qt::BlockingQueuedConnection);
        }
        else
        {
            connect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(rectProcess(cv::Mat)), Qt::BlockingQueuedConnection);
        }
        //--------------------------------------------------------------      
        if(m_settingsDialog.get_FFTflag())
        {
            connect(&m_timer, SIGNAL(timeout()), pt_harmonicProcessor, SLOT(ComputeFrequency()));
        }
        else
        {
            connect(&m_timer, SIGNAL(timeout()), pt_harmonicProcessor, SLOT(CountFrequency()));
        }
        connect(pt_opencvProcessor, SIGNAL(dataCollected(ulong,ulong,ulong,ulong,double)), pt_harmonicProcessor, SLOT(EnrollData(ulong,ulong,ulong,ulong,double)));
        connect(pt_harmonicProcessor, SIGNAL(TooNoisy(qreal)), pt_display, SLOT(clearFrequencyString(qreal)));
        connect(pt_harmonicProcessor, SIGNAL(HeartRateUpdated(qreal,qreal,bool)), pt_display, SLOT(updateValues(qreal,qreal,bool)));
        connect(pt_colorMapper, SIGNAL(mapped(int)), pt_harmonicProcessor, SLOT(switchColorMode(int)));      
        connect(pt_pcaAct, SIGNAL(triggered(bool)), pt_harmonicProcessor, SLOT(setPCAMode(bool)));
        connect(pt_harmonicProcessor, SIGNAL(CurrentValues(qreal,qreal,qreal,qreal,qreal,qreal)), this, SLOT(make_record_to_file(qreal,qreal,qreal,qreal,qreal,qreal)));
        pt_harmonicThread->start();

        m_timer.setInterval( m_settingsDialog.get_timerValue() );
        pt_optionsMenu->setEnabled(true);
        pt_greenAct->trigger(); // because green channel is default in QHarmonicProcessor

        if(m_settingsDialog.get_flagVideoFile())
        {
            if(this->openvideofile()) {
                if(m_sessionsCounter == 0)
                    QTimer::singleShot(2500, this, SLOT(onresume())); // should solve issue with first launch suspension
                else
                    this->onresume();
            }
        }
        else
        {
            if(this->opendevice()) {
                if(m_sessionsCounter == 0)
                    QTimer::singleShot(2500, this, SLOT(onresume())); // should solve issue with first launch suspension
                else
                    this->onresume();     // should solve issue with first launch suspension
            }
        }
        this->statusBar()->showMessage(tr("Plot options available through Menu->Options->New plot"));
        m_sessionsCounter++;
    } else {
        //pt_optionsMenu->setEnabled(false);
        //emit closeVideo();
        statusBar()->showMessage(tr("You can prolong Paused session by means of Resume option in context menu"));
    }
}

//------------------------------------------------------------------------------------------

void MainWindow::createPlotDialog()
{
    if(m_dialogSetCounter < LIMIT_OF_DIALOGS_NUMBER)
    {
        QDialog dialog;
        QVBoxLayout centralLayout;
        dialog.setLayout(&centralLayout);
        QGroupBox groupBox(tr("Select appropriate plot type:"));
        QHBoxLayout buttonsLayout;
        centralLayout.addWidget(&groupBox);
        centralLayout.addLayout(&buttonsLayout);
        QPushButton acceptButton("Accept");
        QPushButton rejectButton("Cancel");
        buttonsLayout.addWidget(&acceptButton);
        buttonsLayout.addWidget(&rejectButton);
        connect(&acceptButton, &QPushButton::clicked, &dialog, &QDialog::accept);
        connect(&rejectButton, &QPushButton::clicked, &dialog, &QDialog::reject);

        QVBoxLayout groupBoxLayout;
        groupBox.setLayout(&groupBoxLayout);
        QComboBox dialogTypeComboBox;
        groupBoxLayout.addWidget(&dialogTypeComboBox);
        for(quint8 i = 0; i < sizeof(QPlotDialogName)/sizeof(char*); i++)
        {
            dialogTypeComboBox.addItem( QPlotDialogName[i] );
        }
        dialogTypeComboBox.setCurrentIndex(0);

        dialog.setWindowTitle("Plot select dialog");
        dialog.setMinimumSize(256,128);

        if(dialog.exec() == QDialog::Accepted)
        {
            pt_dialogSet[ m_dialogSetCounter ] = new QDialog(NULL, Qt::Window);
            pt_dialogSet[ m_dialogSetCounter ]->setWindowTitle(dialogTypeComboBox.currentText() + " plot");
            pt_dialogSet[ m_dialogSetCounter ]->setAttribute(Qt::WA_DeleteOnClose, true);
            connect(pt_dialogSet[ m_dialogSetCounter ], SIGNAL(destroyed()), this, SLOT(decrease_dialogSetCounter()));
            pt_dialogSet[ m_dialogSetCounter ]->setMinimumSize(480, 320);

            QVBoxLayout *pt_layout = new QVBoxLayout( pt_dialogSet[ m_dialogSetCounter ] );
            pt_layout->setMargin(FRAME_MARGIN);
            QEasyPlot *pt_plot = new QEasyPlot( pt_dialogSet[ m_dialogSetCounter ] );
            pt_layout->addWidget( pt_plot );          
                switch(dialogTypeComboBox.currentIndex())
                {
                    case 0: // Signal trace
                        connect(pt_harmonicProcessor, SIGNAL(SignalUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","Centered & normalized signal");
                        pt_plot->set_vertical_Borders(-4.0,4.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                        break;
                    case 1: // Spectrum trace
                        connect(pt_harmonicProcessor, SIGNAL(SpectrumUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Freq.count","DFT amplitude spectrum");
                        pt_plot->set_vertical_Borders(0.0,1.0);
                        pt_plot->set_coordinatesPrecision(0,1);
                        pt_plot->set_DrawRegime(QEasyPlot::FilledTraceRegime);
                        break;
                    case 2: // Time trace
                        connect(pt_harmonicProcessor, SIGNAL(TimeUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","processing period per frame, ms");
                        pt_plot->set_vertical_Borders(0.0,100.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                        pt_plot->set_DrawRegime(QEasyPlot::FilledTraceRegime);
                        break;
                    case 3: // PCA 1st projection trace
                        connect(pt_harmonicProcessor, SIGNAL(PCAProjectionUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","Normalised & centered projection on 1-st PCA direction");
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                    break;
                    case 4: // Digital filter output
                        connect(pt_harmonicProcessor, SIGNAL(BinaryOutputUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","Digital derivative after smoothing");
                        pt_plot->set_vertical_Borders(-2.0,2.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                    break;
                    case 5: // signal phase shift
                        connect(pt_harmonicProcessor, SIGNAL(SignalUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_DrawRegime(QEasyPlot::PhaseRegime);
                        pt_plot->set_axis_names("Signal count","Signal count");
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_horizontal_Borders(-5.0, 5.0);
                        pt_plot->set_X_Ticks(11);
                        pt_plot->set_coordinatesPrecision(2,2);
                    break;
                }
            pt_dialogSet[ m_dialogSetCounter ]->setContextMenuPolicy(Qt::ActionsContextMenu);
            QAction *pt_actionFont = new QAction(tr("Axis font"), pt_dialogSet[ m_dialogSetCounter ]);
            connect(pt_actionFont, SIGNAL(triggered()), pt_plot, SLOT(open_fontSelectDialog()));
            QAction *pt_actionTraceColor = new QAction(tr("Trace color"), pt_dialogSet[ m_dialogSetCounter ]);
            connect(pt_actionTraceColor, SIGNAL(triggered()), pt_plot, SLOT(open_traceColorDialog()));
            QAction *pt_actionBGColor = new QAction(tr("BG color"), pt_dialogSet[ m_dialogSetCounter ]);
            connect(pt_actionBGColor, SIGNAL(triggered()), pt_plot, SLOT(open_backgroundColorDialog()));
            QAction *pt_actionCSColor = new QAction(tr("CS color"), pt_dialogSet[ m_dialogSetCounter ]);
            connect(pt_actionCSColor, SIGNAL(triggered()), pt_plot, SLOT(open_coordinatesystemColorDialog()));
            pt_dialogSet[ m_dialogSetCounter ]->addAction(pt_actionTraceColor);
            pt_dialogSet[ m_dialogSetCounter ]->addAction(pt_actionBGColor);
            pt_dialogSet[ m_dialogSetCounter ]->addAction(pt_actionCSColor);
            pt_dialogSet[ m_dialogSetCounter ]->addAction(pt_actionFont);
            pt_dialogSet[ m_dialogSetCounter ]->show();
            m_dialogSetCounter++;
        }
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("You came up to a limit of dialogs ailable"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }

}

//------------------------------------------------------------------------------------------

void MainWindow::decrease_dialogSetCounter()
{
    m_dialogSetCounter--;
}

//-------------------------------------------------------------------------------------------

void MainWindow::make_record_to_file(qreal signalValue, qreal meanRed, qreal meanGreen, qreal meanBlue, qreal freqValue, qreal snrValue)
{
    if(m_saveFile.isOpen())
    {
        m_textStream << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
                     << "\t" << signalValue << "\t" << meanRed << "\t" << meanGreen
                     << "\t" << meanBlue << "\t" << qRound(freqValue) << "\t" << snrValue << "\n";
    }
}

//-------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent*)
{
    closeAllDialogs();
}

//-------------------------------------------------------------------------------------------

void MainWindow::closeAllDialogs()
{
    for(qint8 i = m_dialogSetCounter; i > 0; i--)
    {
        pt_dialogSet[ i-1 ]->close(); // there is no need to explicitly decrement m_dialogSetCounter value because pt_dialogSet[i] was preset to Qt::WA_DeleteOnClose flag and on_destroy of pt_dialogSet[i] 'this' will decrease counter automatically
    };
}

//-------------------------------------------------------------------------------------------

void MainWindow::startRecord()
{
    if(m_saveFile.isOpen()) {
        m_saveFile.close();
        pt_recordAct->setChecked(false);
        QMessageBox msgBox(QMessageBox::Question, this->windowTitle(), tr("Another record?"), QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog);
        if(msgBox.exec() == QMessageBox::No)
        {
            return;
        }
    }

    m_saveFile.setFileName(QFileDialog::getSaveFileName(this,tr("Save record to file"),"Records/record.txt", "Text file (*.txt)"));

    while(!m_saveFile.open(QIODevice::WriteOnly))   {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not save file with such name, try another name"), QMessageBox::Save | QMessageBox::Cancel, this, Qt::Dialog);
        if(msgBox.exec() == QMessageBox::Save) {
            m_saveFile.setFileName(QFileDialog::getSaveFileName(this,tr("Save record to a file"),"Records/record.txt", tr("Text file (*.txt)")));
        }
        else {
            pt_recordAct->setChecked(false);
            break;
        }
    }

    if(m_saveFile.isOpen()) {
        pt_recordAct->setChecked(true);
        m_textStream.setDevice(&m_saveFile);
        m_textStream.setRealNumberNotation(QTextStream::FixedNotation);
        m_textStream.setRealNumberPrecision(3);
        m_textStream << "QPULSECAPTURE RECORD " << QDateTime::currentDateTime().toString("dd.MM.yyyy")
                     << "\nhh:mm:ss.zzz\tCNSignal\tMeanRed\tMeanGreen\tMeanBlue\tPulseRate,bpm\tSNR,dB\n";
    }
}

//----------------------------------------------------------------------------------------------

void MainWindow::openMapDialog()
{

    cv::Rect tempRect = pt_opencvProcessor->getRect();
    if((tempRect.width <= 0) || (tempRect.height <= 0))
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Select region on image first"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
        pt_mapAct->setChecked(false);
        return;
    }
    else
    {
        if(!pt_mapAct->isChecked())
        {
            QMessageBox msgBox(QMessageBox::Question, this->windowTitle(), tr("Another map?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, this, Qt::Dialog);
            int resultCode = msgBox.exec();
            if(resultCode == QMessageBox::No)
            {
                if(pt_map)
                {
                    if(pt_map)
                    {
                        pt_mapThread->quit();
                        pt_mapThread->wait();
                    }
                    disconnect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(mapProcess(cv::Mat)));
                    disconnect(&m_timer, SIGNAL(timeout()), pt_map, SIGNAL(updateMap()));
                    disconnect(pt_map, SIGNAL(mapUpdated(const qreal*,quint32,quint32,qreal,qreal)), pt_display, SLOT(updateMap(const qreal*,quint32,quint32,qreal,qreal)));
                    pt_display->updateMap(NULL,0,0,0.0,0.0);
                    delete pt_map;
                    pt_map = NULL;

                }
                pt_mapAct->setChecked(false);
                return;
            }
            if(resultCode == QMessageBox::Cancel)
            {
                pt_mapAct->setChecked(true);
                return;
            }
        }

        if(pt_map)
        {
            if(pt_map)
            {
                pt_mapThread->quit();
                pt_mapThread->wait();
            }
            disconnect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(mapProcess(cv::Mat)));
            disconnect(&m_timer, SIGNAL(timeout()), pt_map, SIGNAL(updateMap()));
            disconnect(pt_map, SIGNAL(mapUpdated(const qreal*,quint32,quint32,qreal,qreal)), pt_display, SLOT(updateMap(const qreal*,quint32,quint32,qreal,qreal)));
            pt_display->updateMap(NULL,0,0,0.0,0.0);
            delete pt_map;
            pt_map = NULL;
        }

        mappingdialog dialog;
        dialog.setImageHeight(tempRect.height);
        dialog.setImageWidth(tempRect.width);

        if(dialog.exec() == QDialog::Accepted)
        {            
            pt_opencvProcessor->setMapCellSize(dialog.getCellSize(), dialog.getCellSize());
            pt_opencvProcessor->setMapRegion(cv::Rect(tempRect.x,tempRect.y,dialog.getCellSize()*dialog.getMapWidth(),dialog.getCellSize()*dialog.getMapHeight()));

            pt_mapThread = new QThread(this);
            pt_map = new QHarmonicProcessorMap(NULL, dialog.getMapWidth(), dialog.getMapHeight());
            pt_map->moveToThread(pt_mapThread);
            connect(pt_opencvProcessor, SIGNAL(mapCellProcessed(ulong,ulong,ulong,ulong,double)), pt_map, SLOT(updateHarmonicProcessor(ulong,ulong,ulong,ulong,double)));
            connect(&m_timer, SIGNAL(timeout()), pt_map, SIGNAL(updateMap()));
            connect(pt_map, SIGNAL(mapUpdated(const qreal*,quint32,quint32,qreal,qreal)), pt_display, SLOT(updateMap(const qreal*,quint32,quint32,qreal,qreal)));
            connect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(mapProcess(cv::Mat)), Qt::BlockingQueuedConnection);
            connect(pt_pcaAct, SIGNAL(triggered(bool)), pt_map, SIGNAL(updatePCAMode(bool)));
            connect(pt_colorMapper, SIGNAL(mapped(int)), pt_map, SIGNAL(changeColorChannel(int)));
            connect(pt_mapThread, SIGNAL(finished()), pt_mapThread, SLOT(deleteLater()));
            pt_mapThread->start();
            pt_mapAct->setChecked(true);
        }
        else
        {
            pt_mapAct->setChecked(false);
        }
    }
}

void MainWindow::openProcessingDialog()
{
    if(pt_harmonicProcessor) {

        QProcessingDialog *dialog = new QProcessingDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        dialog->setTimer(m_timer.interval());
        dialog->setMaximumInterval(pt_harmonicProcessor->getDataLength());
        dialog->setInterval(pt_harmonicProcessor->getEstimationInterval());
        connect(dialog, &QProcessingDialog::timerValueUpdated, &m_timer, &QTimer::setInterval);
        connect(dialog, SIGNAL(intervalValueUpdated(int)), pt_harmonicProcessor, SLOT(setEstiamtionInterval(int)));
        dialog->show();

    } else {

        QMessageBox msg(QMessageBox::Information, tr("Warning"),tr("Start new session before!") );
        msg.exec();

    }

}

