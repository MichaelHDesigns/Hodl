// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "hodlcashgui.h"

#include "hodlcashunits.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "networkstyle.h"
#include "notificator.h"
#include "openuridialog.h"
#include "optionsdialog.h"
#include "optionsmodel.h"
#include "rpcconsole.h"
#include "scicon.h"
#include "utilitydialog.h"
#include "miner.h"

#ifdef ENABLE_WALLET
#include "walletframe.h"
#include "walletmodel.h"
#endif // ENABLE_WALLET

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include "init.h"
#include "ui_interface.h"
#include "util.h"

#include <iostream>

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QProgressDialog>
#include <QSettings>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

#if QT_VERSION < 0x050000
#include <QTextDocument>
#include <QUrl>
#else
#include <QUrlQuery>
#endif

#include <boost/thread.hpp>

const QString HodlCashGUI::DEFAULT_WALLET = "~Default";

HodlCashGUI::HodlCashGUI(const NetworkStyle *networkStyle, QWidget *parent) :
    QMainWindow(parent),
    clientModel(0),
    walletFrame(0),
    unitDisplayControl(0),
    labelEncryptionIcon(0),
    labelConnectionsIcon(0),
    labelBlocksIcon(0),
    progressBarLabel(0),
    progressBar(0),
    progressDialog(0),
    appMenuBar(0),
    overviewAction(0),
    historyAction(0),
    quitAction(0),
    sendCoinsAction(0),
    depositCoinsAction(0),
    sendCoinsMenuAction(0),
    usedSendingAddressesAction(0),
    usedReceivingAddressesAction(0),
    signMessageAction(0),
    verifyMessageAction(0),
    aboutAction(0),
    receiveCoinsAction(0),
    receiveCoinsMenuAction(0),
    optionsAction(0),
    toggleHideAction(0),
    encryptWalletAction(0),
    backupWalletAction(0),
    changePassphraseAction(0),
    aboutQtAction(0),
    openRPCConsoleAction(0),
    openAction(0),
    showHelpMessageAction(0),
    trayIcon(0),
    trayIconMenu(0),
    notificator(0),
    rpcConsole(0),
    prevBlocks(0),
    spinnerFrame(0),
    miningOffAction(0),
    miningOnAction(0)
{
    GUIUtil::restoreWindowGeometry("nWindow", QSize(1200, 700), this);
//	qApp->setStyleSheet("QMainWindow { background-image:url(:icons/hodl);border:none; background-repeat: no-repeat;font-family:'Open Sans,sans-serif'; }");

    QString windowTitle = tr("HodlCash Core") + " - ";
#ifdef ENABLE_WALLET
    /* if compiled with wallet support, -disablewallet can still disable the wallet */
    enableWallet = !GetBoolArg("-disablewallet", false);
#else
    enableWallet = false;
#endif // ENABLE_WALLET
    if(enableWallet)
    {
        windowTitle += tr("Wallet");
    } else {
        windowTitle += tr("Node");
    }
    windowTitle += " " + networkStyle->getTitleAddText();
#ifndef Q_OS_MAC
    QApplication::setWindowIcon(networkStyle->getTrayAndWindowIcon());
    setWindowIcon(networkStyle->getTrayAndWindowIcon());
#else
    MacDockIconHandler::instance()->setIcon(networkStyle->getAppIcon());
#endif
    setWindowTitle(windowTitle);

#if defined(Q_OS_MAC) && QT_VERSION < 0x050000
    // This property is not implemented in Qt 5. Setting it has no effect.
    // A replacement API (QtMacUnifiedToolBar) is available in QtMacExtras.
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    rpcConsole = new RPCConsole(0);
#ifdef ENABLE_WALLET
    if(enableWallet)
    {
        /** Create wallet frame and make it the central widget */
        walletFrame = new WalletFrame(this);
        setCentralWidget(walletFrame);
    } else
#endif // ENABLE_WALLET
    {
        /* When compiled without wallet or -disablewallet is provided,
         * the central widget is the rpc console.
         */
        setCentralWidget(rpcConsole);
    }

    // Accept D&D of URIs
    setAcceptDrops(true);

    // Create actions for the toolbar, menu bar and tray/dock icon
    // Needs walletFrame to be initialized
    createActions();

    // Create application menu bar
    createMenuBar();

    // Create the toolbars
    createToolBars();

    // Create system tray icon and notification
    createTrayIcon(networkStyle);

    // Create status bar
    statusBar();

    // Disable size grip because it looks ugly and nobody needs it
    statusBar()->setSizeGripEnabled(false);

    // Status bar notification icons
    QFrame *frameBlocks = new QFrame();
    frameBlocks->setContentsMargins(0,0,0,0);
    frameBlocks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QHBoxLayout *frameBlocksLayout = new QHBoxLayout(frameBlocks);
    frameBlocksLayout->setContentsMargins(3,0,3,0);
    frameBlocksLayout->setSpacing(3);
    unitDisplayControl = new UnitDisplayStatusBarControl();
    labelEncryptionIcon = new QLabel();
    labelMiningIcon = new QLabel();

    labelConnectionsIcon = new QLabel();
    labelBlocksIcon = new QLabel();
    if(enableWallet)
    {
        frameBlocksLayout->addStretch();
        frameBlocksLayout->addWidget(unitDisplayControl);
        frameBlocksLayout->addStretch();
        frameBlocksLayout->addWidget(labelEncryptionIcon);
    }
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelMiningIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelConnectionsIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelBlocksIcon);
    frameBlocksLayout->addStretch();
    
    // Set mining pixmap
    labelMiningIcon->setPixmap(QIcon(":/icons/transaction_conflicted").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
#ifdef ENABLE_WALLET
    if(enableWallet)
    {
        QTimer *timerMiningIcon = new QTimer(labelMiningIcon);
        timerMiningIcon->start(MODEL_UPDATE_DELAY);
        connect(timerMiningIcon, SIGNAL(timeout()), this, SLOT(updateMiningIcon()));
        // Set initial values for mining icon
        fGenerate = false;
    } else
#endif // ENABLE_WALLET
    {
        labelMiningIcon->setVisible(false);
    }

    // Progress bar and label for blocks download
    progressBarLabel = new QLabel();
    progressBarLabel->setVisible(false);
    progressBar = new GUIUtil::ProgressBar();
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setVisible(false);

    // Override style sheet for progress bar for styles that have a segmented progress bar,
    // as they make the text unreadable (workaround for issue #1071)
    // See https://qt-project.org/doc/qt-4.8/gallery.html
    QString curStyle = QApplication::style()->metaObject()->className();
    if(curStyle == "QWindowsStyle" || curStyle == "QWindowsXPStyle")
    {
        progressBar->setStyleSheet("QProgressBar { background-color: #e8e8e8; border: 1px solid grey; border-radius: 7px; padding: 1px; text-align: center; } QProgressBar::chunk { background: QLinearGradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #001D7C, stop: 1 dark blue); border-radius: 7px; margin: 0px; }");
    }

    statusBar()->addWidget(progressBarLabel);
    statusBar()->addWidget(progressBar);
    statusBar()->addPermanentWidget(frameBlocks);

    // Install event filter to be able to catch status tip events (QEvent::StatusTip)
    this->installEventFilter(this);

    // Initially wallet actions should be disabled
    setWalletActionsEnabled(false);

    // Subscribe to notifications from core
    subscribeToCoreSignals();
}

HodlCashGUI::~HodlCashGUI()
{
    // Unsubscribe from notifications from core
    unsubscribeFromCoreSignals();

    GUIUtil::saveWindowGeometry("nWindow", this);
    if(trayIcon) // Hide tray icon, as deleting will let it linger until quit (on Ubuntu)
        trayIcon->hide();
#ifdef Q_OS_MAC
    delete appMenuBar;
    MacDockIconHandler::cleanup();
#endif

    delete rpcConsole;
}

void HodlCashGUI::createActions()
{
    QActionGroup *tabGroup = new QActionGroup(this);

    overviewAction = new QAction(QIcon(":/icons/overview"), tr("Overview"), this);
    overviewAction->setStatusTip(tr("Show general overview of wallet"));
    overviewAction->setToolTip(overviewAction->statusTip());
    overviewAction->setCheckable(true);
    overviewAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
    tabGroup->addAction(overviewAction);

    sendCoinsAction = new QAction(QIcon(":/icons/send"), tr("Send"), this);
    sendCoinsAction->setStatusTip(tr("Send coins to a HodlCash address"));
    sendCoinsAction->setToolTip(sendCoinsAction->statusTip());
    sendCoinsAction->setCheckable(true);
    sendCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
    tabGroup->addAction(sendCoinsAction);

    sendCoinsMenuAction = new QAction(TextColorIcon(":/icons/send"), sendCoinsAction->text(), this);
    sendCoinsMenuAction->setStatusTip(sendCoinsAction->statusTip());
    sendCoinsMenuAction->setToolTip(sendCoinsMenuAction->statusTip());

    depositCoinsAction = new QAction(QIcon(":/icons/deposit"), tr("Deposit"), this);
    depositCoinsAction->setStatusTip(tr("Make a term deposit"));
    depositCoinsAction->setToolTip(depositCoinsAction->statusTip());
    depositCoinsAction->setCheckable(true);
    depositCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
    tabGroup->addAction(depositCoinsAction);

    receiveCoinsAction = new QAction(QIcon(":/icons/receiving_addresses"), tr("Receive"), this);
    receiveCoinsAction->setStatusTip(tr("Request payments (generates QR codes and HodlCash: URIs)"));
    receiveCoinsAction->setToolTip(receiveCoinsAction->statusTip());
    receiveCoinsAction->setCheckable(true);
    receiveCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
    tabGroup->addAction(receiveCoinsAction);

    receiveCoinsMenuAction = new QAction(TextColorIcon(":/icons/receiving_addresses"), receiveCoinsAction->text(), this);
    receiveCoinsMenuAction->setStatusTip(receiveCoinsAction->statusTip());
    receiveCoinsMenuAction->setToolTip(receiveCoinsMenuAction->statusTip());

    historyAction = new QAction(QIcon(":/icons/history"), tr("Transactions"), this);
    historyAction->setStatusTip(tr("Browse transaction history"));
    historyAction->setToolTip(historyAction->statusTip());
    historyAction->setCheckable(true);
    historyAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_4));
    tabGroup->addAction(historyAction);

/** Add new nav menu */
#ifdef ENABLE_WALLET
    if(enableWallet)
    {
    websiteAction = new QAction(QIcon(":/icons/website"), tr("Website"), this);
    websiteAction->setToolTip(tr("HODL Website"));
    blockexplorer1Action = new QAction(QIcon(":/icons/blockexplorer"), tr("Blockexplorer 1"), this);
    blockexplorer1Action->setToolTip(tr("HODL Blockexplorer 1"));
    blockexplorer2Action = new QAction(QIcon(":/icons/blockexplorer"), tr("Blockexplorer 2"), this);
    blockexplorer2Action->setToolTip(tr("HODL Blockexplorer 2"));
    paperwalletAction = new QAction(QIcon(":/icons/paperwallet"), tr("Paper Wallet"), this);
    paperwalletAction->setToolTip(tr("HODL Paper Wallet"));
    githubsourceAction = new QAction(QIcon(":/icons/github"), tr("Github Source"), this);
    githubsourceAction->setToolTip(tr("HODL Github Source"));
    whitepaperAction = new QAction(QIcon(":/icons/whitepaper"), tr("White Paper"), this);
    whitepaperAction->setToolTip(tr("HODL White Paper"));
    roadmapAction = new QAction(QIcon(":/icons/roadmap"), tr("Road Map"), this);
    roadmapAction->setToolTip(tr("HODL Road Map"));
    facebookAction = new QAction(QIcon(":/icons/facebook"), tr("Facebook"), this);
    facebookAction->setToolTip(tr("HODL Facebook Page"));
    facebookgroupAction = new QAction(QIcon(":/icons/facebook"), tr("Facebook Group"), this);
    facebookgroupAction->setToolTip(tr("HODL Facebook Group"));
    twitterAction = new QAction(QIcon(":/icons/twitter"), tr("Twitter"), this);
    twitterAction->setToolTip(tr("HODL Twitter"));
    googleplusAction = new QAction(QIcon(":/icons/googleplus"), tr("Google Plus"), this);
    googleplusAction->setToolTip(tr("HODL Google Plus"));
    youtubeAction = new QAction(QIcon(":/icons/youtube"), tr("YouTube"), this);
    youtubeAction->setToolTip(tr("HODL YouTube"));
    redditAction = new QAction(QIcon(":/icons/reddit"), tr("Reddit"), this);
    redditAction->setToolTip(tr("HODL Reddit"));
    discordAction = new QAction(QIcon(":/icons/discord"), tr("Discord"), this);
    discordAction->setToolTip(tr("HODL Discord"));
    telegramgroupAction = new QAction(QIcon(":/icons/telegram"), tr("Telegram Group"), this);
    telegramgroupAction->setToolTip(tr("HODL Telegram Group"));
    telegramgannouncementsAction = new QAction(QIcon(":/icons/telegram"), tr("Telegram Announcements"), this);
    telegramgannouncementsAction->setToolTip(tr("HODL Telegram Announcements"));
    }
#endif // ENABLE_WALLET
	
#ifdef ENABLE_WALLET
    if(enableWallet)
    {
        miningOffAction = new QAction(QIcon(":/icons/transaction_conflicted"), tr("Solo Mining Off"), this);
        miningOffAction->setStatusTip(tr("Stop Mining. May take some time to wind down."));
        miningOnAction = new QAction(QIcon(":/icons/tx_mined"), tr("Solo Mining On (1GB Required)"), this);
        miningOnAction->setStatusTip(tr("Mine solo."));
    }

    // These showNormalIfMinimized are needed because Send Coins and Receive Coins
    // can be triggered from the tray menu, and need to show the GUI to be useful.
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(gotoOverviewPage()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(depositCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(depositCoinsAction, SIGNAL(triggered()), this, SLOT(gotoDepositCoinsPage()));
    connect(sendCoinsMenuAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsMenuAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(receiveCoinsMenuAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsMenuAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(gotoHistoryPage()));
    connect(miningOffAction, SIGNAL(triggered()), this, SLOT(miningOff()));
    connect(miningOnAction, SIGNAL(triggered()), this, SLOT(miningOn()));
#endif // ENABLE_WALLET

    quitAction = new QAction(TextColorIcon(":/icons/quit"), tr("E&xit"), this);
    quitAction->setStatusTip(tr("Quit application"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    quitAction->setMenuRole(QAction::QuitRole);
    aboutAction = new QAction(TextColorIcon(":/icons/about"), tr("About HodlCash Core"), this);
    aboutAction->setStatusTip(tr("Show information about HodlCash Core"));
    aboutAction->setMenuRole(QAction::AboutRole);
    aboutQtAction = new QAction(TextColorIcon(":/icons/about_qt"), tr("About Qt"), this);
    aboutQtAction->setStatusTip(tr("Show information about Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    optionsAction = new QAction(TextColorIcon(":/icons/options"), tr("Options..."), this);
    optionsAction->setStatusTip(tr("Modify configuration options for HodlCash Core"));
    optionsAction->setMenuRole(QAction::PreferencesRole);
    toggleHideAction = new QAction(TextColorIcon(":/icons/about"), tr("Show / Hide"), this);
    toggleHideAction->setStatusTip(tr("Show or hide the main Window"));

    encryptWalletAction = new QAction(TextColorIcon(":/icons/lock_closed"), tr("Encrypt Wallet..."), this);
    encryptWalletAction->setStatusTip(tr("Encrypt the private keys that belong to your wallet"));
    encryptWalletAction->setCheckable(true);
    backupWalletAction = new QAction(TextColorIcon(":/icons/filesave"), tr("Backup Wallet..."), this);
    backupWalletAction->setStatusTip(tr("Backup wallet to another location"));
    changePassphraseAction = new QAction(TextColorIcon(":/icons/key"), tr("Change Passphrase..."), this);
    changePassphraseAction->setStatusTip(tr("Change the passphrase used for wallet encryption"));
    signMessageAction = new QAction(TextColorIcon(":/icons/edit"), tr("Sign &message..."), this);
    signMessageAction->setStatusTip(tr("Sign messages with your HodlCash addresses to prove you own them"));
    verifyMessageAction = new QAction(TextColorIcon(":/icons/verify"), tr("Verify message..."), this);
    verifyMessageAction->setStatusTip(tr("Verify messages to ensure they were signed with specified HodlCash addresses"));

    openRPCConsoleAction = new QAction(TextColorIcon(":/icons/debugwindow"), tr("Debug window"), this);
    openRPCConsoleAction->setStatusTip(tr("Open debugging and diagnostic console"));

    usedSendingAddressesAction = new QAction(TextColorIcon(":/icons/address-book"), tr("Sending addresses..."), this);
    usedSendingAddressesAction->setStatusTip(tr("Show the list of used sending addresses and labels"));
    usedReceivingAddressesAction = new QAction(TextColorIcon(":/icons/address-book"), tr("Receiving addresses..."), this);
    usedReceivingAddressesAction->setStatusTip(tr("Show the list of used receiving addresses and labels"));

    openAction = new QAction(TextColorIcon(":/icons/open"), tr("Open URI..."), this);
    openAction->setStatusTip(tr("Open a HodlCash: URI or payment request"));

    showHelpMessageAction = new QAction(TextColorIcon(":/icons/info"), tr("Command-line options"), this);
    showHelpMessageAction->setMenuRole(QAction::NoRole);
    showHelpMessageAction->setStatusTip(tr("Show the HodlCash Core help message to get a list with possible HodlCash command-line options"));

    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(optionsAction, SIGNAL(triggered()), this, SLOT(optionsClicked()));
    connect(toggleHideAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    connect(showHelpMessageAction, SIGNAL(triggered()), this, SLOT(showHelpMessageClicked()));
    connect(openRPCConsoleAction, SIGNAL(triggered()), this, SLOT(showDebugWindow()));
    // prevents an open debug window from becoming stuck/unusable on client shutdown
    connect(quitAction, SIGNAL(triggered()), rpcConsole, SLOT(hide()));

#ifdef ENABLE_WALLET
    if(walletFrame)
    {
        connect(encryptWalletAction, SIGNAL(triggered(bool)), walletFrame, SLOT(encryptWallet(bool)));
        connect(backupWalletAction, SIGNAL(triggered()), walletFrame, SLOT(backupWallet()));
        connect(changePassphraseAction, SIGNAL(triggered()), walletFrame, SLOT(changePassphrase()));
        connect(signMessageAction, SIGNAL(triggered()), this, SLOT(gotoSignMessageTab()));
        connect(verifyMessageAction, SIGNAL(triggered()), this, SLOT(gotoVerifyMessageTab()));
        connect(usedSendingAddressesAction, SIGNAL(triggered()), walletFrame, SLOT(usedSendingAddresses()));
        connect(usedReceivingAddressesAction, SIGNAL(triggered()), walletFrame, SLOT(usedReceivingAddresses()));
        connect(openAction, SIGNAL(triggered()), this, SLOT(openClicked()));
    }
#endif // ENABLE_WALLET
	
/** Add new nav menu */
#ifdef ENABLE_WALLET
    if(walletFrame)
    {
	connect(websiteAction, SIGNAL(triggered()), this, SLOT(websiteActionClicked()));
	connect(blockexplorer1Action, SIGNAL(triggered()), this, SLOT(blockexplorer1ActionClicked()));
	connect(blockexplorer2Action, SIGNAL(triggered()), this, SLOT(blockexplorer2ActionClicked()));
	connect(paperwalletAction, SIGNAL(triggered()), this, SLOT(paperwalletActionClicked()));
	connect(githubsourceAction, SIGNAL(triggered()), this, SLOT(githubsourceActionClicked()));
	connect(whitepaperAction, SIGNAL(triggered()), this, SLOT(whitepaperActionClicked()));
	connect(roadmapAction, SIGNAL(triggered()), this, SLOT(roadmapActionClicked()));
	connect(facebookAction, SIGNAL(triggered()), this, SLOT(facebookActionClicked()));
	connect(facebookgroupAction, SIGNAL(triggered()), this, SLOT(facebookgroupActionClicked()));
	connect(twitterAction, SIGNAL(triggered()), this, SLOT(twitterActionClicked()));
	connect(googleplusAction, SIGNAL(triggered()), this, SLOT(googleplusActionClicked()));
	connect(youtubeAction, SIGNAL(triggered()), this, SLOT(youtubeActionClicked()));
	connect(redditAction, SIGNAL(triggered()), this, SLOT(redditActionClicked()));
	connect(discordAction, SIGNAL(triggered()), this, SLOT(discordActionClicked()));
	connect(telegramgroupAction, SIGNAL(triggered()), this, SLOT(telegramgroupActionClicked()));
	connect(telegramgannouncementsAction, SIGNAL(triggered()), this, SLOT(telegramgannouncementsActionClicked()));
}
#endif // ENABLE_WALLET    
}

void HodlCashGUI::createMenuBar()
{
#ifdef Q_OS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
#else
    // Get the main window's menu bar on other platforms
    appMenuBar = menuBar();
#endif

    // Configure the menus
    QMenu *file = appMenuBar->addMenu(tr("File"));
    if(walletFrame)
    {
        file->addAction(openAction);
        file->addAction(backupWalletAction);
        file->addAction(signMessageAction);
        file->addAction(verifyMessageAction);
        file->addSeparator();
        file->addAction(usedSendingAddressesAction);
        file->addAction(usedReceivingAddressesAction);
        file->addSeparator();
    }
    file->addAction(quitAction);

    QMenu *settings = appMenuBar->addMenu(tr("Settings"));
    if(walletFrame)
    {
        settings->addAction(encryptWalletAction);
        settings->addAction(changePassphraseAction);
        settings->addSeparator();
    }
    settings->addAction(optionsAction);

    QMenu *help = appMenuBar->addMenu(tr("Help"));
    if(walletFrame)
    {
        help->addAction(openRPCConsoleAction);
    }
    help->addAction(showHelpMessageAction);
    help->addSeparator();
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);

#ifdef ENABLE_WALLET
    if(walletFrame)
    {
        QMenu *mining = appMenuBar->addMenu(tr("Mining"));
        mining->addSeparator();
        mining->addAction(miningOnAction);
        mining->addAction(miningOffAction);
    }
#endif // ENABLE_WALLET
	
/** Add new nav menu */
#ifdef ENABLE_WALLET
    if(walletFrame)
    {
	QMenu *links = appMenuBar->addMenu(tr("Links"));
	links->addAction(websiteAction);
	links->addAction(blockexplorer1Action);
	links->addAction(blockexplorer2Action);
	links->addAction(paperwalletAction);
	links->addAction(githubsourceAction);
	links->addAction(whitepaperAction);
	links->addAction(roadmapAction);
	    
	QMenu *social = appMenuBar->addMenu(tr("Social"));
	social->addAction(facebookAction);
	social->addAction(facebookgroupAction);
	social->addAction(twitterAction);
	social->addAction(googleplusAction);
	social->addAction(youtubeAction);
	social->addAction(redditAction);
	social->addAction(discordAction);
	social->addAction(telegramgroupAction);
	social->addAction(telegramgannouncementsAction);
    }
#endif // ENABLE_WALLET

}

void HodlCashGUI::createToolBars()
{
    if(walletFrame)
    {
        QToolBar *toolbar = addToolBar(tr("Tabs toolbar"));
        toolbar->setMovable(false);
        toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolbar->addAction(overviewAction);
        toolbar->addAction(sendCoinsAction);
        toolbar->addAction(depositCoinsAction);
        toolbar->addAction(receiveCoinsAction);
        toolbar->addAction(historyAction);
        overviewAction->setChecked(true);
    }
}

void HodlCashGUI::setClientModel(ClientModel *clientModel)
{
    this->clientModel = clientModel;
    if(clientModel)
    {
        // Create system tray menu (or setup the dock menu) that late to prevent users from calling actions,
        // while the client has not yet fully loaded
        createTrayIconMenu();

        // Keep up to date with client
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        setNumBlocks(clientModel->getNumBlocks(), clientModel->getLastBlockDate());
        connect(clientModel, SIGNAL(numBlocksChanged(int,QDateTime)), this, SLOT(setNumBlocks(int,QDateTime)));

        //setMining(0, 0);
        //connect(clientModel, SIGNAL(miningChanged(double,int)), this, SLOT(setMining(double,int)));

        // Receive and report messages from client model
        connect(clientModel, SIGNAL(message(QString,QString,unsigned int)), this, SLOT(message(QString,QString,unsigned int)));

        // Show progress dialog
        connect(clientModel, SIGNAL(showProgress(QString,int)), this, SLOT(showProgress(QString,int)));

        rpcConsole->setClientModel(clientModel);
#ifdef ENABLE_WALLET
        if(walletFrame)
        {
            walletFrame->setClientModel(clientModel);
        }
#endif // ENABLE_WALLET
        unitDisplayControl->setOptionsModel(clientModel->getOptionsModel());
    } else {
        // Disable possibility to show main window via action
        toggleHideAction->setEnabled(false);
        if(trayIconMenu)
        {
            // Disable context menu on tray icon
            trayIconMenu->clear();
        }
    }
}

#ifdef ENABLE_WALLET
bool HodlCashGUI::addWallet(const QString& name, WalletModel *walletModel)
{
    if(!walletFrame)
        return false;
    setWalletActionsEnabled(true);
    return walletFrame->addWallet(name, walletModel);
}

bool HodlCashGUI::setCurrentWallet(const QString& name)
{
    if(!walletFrame)
        return false;
    return walletFrame->setCurrentWallet(name);
}

void HodlCashGUI::removeAllWallets()
{
    if(!walletFrame)
        return;
    setWalletActionsEnabled(false);
    walletFrame->removeAllWallets();
}
#endif // ENABLE_WALLET

void HodlCashGUI::setWalletActionsEnabled(bool enabled)
{
    overviewAction->setEnabled(enabled);
    sendCoinsAction->setEnabled(enabled);
    depositCoinsAction->setEnabled(enabled);
    sendCoinsMenuAction->setEnabled(enabled);
    receiveCoinsAction->setEnabled(enabled);
    receiveCoinsMenuAction->setEnabled(enabled);
    historyAction->setEnabled(enabled);
    encryptWalletAction->setEnabled(enabled);
    backupWalletAction->setEnabled(enabled);
    changePassphraseAction->setEnabled(enabled);
    signMessageAction->setEnabled(enabled);
    verifyMessageAction->setEnabled(enabled);
    usedSendingAddressesAction->setEnabled(enabled);
    usedReceivingAddressesAction->setEnabled(enabled);
    openAction->setEnabled(enabled);
}

void HodlCashGUI::createTrayIcon(const NetworkStyle *networkStyle)
{
#ifndef Q_OS_MAC
    trayIcon = new QSystemTrayIcon(this);
    QString toolTip = tr("HodlCash Core client") + " " + networkStyle->getTitleAddText();
    trayIcon->setToolTip(toolTip);
    trayIcon->setIcon(networkStyle->getTrayAndWindowIcon());
    trayIcon->show();
#endif

    notificator = new Notificator(QApplication::applicationName(), trayIcon, this);
}

void HodlCashGUI::createTrayIconMenu()
{
#ifndef Q_OS_MAC
    // return if trayIcon is unset (only on non-Mac OSes)
    if (!trayIcon)
        return;

    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
#else
    // Note: On Mac, the dock icon is used to provide the tray's functionality.
    MacDockIconHandler *dockIconHandler = MacDockIconHandler::instance();
    dockIconHandler->setMainWindow((QMainWindow *)this);
    trayIconMenu = dockIconHandler->dockMenu();
#endif

    // Configuration of the tray icon (or dock icon) icon menu
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(sendCoinsMenuAction);
    trayIconMenu->addAction(receiveCoinsMenuAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(signMessageAction);
    trayIconMenu->addAction(verifyMessageAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(optionsAction);
    trayIconMenu->addAction(openRPCConsoleAction);
#ifndef Q_OS_MAC // This is built-in on Mac
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
#endif
}

#ifndef Q_OS_MAC
void HodlCashGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        // Click on system tray icon triggers show/hide of the main window
        toggleHidden();
    }
}
#endif

void HodlCashGUI::optionsClicked()
{
    if(!clientModel || !clientModel->getOptionsModel())
        return;

    OptionsDialog dlg(this, enableWallet);
    dlg.setModel(clientModel->getOptionsModel());
    dlg.exec();
}

void HodlCashGUI::aboutClicked()
{
    if(!clientModel)
        return;

    HelpMessageDialog dlg(this, true);
    dlg.exec();
}

/** New clickable menu links */
void HodlCashGUI::websiteActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://hodlcash.com/"));
}

void HodlCashGUI::blockexplorer1ActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://hodlcash-blockexplorer.hodlcash.com/"));
}

void HodlCashGUI::blockexplorer2ActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://blockexplorer.hodlcash.com/"));
}

void HodlCashGUI::paperwalletActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://hodl-paperwallet.hodlcash.com/"));
}

void HodlCashGUI::onlinewalletActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://online-wallet.hodlcash.com/"));
}

void HodlCashGUI::githubsourceActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://github.com/HodlCash/HodlCash"));
}

void HodlCashGUI::whitepaperActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://hodlcash.com/hodlcash-white-paper/"));
}

void HodlCashGUI::roadmapActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://hodlcash.com/hodlcash-road-map/"));
}

void HodlCashGUI::facebookActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://www.facebook.com/hodlcash"));
}

void HodlCashGUI::facebookgroupActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://www.facebook.com/groups/177521572870678/"));
}

void HodlCashGUI::twitterActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://twitter.com/HODL_Coin"));
}

void HodlCashGUI::googleplusActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://plus.google.com/b/109809615316952270396/109809615316952270396"));
}

void HodlCashGUI::youtubeActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://www.youtube.com/channel/UCclukUCxqxHk-nrUSj3MM5g"));
}

void HodlCashGUI::redditActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://www.reddit.com/r/Roi_Coin/"));
}

void HodlCashGUI::discordActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://discord.gg/F66Cs6X"));
}

void HodlCashGUI::telegramgroupActionClicked()
{
        QDesktopServices::openUrl(QUrl("https://t.me/joinchat/I0JIzQ72Fz6EWWAMwQURhQ"));
}

void HodlCashGUI::telegramgannouncementsActionClicked()
{
        QDesktopServices::openUrl(QUrl("http://t.me/HODL_coin"));
}

void HodlCashGUI::showDebugWindow()
{
    rpcConsole->showNormal();
    rpcConsole->show();
    rpcConsole->raise();
    rpcConsole->activateWindow();
}

void HodlCashGUI::showHelpMessageClicked()
{
    HelpMessageDialog *help = new HelpMessageDialog(this, false);
    help->setAttribute(Qt::WA_DeleteOnClose);
    help->show();
}

#ifdef ENABLE_WALLET
void HodlCashGUI::openClicked()
{
    OpenURIDialog dlg(this);
    if(dlg.exec())
    {
        Q_EMIT receivedURI(dlg.getURI());
    }
}

void HodlCashGUI::gotoOverviewPage()
{
    overviewAction->setChecked(true);
    if (walletFrame) walletFrame->gotoOverviewPage();
}

void HodlCashGUI::gotoHistoryPage()
{
    historyAction->setChecked(true);
    if (walletFrame) walletFrame->gotoHistoryPage();
}

void HodlCashGUI::gotoReceiveCoinsPage()
{
    receiveCoinsAction->setChecked(true);
    if (walletFrame) walletFrame->gotoReceiveCoinsPage();
}

void HodlCashGUI::gotoSendCoinsPage(QString addr)
{
    sendCoinsAction->setChecked(true);
    if (walletFrame) walletFrame->gotoSendCoinsPage(addr);
}

void HodlCashGUI::gotoDepositCoinsPage(QString addr)
{
    depositCoinsAction->setChecked(true);
    if (walletFrame) walletFrame->gotoDepositCoinsPage(addr);
}


void HodlCashGUI::gotoSignMessageTab(QString addr)
{
    if (walletFrame) walletFrame->gotoSignMessageTab(addr);
}

void HodlCashGUI::gotoVerifyMessageTab(QString addr)
{
    if (walletFrame) walletFrame->gotoVerifyMessageTab(addr);
}
#endif // ENABLE_WALLET

void HodlCashGUI::setNumConnections(int count)
{
    QString icon;
    switch(count)
    {
    case 0: icon = ":/icons/connect_0"; break;
    case 1: case 2: case 3: icon = ":/icons/connect_1"; break;
    case 4: case 5: case 6: icon = ":/icons/connect_2"; break;
    case 7: case 8: case 9: icon = ":/icons/connect_3"; break;
    default: icon = ":/icons/connect_4"; break;
    }
    labelConnectionsIcon->setPixmap(QIcon(icon).pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
    labelConnectionsIcon->setToolTip(tr("%n active connection(s) to HodlCash network", "", count));
}

void HodlCashGUI::setNumBlocks(int count, const QDateTime& blockDate)
{
    if(!clientModel)
        return;

    // Prevent orphan statusbar messages (e.g. hover Quit in main menu, wait until chain-sync starts -> garbelled text)
    statusBar()->clearMessage();

    // Acquire current block source
    enum BlockSource blockSource = clientModel->getBlockSource();
    switch (blockSource) {
        case BLOCK_SOURCE_NETWORK:
            progressBarLabel->setText(tr("Synchronizing with network..."));
            break;
        case BLOCK_SOURCE_DISK:
            progressBarLabel->setText(tr("Importing blocks from disk..."));
            break;
        case BLOCK_SOURCE_REINDEX:
            progressBarLabel->setText(tr("Reindexing blocks on disk..."));
            break;
        case BLOCK_SOURCE_NONE:
            // Case: not Importing, not Reindexing and no network connection
            progressBarLabel->setText(tr("No block source available..."));
            break;
    }

    QString tooltip;

    QDateTime currentDate = QDateTime::currentDateTime();
    qint64 secs = blockDate.secsTo(currentDate);

    tooltip = tr("Processed %n block(s) of transaction history.", "", count);

    // Set icon state: spinning if catching up, tick otherwise
    if(secs < 90*60)
    {
        tooltip = tr("Up to date") + QString(".<br>") + tooltip;
        labelBlocksIcon->setPixmap(QIcon(":/icons/synced").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));

#ifdef ENABLE_WALLET
        if(walletFrame)
            walletFrame->showOutOfSyncWarning(false);
#endif // ENABLE_WALLET

        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);
    }
    else
    {
        // Represent time from last generated block in human readable text
        QString timeBehindText;
        const int HOUR_IN_SECONDS = 60*60;
        const int DAY_IN_SECONDS = 24*60*60;
        const int WEEK_IN_SECONDS = 7*24*60*60;
        const int YEAR_IN_SECONDS = 31556952; // Average length of year in Gregorian calendar
        if(secs < 2*DAY_IN_SECONDS)
        {
            timeBehindText = tr("%n hour(s)","",secs/HOUR_IN_SECONDS);
        }
        else if(secs < 2*WEEK_IN_SECONDS)
        {
            timeBehindText = tr("%n day(s)","",secs/DAY_IN_SECONDS);
        }
        else if(secs < YEAR_IN_SECONDS)
        {
            timeBehindText = tr("%n week(s)","",secs/WEEK_IN_SECONDS);
        }
        else
        {
            qint64 years = secs / YEAR_IN_SECONDS;
            qint64 remainder = secs % YEAR_IN_SECONDS;
            timeBehindText = tr("%1 and %2").arg(tr("%n year(s)", "", years)).arg(tr("%n week(s)","", remainder/WEEK_IN_SECONDS));
        }

        progressBarLabel->setVisible(true);
        progressBar->setFormat(tr("%1 behind").arg(timeBehindText));
        progressBar->setMaximum(1000000000);
        progressBar->setValue(clientModel->getVerificationProgress() * 1000000000.0 + 0.5);
        progressBar->setVisible(true);

        tooltip = tr("Catching up...") + QString("<br>") + tooltip;
        if(count != prevBlocks)
        {
            labelBlocksIcon->setPixmap(QIcon(QString(
                ":/movies/spinner-%1").arg(spinnerFrame, 3, 10, QChar('0')))
                .pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
            spinnerFrame = (spinnerFrame + 1) % SPINNER_FRAMES;
        }
        prevBlocks = count;

#ifdef ENABLE_WALLET
        if(walletFrame)
            walletFrame->showOutOfSyncWarning(true);
#endif // ENABLE_WALLET

        tooltip += QString("<br>");
        tooltip += tr("Last received block was generated %1 ago.").arg(timeBehindText);
        tooltip += QString("<br>");
        tooltip += tr("Transactions after this will not yet be visible.");
    }

    // Don't word-wrap this (fixed-width) tooltip
    tooltip = QString("<nobr>") + tooltip + QString("</nobr>");

    labelBlocksIcon->setToolTip(tooltip);
    progressBarLabel->setToolTip(tooltip);
    progressBar->setToolTip(tooltip);
}

#ifdef ENABLE_WALLET
void HodlCashGUI::setMining(bool mining, double *xhashrate, int miners, int threads)
{
    if (mining)
    {

    	double hashrate = 0.0;
    	if (xhashrate != NULL) {
    		hashrate = sumArray(xhashrate, miners);
    	}

        if (hashrate > 0)
        {
            labelMiningIcon->setPixmap(QIcon(":/icons/tx_mined").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
            labelMiningIcon->setToolTip(tr("Mining.<br>HPS (Total): %1h/s<br>Miners: %2<br>Threads: %3").arg(hashrate).arg(miners).arg(threads));
        }
        else
        {
            labelMiningIcon->setPixmap(QIcon(":/icons/tx_mined").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
            labelMiningIcon->setToolTip(tr("Mining.<br>HPS (Total): Calculating...<br>Miners: %1<br>Threads: %2").arg(miners).arg(threads));
        }
    }
    else
    {
        labelMiningIcon->setPixmap(QIcon(":/icons/transaction_conflicted").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelMiningIcon->setToolTip(tr("Not mining."));
    }
}
#endif // ENABLE_WALLET

void HodlCashGUI::message(const QString &title, const QString &message, unsigned int style, bool *ret)
{
    QString strTitle = tr("HodlCash"); // default title
    // Default to information icon
    int nMBoxIcon = QMessageBox::Information;
    int nNotifyIcon = Notificator::Information;

    QString msgType;

    // Prefer supplied title over style based title
    if (!title.isEmpty()) {
        msgType = title;
    }
    else {
        switch (style) {
        case CClientUIInterface::MSG_ERROR:
            msgType = tr("Error");
            break;
        case CClientUIInterface::MSG_WARNING:
            msgType = tr("Warning");
            break;
        case CClientUIInterface::MSG_INFORMATION:
            msgType = tr("Information");
            break;
        default:
            break;
        }
    }
    // Append title to "HodlCash - "
    if (!msgType.isEmpty())
        strTitle += " - " + msgType;

    // Check for error/warning icon
    if (style & CClientUIInterface::ICON_ERROR) {
        nMBoxIcon = QMessageBox::Critical;
        nNotifyIcon = Notificator::Critical;
    }
    else if (style & CClientUIInterface::ICON_WARNING) {
        nMBoxIcon = QMessageBox::Warning;
        nNotifyIcon = Notificator::Warning;
    }

    // Display message
    if (style & CClientUIInterface::MODAL) {
        // Check for buttons, use OK as default, if none was supplied
        QMessageBox::StandardButton buttons;
        if (!(buttons = (QMessageBox::StandardButton)(style & CClientUIInterface::BTN_MASK)))
            buttons = QMessageBox::Ok;

        showNormalIfMinimized();
        QMessageBox mBox((QMessageBox::Icon)nMBoxIcon, strTitle, message, buttons, this);
        int r = mBox.exec();
        if (ret != NULL)
            *ret = r == QMessageBox::Ok;
    }
    else
        notificator->notify((Notificator::Class)nNotifyIcon, strTitle, message);
}

void HodlCashGUI::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_OS_MAC // Ignored on Mac
    if(e->type() == QEvent::WindowStateChange)
    {
        if(clientModel && clientModel->getOptionsModel() && clientModel->getOptionsModel()->getMinimizeToTray())
        {
            QWindowStateChangeEvent *wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if(!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized())
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                e->ignore();
            }
        }
    }
#endif
}

void HodlCashGUI::closeEvent(QCloseEvent *event)
{
#ifndef Q_OS_MAC // Ignored on Mac
    if(clientModel && clientModel->getOptionsModel())
    {
        if(!clientModel->getOptionsModel()->getMinimizeToTray() &&
           !clientModel->getOptionsModel()->getMinimizeOnClose())
        {
            // close rpcConsole in case it was open to make some space for the shutdown window
            rpcConsole->close();

            QApplication::quit();
        }
    }
#endif
    QMainWindow::closeEvent(event);
}

#ifdef ENABLE_WALLET
void HodlCashGUI::incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address, const QString& label)
{
    // On new transaction, make an info balloon
    QString msg = tr("Date: %1\n").arg(date) +
                  tr("Amount: %1\n").arg(HodlCashUnits::formatWithUnit(unit, amount, true)) +
                  tr("Type: %1\n").arg(type);
    if (!label.isEmpty())
        msg += tr("Label: %1\n").arg(label);
    else if (!address.isEmpty())
        msg += tr("Address: %1\n").arg(address);
    message((amount)<0 ? tr("Sent transaction") : tr("Incoming transaction"),
             msg, CClientUIInterface::MSG_INFORMATION);
}

void HodlCashGUI::maturedCoinsNotification(int count, int unit, CAmount& amount)
{
	LogPrintf("got maturedCoinsNotification for %d/%s\n", count, HodlCashUnits::formatWithUnit(HodlCashUnits::HODL, amount, true).data() );

    // On matured coins, make an info balloon
    QString msg = tr("Count: %1\n").arg(count) +
                  tr("Amount: %1\n").arg(HodlCashUnits::formatWithUnit(unit, amount, true));
    message(tr("Matured coins"),
             msg, CClientUIInterface::MSG_INFORMATION);
}

#endif // ENABLE_WALLET

void HodlCashGUI::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept only URIs
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void HodlCashGUI::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        Q_FOREACH(const QUrl &uri, event->mimeData()->urls())
        {
            Q_EMIT receivedURI(uri.toString());
        }
    }
    event->acceptProposedAction();
}

bool HodlCashGUI::eventFilter(QObject *object, QEvent *event)
{
    // Catch status tip events
    if (event->type() == QEvent::StatusTip)
    {
        // Prevent adding text from setStatusTip(), if we currently use the status bar for displaying other stuff
        if (progressBarLabel->isVisible() || progressBar->isVisible())
            return true;
    }
    return QMainWindow::eventFilter(object, event);
}

#ifdef ENABLE_WALLET
bool HodlCashGUI::handlePaymentRequest(const SendCoinsRecipient& recipient)
{
    // URI has to be valid
    if (walletFrame && walletFrame->handlePaymentRequest(recipient))
    {
        showNormalIfMinimized();
        gotoSendCoinsPage();
        return true;
    }
    return false;
}

void HodlCashGUI::setEncryptionStatus(int status)
{
    switch(status)
    {
    case WalletModel::Unencrypted:
        labelEncryptionIcon->hide();
        encryptWalletAction->setChecked(false);
        changePassphraseAction->setEnabled(false);
        encryptWalletAction->setEnabled(true);
        break;
    case WalletModel::Unlocked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_open").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>unlocked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    case WalletModel::Locked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_closed").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>locked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    }
}
#endif // ENABLE_WALLET

void HodlCashGUI::showNormalIfMinimized(bool fToggleHidden)
{
    if(!clientModel)
        return;

    // activateWindow() (sometimes) helps with keyboard focus on Windows
    if (isHidden())
    {
        show();
        activateWindow();
    }
    else if (isMinimized())
    {
        showNormal();
        activateWindow();
    }
    else if (GUIUtil::isObscured(this))
    {
        raise();
        activateWindow();
    }
    else if(fToggleHidden)
        hide();
}

void HodlCashGUI::toggleHidden()
{
    showNormalIfMinimized(true);
}

#ifdef ENABLE_WALLET
void HodlCashGUI::updateMiningIcon()
{
    if (fGenerate)
    {
        int nInstances = GetArg("-minermemory",1);
        int nCores = GetArg("-genproclimit",-1);
        if (nCores < 0)
            nCores = boost::thread::hardware_concurrency();

        setMining(true, dHashesPerSec, nInstances, nCores);
    }
    else
        setMining(false, 0, 0, 0);
}

void HodlCashGUI::miningOff()
{
//mapArgs["-genproclimit"] = "0";
GenerateHodlCashs(false, pwalletMain,0);
setMining(false,0, 0, 0);
}

void HodlCashGUI::miningOn()
{
//mapArgs["-genproclimit"] = itostr(processes);
GenerateHodlCashs(true, pwalletMain,-1);
setMining(true,0, 1, -1);
}
#endif // ENABLE_WALLET

void HodlCashGUI::detectShutdown()
{
    if (ShutdownRequested())
    {
        if(rpcConsole)
            rpcConsole->hide();
        qApp->quit();
    }
}

void HodlCashGUI::showProgress(const QString &title, int nProgress)
{
    if (nProgress == 0)
    {
        progressDialog = new QProgressDialog(title, "", 0, 100);
        progressDialog->setWindowModality(Qt::ApplicationModal);
        progressDialog->setMinimumDuration(0);
        progressDialog->setCancelButton(0);
        progressDialog->setAutoClose(false);
        progressDialog->setValue(0);
    }
    else if (nProgress == 100)
    {
        if (progressDialog)
        {
            progressDialog->close();
            progressDialog->deleteLater();
        }
    }
    else if (progressDialog)
        progressDialog->setValue(nProgress);
}

static bool ThreadSafeMessageBox(HodlCashGUI *gui, const std::string& message, const std::string& caption, unsigned int style)
{
    bool modal = (style & CClientUIInterface::MODAL);
    // The SECURE flag has no effect in the Qt GUI.
    // bool secure = (style & CClientUIInterface::SECURE);
    style &= ~CClientUIInterface::SECURE;
    bool ret = false;
    // In case of modal message, use blocking connection to wait for user to click a button
    QMetaObject::invokeMethod(gui, "message",
                               modal ? GUIUtil::blockingGUIThreadConnection() : Qt::QueuedConnection,
                               Q_ARG(QString, QString::fromStdString(caption)),
                               Q_ARG(QString, QString::fromStdString(message)),
                               Q_ARG(unsigned int, style),
                               Q_ARG(bool*, &ret));
    return ret;
}

void HodlCashGUI::subscribeToCoreSignals()
{
    // Connect signals to client
    uiInterface.ThreadSafeMessageBox.connect(boost::bind(ThreadSafeMessageBox, this, _1, _2, _3));
}

void HodlCashGUI::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    uiInterface.ThreadSafeMessageBox.disconnect(boost::bind(ThreadSafeMessageBox, this, _1, _2, _3));
}

UnitDisplayStatusBarControl::UnitDisplayStatusBarControl() :
    optionsModel(0),
    menu(0)
{
    createContextMenu();
    setToolTip(tr("Unit to show amounts in. Click to select another unit."));
    QList<HodlCashUnits::Unit> units = HodlCashUnits::availableUnits();
    int max_width = 0;
    const QFontMetrics fm(font());
    Q_FOREACH (const HodlCashUnits::Unit unit, units)
    {
        max_width = qMax(max_width, fm.width(HodlCashUnits::name(unit)));
    }
    setMinimumSize(max_width, 0);
    setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setStyleSheet(QString("QLabel { color : %1 }").arg(SingleColor().name()));
}

/** So that it responds to button clicks */
void UnitDisplayStatusBarControl::mousePressEvent(QMouseEvent *event)
{
    onDisplayUnitsClicked(event->pos());
}

/** Creates context menu, its actions, and wires up all the relevant signals for mouse events. */
void UnitDisplayStatusBarControl::createContextMenu()
{
    menu = new QMenu();
    Q_FOREACH(HodlCashUnits::Unit u, HodlCashUnits::availableUnits())
    {
        QAction *menuAction = new QAction(QString(HodlCashUnits::name(u)), this);
        menuAction->setData(QVariant(u));
        menu->addAction(menuAction);
    }
    connect(menu,SIGNAL(triggered(QAction*)),this,SLOT(onMenuSelection(QAction*)));
}

/** Lets the control know about the Options Model (and its signals) */
void UnitDisplayStatusBarControl::setOptionsModel(OptionsModel *optionsModel)
{
    if (optionsModel)
    {
        this->optionsModel = optionsModel;

        // be aware of a display unit change reported by the OptionsModel object.
        connect(optionsModel,SIGNAL(displayUnitChanged(int)),this,SLOT(updateDisplayUnit(int)));

        // initialize the display units label with the current value in the model.
        updateDisplayUnit(optionsModel->getDisplayUnit());
    }
}

/** When Display Units are changed on OptionsModel it will refresh the display text of the control on the status bar */
void UnitDisplayStatusBarControl::updateDisplayUnit(int newUnits)
{
    setText(HodlCashUnits::name(newUnits));
}

/** Shows context menu with Display Unit options by the mouse coordinates */
void UnitDisplayStatusBarControl::onDisplayUnitsClicked(const QPoint& point)
{
    QPoint globalPos = mapToGlobal(point);
    menu->exec(globalPos);
}

/** Tells underlying optionsModel to update its current display unit. */
void UnitDisplayStatusBarControl::onMenuSelection(QAction* action)
{
    if (action)
    {
        optionsModel->setDisplayUnit(action->data());
    }
}
