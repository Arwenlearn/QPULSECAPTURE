#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//------------------------------------------------------------------------------------------------------

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QString>
#include <QLayout>
#include <QLabel>
#include <QThread>
#include <QStatusBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QActionGroup>
#include <QSignalMapper>

#include "qimagewidget.h"
#include "qopencvprocessor.h"
#include "qvideocapture.h"
#include "about.h"
#include "qharmonicprocessor.h"
#include "qharmonicmap.h"
#include "qsettingsdialog.h"
#include "qeasyplot.h"
#include "qbackgroundwidget.h"
#include "mappingdialog.h"
#include "qvideoslider.h"

#include "QKeyEvent"

#define LIMIT_OF_DIALOGS_NUMBER 5
//------------------------------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void pauseVideo();
    void resumeVideo();
    void closeVideo();
    void updateTimer();

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void closeEvent(QCloseEvent *);

public slots:
    bool openvideofile(); // for video file open
    bool opendevice(); // for video device open
    void opendeviceresolutiondialog();
    void opendevicesettingsdialog();
    void createPlotDialog();
    void callDirectShowSdialog();
    void show_about();
    void show_help();
    void onpause();
    void onresume();
    void configure_and_start_session();
    void startRecord();
    void startMeasurementsRecord();
    void openMapDialog();
    void openProcessingDialog();

private:
    void createActions();
    void createMenus();
    void createTimers();
    void createThreads();
    QImageWidget *pt_display;
    QVBoxLayout *pt_mainLayout;
    QBackgroundWidget *pt_centralWidget;
    QVBoxLayout *pt_centralWidgetLayout;
    QVideoSlider *pt_videoSlider;
    QLabel *pt_infoLabel;
    QLabel *pt_statusLabel;
    QAction *pt_openSessionAct;
    QAction *pt_exitAct;
    QAction *pt_aboutAct;
    QAction *pt_helpAct;
    QAction *pt_pauseAct;
    QAction *pt_resumeAct;
    QAction *pt_deviceResAct;
    QAction *pt_deviceSetAct;
    QAction *pt_DirectShowAct;
    QAction *pt_fastVisualizationAct;
    QAction *pt_changeColorsAct;
    QAction *pt_openPlotDialog;
    QAction *pt_recordAct;
    QAction *pt_mapAct;
    QAction *pt_selectAllAct;
    QAction *pt_skinAct;
    QAction *pt_adjustAct;
    QAction *pt_imageAct;
    QAction *pt_calibAct;
    QAction *pt_measRecAct;
    QAction *pt_prunAct;
    QAction *pt_fillAct;
    QMenu *pt_RecordsMenu;
    QMenu *pt_fileMenu;
    QMenu *pt_optionsMenu;
    QMenu *pt_deviceMenu;
    QMenu *pt_helpMenu;
    QMenu *pt_colormodeMenu;
    QMenu *pt_modeMenu;
    QMenu *pt_appearenceMenu;
    QVideoCapture *pt_videoCapture;
    QOpencvProcessor *pt_opencvProcessor;
    QThread *pt_improcThread;
    QThread *pt_harmonicThread;
    QThread *pt_videoThread;
    QThread *pt_mapThread;
    QHarmonicProcessor *pt_harmonicProcessor;
    QTimer m_timer;
    QDialog *pt_dialogSet[LIMIT_OF_DIALOGS_NUMBER];
    quint8 m_dialogSetCounter;
    QFile m_signalsFile;
    QTextStream m_signalsStream;
    QFile m_measurementsFile;
    QTextStream m_measurementsStream;
    static const char *QPlotDialogName[];

    QActionGroup *pt_colorActGroup;
    QSignalMapper *pt_colorMapper;
    QAction *pt_redAct;
    QAction *pt_blueAct;
    QAction *pt_greenAct;
    QAction *pt_allAct;
    QAction *pt_pcaAct;
    QAction *pt_experimentalAct;

    QHarmonicProcessorMap *pt_map;
    QSettingsDialog m_settingsDialog;

    quint16 m_sessionsCounter;

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void decrease_dialogSetCounter();
    void closeAllDialogs();
    void make_record_to_file(qreal signalValue, qreal meanRed, qreal meanGreen, qreal meanBlue);
    void updateMeasurementsRecord(qreal heartRate, qreal heartSNR, qreal breathRate, qreal breathSNR, qreal acR, qreal dcR, qreal acG, qreal dcG, qreal acB, qreal dcB);
    void updateStatus(qreal value);
};
//------------------------------------------------------------------------------------------------------
#endif // MAINWINDOW_H
