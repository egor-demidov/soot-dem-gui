/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionSave;
    QAction *actionSave_2;
    QAction *actionOpen;
    QAction *actionAdvance_One_Step;
    QAction *actionAdvance_Continuously;
    QAction *actionPause;
    QAction *actionNew;
    QWidget *centralwidget;
    QGridLayout *gridLayout_2;
    QVBoxLayout *verticalLayout;
    QFrame *frame;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *newButton;
    QToolButton *openButton;
    QToolButton *saveButton;
    QFrame *line;
    QToolButton *playButton;
    QToolButton *playAllButton;
    QToolButton *pauseButton;
    QSpacerItem *horizontalSpacer;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_2;
    QSplitter *splitter_2;
    QSplitter *splitter;
    QTableWidget *parameterTable;
    QPlainTextEdit *stdoutBox;
    QWidget *previewWidget;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuSimulation;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 800);
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName("actionSave");
        actionSave_2 = new QAction(MainWindow);
        actionSave_2->setObjectName("actionSave_2");
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName("actionOpen");
        actionAdvance_One_Step = new QAction(MainWindow);
        actionAdvance_One_Step->setObjectName("actionAdvance_One_Step");
        actionAdvance_Continuously = new QAction(MainWindow);
        actionAdvance_Continuously->setObjectName("actionAdvance_Continuously");
        actionPause = new QAction(MainWindow);
        actionPause->setObjectName("actionPause");
        actionNew = new QAction(MainWindow);
        actionNew->setObjectName("actionNew");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        gridLayout_2 = new QGridLayout(centralwidget);
        gridLayout_2->setObjectName("gridLayout_2");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        frame = new QFrame(centralwidget);
        frame->setObjectName("frame");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy);
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Raised);
        horizontalLayout_2 = new QHBoxLayout(frame);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        newButton = new QToolButton(frame);
        newButton->setObjectName("newButton");
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/new.png"), QSize(), QIcon::Normal, QIcon::Off);
        newButton->setIcon(icon);
        newButton->setIconSize(QSize(32, 32));

        horizontalLayout_2->addWidget(newButton);

        openButton = new QToolButton(frame);
        openButton->setObjectName("openButton");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/open.png"), QSize(), QIcon::Normal, QIcon::Off);
        openButton->setIcon(icon1);
        openButton->setIconSize(QSize(32, 32));

        horizontalLayout_2->addWidget(openButton);

        saveButton = new QToolButton(frame);
        saveButton->setObjectName("saveButton");
        saveButton->setEnabled(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        saveButton->setIcon(icon2);
        saveButton->setIconSize(QSize(32, 32));

        horizontalLayout_2->addWidget(saveButton);

        line = new QFrame(frame);
        line->setObjectName("line");
        line->setFrameShape(QFrame::Shape::VLine);
        line->setFrameShadow(QFrame::Shadow::Sunken);

        horizontalLayout_2->addWidget(line);

        playButton = new QToolButton(frame);
        playButton->setObjectName("playButton");
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/play.png"), QSize(), QIcon::Normal, QIcon::Off);
        playButton->setIcon(icon3);
        playButton->setIconSize(QSize(32, 32));

        horizontalLayout_2->addWidget(playButton);

        playAllButton = new QToolButton(frame);
        playAllButton->setObjectName("playAllButton");
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/play_all.png"), QSize(), QIcon::Normal, QIcon::Off);
        playAllButton->setIcon(icon4);
        playAllButton->setIconSize(QSize(32, 32));
        playAllButton->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);

        horizontalLayout_2->addWidget(playAllButton);

        pauseButton = new QToolButton(frame);
        pauseButton->setObjectName("pauseButton");
        pauseButton->setEnabled(false);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/pause.png"), QSize(), QIcon::Normal, QIcon::Off);
        pauseButton->setIcon(icon5);
        pauseButton->setIconSize(QSize(32, 32));

        horizontalLayout_2->addWidget(pauseButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout->addWidget(frame);


        gridLayout_2->addLayout(verticalLayout, 0, 0, 1, 1);

        frame_2 = new QFrame(centralwidget);
        frame_2->setObjectName("frame_2");
        frame_2->setFrameShape(QFrame::Shape::StyledPanel);
        frame_2->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_2 = new QVBoxLayout(frame_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        splitter_2 = new QSplitter(frame_2);
        splitter_2->setObjectName("splitter_2");
        splitter_2->setOrientation(Qt::Orientation::Horizontal);
        splitter = new QSplitter(splitter_2);
        splitter->setObjectName("splitter");
        splitter->setOrientation(Qt::Orientation::Vertical);
        parameterTable = new QTableWidget(splitter);
        if (parameterTable->columnCount() < 3)
            parameterTable->setColumnCount(3);
        if (parameterTable->rowCount() < 2)
            parameterTable->setRowCount(2);
        parameterTable->setObjectName("parameterTable");
        parameterTable->setRowCount(2);
        parameterTable->setColumnCount(3);
        splitter->addWidget(parameterTable);
        parameterTable->verticalHeader()->setVisible(false);
        stdoutBox = new QPlainTextEdit(splitter);
        stdoutBox->setObjectName("stdoutBox");
        QFont font;
        font.setFamilies({QString::fromUtf8("Monospace")});
        stdoutBox->setFont(font);
        stdoutBox->setReadOnly(true);
        splitter->addWidget(stdoutBox);
        splitter_2->addWidget(splitter);
        previewWidget = new QWidget(splitter_2);
        previewWidget->setObjectName("previewWidget");
        previewWidget->setBaseSize(QSize(0, 0));
        splitter_2->addWidget(previewWidget);

        verticalLayout_2->addWidget(splitter_2);


        gridLayout_2->addWidget(frame_2, 1, 0, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1200, 30));
        menubar->setNativeMenuBar(false);
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuSimulation = new QMenu(menubar);
        menuSimulation->setObjectName("menuSimulation");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuSimulation->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSave_2);
        menuFile->addAction(actionOpen);
        menuSimulation->addAction(actionAdvance_One_Step);
        menuSimulation->addAction(actionAdvance_Continuously);
        menuSimulation->addAction(actionPause);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "soot-dem-gui by Egor Demidov", nullptr));
        actionSave->setText(QCoreApplication::translate("MainWindow", "&Save", nullptr));
        actionSave_2->setText(QCoreApplication::translate("MainWindow", "Sa&ve As", nullptr));
        actionOpen->setText(QCoreApplication::translate("MainWindow", "&Open", nullptr));
        actionAdvance_One_Step->setText(QCoreApplication::translate("MainWindow", "&Advance One Step", nullptr));
        actionAdvance_Continuously->setText(QCoreApplication::translate("MainWindow", "Advance &Continuously", nullptr));
        actionPause->setText(QCoreApplication::translate("MainWindow", "&Pause", nullptr));
        actionNew->setText(QCoreApplication::translate("MainWindow", "New", nullptr));
#if QT_CONFIG(tooltip)
        newButton->setToolTip(QCoreApplication::translate("MainWindow", "Initialize a new simulation", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        newButton->setStatusTip(QCoreApplication::translate("MainWindow", "New Simulation", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(whatsthis)
        newButton->setWhatsThis(QString());
#endif // QT_CONFIG(whatsthis)
        newButton->setText(QString());
#if QT_CONFIG(shortcut)
        newButton->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        openButton->setToolTip(QCoreApplication::translate("MainWindow", "Open an existing simulation", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        openButton->setStatusTip(QCoreApplication::translate("MainWindow", "Open Simulation", nullptr));
#endif // QT_CONFIG(statustip)
        openButton->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(shortcut)
        openButton->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        saveButton->setToolTip(QCoreApplication::translate("MainWindow", "Save the simulation configuration", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        saveButton->setStatusTip(QCoreApplication::translate("MainWindow", "Save Simulation", nullptr));
#endif // QT_CONFIG(statustip)
        saveButton->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(shortcut)
        saveButton->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        playButton->setToolTip(QCoreApplication::translate("MainWindow", "Advance simulation by one step", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        playButton->setStatusTip(QCoreApplication::translate("MainWindow", "Advance Simulation One Step", nullptr));
#endif // QT_CONFIG(statustip)
        playButton->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(shortcut)
        playButton->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+R", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        playAllButton->setToolTip(QCoreApplication::translate("MainWindow", "Advance simulation continuously", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        playAllButton->setStatusTip(QCoreApplication::translate("MainWindow", "Advance SImulation Continuously", nullptr));
#endif // QT_CONFIG(statustip)
        playAllButton->setText(QString());
#if QT_CONFIG(shortcut)
        playAllButton->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+R", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        pauseButton->setToolTip(QCoreApplication::translate("MainWindow", "Pause continuous simulation", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        pauseButton->setStatusTip(QCoreApplication::translate("MainWindow", "Pause Continuous Simulation", nullptr));
#endif // QT_CONFIG(statustip)
        pauseButton->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(shortcut)
        pauseButton->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+P", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        stdoutBox->setToolTip(QCoreApplication::translate("MainWindow", "DEM engine standard output", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        stdoutBox->setStatusTip(QCoreApplication::translate("MainWindow", "DEM engine standard output", nullptr));
#endif // QT_CONFIG(statustip)
        stdoutBox->setPlainText(QString());
        stdoutBox->setPlaceholderText(QString());
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "Fi&le", nullptr));
        menuSimulation->setTitle(QCoreApplication::translate("MainWindow", "Sim&ulation", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
