#include "mainwindow.h"
#include "datetimepicker.h"
#include "task.h"

#include <QApplication>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QScrollArea>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QSettings>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <QFont>
#include <QFontDatabase>
#include <functional>

// ─── helpers ───────────────────────────────────────────────
static QString nowStr() {
    return QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
}

static QDateTime parseWaktu(const QString &s) {
    return QDateTime::fromString(s, "dd/MM/yyyy HH:mm");
}

static long selisihMenit(const QString &waktu) {
    QDateTime dl = parseWaktu(waktu);
    if (!dl.isValid()) return 999;
    return QDateTime::currentDateTime().secsTo(dl) / 60;
}

// ─── shadow helper ─────────────────────────────────────────
static void addShadow(QWidget *w, int blur = 18, QColor color = QColor(0,0,0,80)) {
    auto *eff = new QGraphicsDropShadowEffect(w);
    eff->setBlurRadius(blur);
    eff->setColor(color);
    eff->setOffset(0, 4);
    w->setGraphicsEffect(eff);
}

// ═══════════════════════════════════════════════════════════
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("TodoList");
    setMinimumSize(900, 620);
    resize(1020, 660);

    // Center window
    QRect sg = QGuiApplication::primaryScreen()->availableGeometry();
    move(sg.center() - rect().center());

    setStyleSheet(appStyleSheet());

    // ── Root layout ─────────────────────────────────────
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0,0,0,0);
    rootLayout->setSpacing(0);

    // ── Sidebar ─────────────────────────────────────────
    sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(200);
    QVBoxLayout *sbl = new QVBoxLayout(sidebar);
    sbl->setContentsMargins(16, 28, 16, 20);
    sbl->setSpacing(0);

    // Logo
    QLabel *logo = new QLabel("◈ TodoList");
    logo->setObjectName("logo");
    sbl->addWidget(logo);
    sbl->addSpacing(8);

    // Clock
    lblClock = new QLabel(nowStr());
    lblClock->setObjectName("clock");
    sbl->addWidget(lblClock);
    sbl->addSpacing(32);

    // Nav buttons
    btnCatat   = new QPushButton("＋  Catat Tugas");
    btnLihat   = new QPushButton("▤  Lihat Tugas");
    btnHistory = new QPushButton("⏱  History Telat");
    for (auto *b : {btnCatat, btnLihat, btnHistory}) {
        b->setObjectName("navBtn");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        sbl->addWidget(b);
        sbl->addSpacing(4);
    }

    sbl->addStretch();

    // Stats
    lblStats = new QLabel("0 pending · 0 selesai");
    lblStats->setObjectName("stats");
    lblStats->setWordWrap(true);
    sbl->addWidget(lblStats);

    rootLayout->addWidget(sidebar);

    // ── Pages ────────────────────────────────────────────
    pages = new QStackedWidget;
    pages->setObjectName("pages");
    pages->addWidget(buildPageCatat());
    pages->addWidget(buildPageLihat());
    pages->addWidget(buildPageHistory());
    rootLayout->addWidget(pages, 1);

    // ── Connect nav ──────────────────────────────────────
    connect(btnCatat,   &QPushButton::clicked, this, &MainWindow::showPageCatat);
    connect(btnLihat,   &QPushButton::clicked, this, &MainWindow::showPageLihat);
    connect(btnHistory, &QPushButton::clicked, this, &MainWindow::showPageHistory);

    // ── Tray icon ────────────────────────────────────────
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    trayIcon->show();

    // ── Timers ───────────────────────────────────────────
    clockTimer = new QTimer(this);
    connect(clockTimer, &QTimer::timeout, this, [this]{
        lblClock->setText(nowStr());
    });
    clockTimer->start(30000);

    notifTimer = new QTimer(this);
    connect(notifTimer, &QTimer::timeout, this, &MainWindow::onNotifTick);
    notifTimer->start(10000); // tick tiap 10 detik — cukup halus untuk notif 1 menit

    // ── Load data ────────────────────────────────────────
    loadData();
    showPageCatat();
}

MainWindow::~MainWindow() { saveData(); }

// ─── Style Sheet (dark minimalis) ──────────────────────────
QString MainWindow::appStyleSheet() {
    return R"(
    QMainWindow, QWidget { background: #0F0F13; color: #E8E8F0; font-family: 'Segoe UI', sans-serif; font-size: 13px; }

    /* Sidebar */
    #sidebar { background: #16161D; border-right: 1px solid #2A2A35; }
    #logo    { font-size: 17px; font-weight: 700; color: #E8E8F0; letter-spacing: 1px; padding: 4px 0; }
    #clock   { font-size: 11px; color: #55556A; font-family: 'Consolas', monospace; }
    #stats   { font-size: 11px; color: #55556A; line-height: 1.6; }

    #navBtn {
        background: transparent;
        color: #666680;
        border: none;
        text-align: left;
        padding: 11px 14px;
        border-radius: 8px;
        font-size: 13px;
        font-weight: 500;
    }
    #navBtn:hover   { background: #1E1E28; color: #C0C0D8; }
    #navBtn:checked { background: #1E1E2E; color: #E8E8F0; border-left: 3px solid #7C7CFF; padding-left: 11px; }

    /* Pages */
    #pages { background: #0F0F13; }
    #pageTitle { font-size: 20px; font-weight: 700; color: #E8E8F0; letter-spacing: 0.5px; }
    #pageSub   { font-size: 12px; color: #44445A; }

    /* Form */
    QLineEdit, QComboBox, QTextEdit {
        background: #16161D;
        border: 1px solid #2A2A35;
        border-radius: 8px;
        color: #D8D8EC;
        padding: 10px 14px;
        font-size: 13px;
        selection-background-color: #3A3A6A;
    }
    QLineEdit:focus, QComboBox:focus, QTextEdit:focus {
        border: 1px solid #5555CC;
        background: #18181F;
    }
    QLineEdit::placeholder { color: #40404A; }

    /* DateTimePicker buttons */
    #datePickerBtn, #timePickerBtn {
        background: #16161D;
        border: 1px solid #2A2A35;
        border-radius: 8px;
        color: #D8D8EC;
        padding: 0 14px;
        font-size: 13px;
        text-align: left;
    }
    #datePickerBtn:hover, #timePickerBtn:hover {
        border: 1px solid #5555CC;
        background: #18181F;
    }
    #datePickerBtn:pressed, #timePickerBtn:pressed { background: #141420; }

    QComboBox::drop-down { border: none; width: 28px; }
    QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid #666680; margin-right: 10px; }
    QComboBox QAbstractItemView { background: #1A1A24; border: 1px solid #2A2A35; color: #D0D0E8; selection-background-color: #2E2E50; }

    /* Primary button */
    #btnPrimary {
        background: #4444CC;
        color: #FFFFFF;
        border: none;
        border-radius: 8px;
        padding: 11px 28px;
        font-size: 13px;
        font-weight: 600;
        letter-spacing: 0.3px;
    }
    #btnPrimary:hover    { background: #5555DD; }
    #btnPrimary:pressed  { background: #3333BB; }

    /* Danger button */
    #btnDanger {
        background: #2A1A1A;
        color: #FF6666;
        border: 1px solid #3A2020;
        border-radius: 6px;
        padding: 6px 14px;
        font-size: 12px;
    }
    #btnDanger:hover { background: #3A1F1F; }

    /* Success button */
    #btnSuccess {
        background: #1A2A1A;
        color: #66CC66;
        border: 1px solid #203A20;
        border-radius: 6px;
        padding: 6px 14px;
        font-size: 12px;
    }
    #btnSuccess:hover { background: #1F3A1F; }

    /* Task card */
    #taskCard {
        background: #16161D;
        border: 1px solid #22222C;
        border-radius: 10px;
    }
    #taskCard:hover { border: 1px solid #33334A; }

    /* Priority badges */
    #badgeUrgent    { background: #3A1010; color: #FF5555; border-radius: 5px; padding: 2px 9px; font-size: 11px; font-weight: 700; }
    #badgeImportant { background: #2A2010; color: #FFB830; border-radius: 5px; padding: 2px 9px; font-size: 11px; font-weight: 700; }
    #badgeStandard  { background: #0E2230; color: #4CC9F0; border-radius: 5px; padding: 2px 9px; font-size: 11px; font-weight: 700; }

    /* Status label */
    #statusPending  { color: #55556A; font-size: 12px; }
    #statusSegera   { color: #FFB830; font-size: 12px; font-weight: 600; }
    #statusLewat    { color: #FF5555; font-size: 12px; font-weight: 600; }
    #statusSelesai  { color: #55AA55; font-size: 12px; }
    #statusTelat    { color: #FF4444; font-size: 12px; }

    /* Scroll */
    QScrollBar:vertical { background: #0F0F13; width: 6px; border-radius: 3px; }
    QScrollBar::handle:vertical { background: #2A2A3A; border-radius: 3px; min-height: 30px; }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

    /* Label */
    #formLabel { color: #888899; font-size: 12px; font-weight: 600; letter-spacing: 0.5px; margin-bottom: 2px; }

    /* History row */
    #histRow { background: #16161D; border: 1px solid #22222C; border-radius: 8px; padding: 12px 16px; }
    #histRowText { color: #AAAACC; font-size: 12px; font-family: 'Consolas', monospace; }

    /* Empty state */
    #emptyState { color: #333344; font-size: 15px; }

    /* Section divider */
    #divider { background: #1E1E28; border-radius: 1px; }
    )";
}

// ─── Sidebar active state ───────────────────────────────────
void MainWindow::setSidebarActive(QPushButton *btn) {
    for (auto *b : {btnCatat, btnLihat, btnHistory})
        b->setChecked(false);
    btn->setChecked(true);
}

// ─── Page: Catat ────────────────────────────────────────────
QWidget *MainWindow::buildPageCatat() {
    pageCatat = new QWidget;
    QVBoxLayout *vl = new QVBoxLayout(pageCatat);
    vl->setContentsMargins(36, 32, 36, 32);
    vl->setSpacing(0);

    auto *title = new QLabel("Catat Tugas Baru");
    title->setObjectName("pageTitle");
    vl->addWidget(title);
    auto *sub = new QLabel("Tambahkan tugas baru ke daftar kamu");
    sub->setObjectName("pageSub");
    vl->addWidget(sub);
    vl->addSpacing(30);

    // Helper lambda: add labeled field
    auto addField = [&](const QString &label, QWidget *field) {
        auto *lbl = new QLabel(label.toUpper());
        lbl->setObjectName("formLabel");
        vl->addWidget(lbl);
        vl->addSpacing(6);
        vl->addWidget(field);
        vl->addSpacing(18);
    };

    inputCatatan = new QLineEdit;
    inputCatatan->setPlaceholderText("Contoh: Kerjakan tugas statistika bab 5...");
    inputCatatan->setFixedHeight(44);
    addField("Catatan", inputCatatan);

    inputWaktu = new DateTimePicker;
    inputWaktu->setDateTime(QDateTime::currentDateTime().addSecs(3600));
    addField("Waktu Deadline", inputWaktu);

    inputPriority = new QComboBox;
    inputPriority->addItem("🔴  URGENT", 1);
    inputPriority->addItem("🟡  IMPORTANT", 2);
    inputPriority->addItem("🔵  STANDARD", 3);
    inputPriority->setFixedHeight(44);
    addField("Priority", inputPriority);

    vl->addSpacing(6);

    auto *btnTambah = new QPushButton("Tambah Tugas");
    btnTambah->setObjectName("btnPrimary");
    btnTambah->setFixedHeight(46);
    btnTambah->setCursor(Qt::PointingHandCursor);
    connect(btnTambah, &QPushButton::clicked, this, &MainWindow::onTambahTask);

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(btnTambah);
    hl->addStretch();
    vl->addLayout(hl);

    vl->addStretch();
    return pageCatat;
}

// ─── Page: Lihat ────────────────────────────────────────────
QWidget *MainWindow::buildPageLihat() {
    pageLihat = new QWidget;
    QVBoxLayout *vl = new QVBoxLayout(pageLihat);
    vl->setContentsMargins(36, 32, 36, 32);
    vl->setSpacing(0);

    auto *title = new QLabel("Daftar Tugas");
    title->setObjectName("pageTitle");
    vl->addWidget(title);
    auto *sub = new QLabel("Kelola, tandai selesai, atau hapus tugas");
    sub->setObjectName("pageSub");
    vl->addWidget(sub);
    vl->addSpacing(20);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    lihatContainer = new QWidget;
    lihatLayout    = new QVBoxLayout(lihatContainer);
    lihatLayout->setContentsMargins(0,0,0,0);
    lihatLayout->setSpacing(8);

    scroll->setWidget(lihatContainer);
    vl->addWidget(scroll, 1);

    return pageLihat;
}

// ─── Page: History ──────────────────────────────────────────
QWidget *MainWindow::buildPageHistory() {
    pageHistory = new QWidget;
    QVBoxLayout *vl = new QVBoxLayout(pageHistory);
    vl->setContentsMargins(36, 32, 36, 32);
    vl->setSpacing(0);

    auto *title = new QLabel("History Telat");
    title->setObjectName("pageTitle");
    vl->addWidget(title);
    auto *sub = new QLabel("Tugas yang ditandai selesai setelah deadline");
    sub->setObjectName("pageSub");
    vl->addWidget(sub);
    vl->addSpacing(20);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    historyContainer = new QWidget;
    historyLayout    = new QVBoxLayout(historyContainer);
    historyLayout->setContentsMargins(0,0,0,0);
    historyLayout->setSpacing(8);

    scroll->setWidget(historyContainer);
    vl->addWidget(scroll, 1);

    return pageHistory;
}

// ─── Show pages ─────────────────────────────────────────────
void MainWindow::showPageCatat() {
    pages->setCurrentWidget(pageCatat);
    setSidebarActive(btnCatat);
}

void MainWindow::showPageLihat() {
    pages->setCurrentWidget(pageLihat);
    setSidebarActive(btnLihat);
    refreshLihat();
}

void MainWindow::showPageHistory() {
    pages->setCurrentWidget(pageHistory);
    setSidebarActive(btnHistory);
    refreshHistory();
}

// ─── Tambah task ────────────────────────────────────────────
void MainWindow::onTambahTask() {
    QString cat = inputCatatan->text().trimmed();
    QString wkt = inputWaktu->dateTime().toString("dd/MM/yyyy HH:mm");

    if (cat.isEmpty()) {
        QMessageBox::warning(this, "Perhatian", "Catatan tidak boleh kosong.");
        return;
    }
    // DateTimePicker selalu valid — tidak perlu validasi manual format

    Task t;
    t.id       = tasks.isEmpty() ? 1 : tasks.last().id + 1;
    t.catatan  = cat;
    t.waktu    = wkt;
    t.priority = static_cast<Priority>(inputPriority->currentData().toInt());

    tasks.append(t);
    saveData();
    updateStats();

    inputCatatan->clear();
    inputWaktu->setDateTime(QDateTime::currentDateTime().addSecs(3600)); // reset ke default
    inputPriority->setCurrentIndex(2);

    trayIcon->showMessage("TodoList", "✓ Tugas \"" + cat + "\" berhasil dicatat.",
                          QSystemTrayIcon::Information, 2500);

    showPageLihat();
}

// ─── Refresh: Lihat ─────────────────────────────────────────
void MainWindow::refreshLihat() {
    // Clear
    QLayoutItem *item;
    while ((item = lihatLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // Sort: belum selesai dulu, lalu by priority
    QVector<Task*> sorted;
    for (auto &t : tasks) sorted.append(&t);
    std::sort(sorted.begin(), sorted.end(), [](Task *a, Task *b){
        if (a->selesai != b->selesai) return !a->selesai;
        return static_cast<int>(a->priority) < static_cast<int>(b->priority);
    });

    if (sorted.isEmpty()) {
        auto *emp = new QLabel("Belum ada tugas. Yuk catat sesuatu! ✍");
        emp->setObjectName("emptyState");
        emp->setAlignment(Qt::AlignCenter);
        lihatLayout->addWidget(emp);
        lihatLayout->addStretch();
        return;
    }

    for (Task *t : sorted) {
        // ── Card ──────────────────────────────────────────
        QFrame *card = new QFrame;
        card->setObjectName("taskCard");
        QHBoxLayout *hl = new QHBoxLayout(card);
        hl->setContentsMargins(16, 14, 16, 14);
        hl->setSpacing(14);

        // Priority badge
        QLabel *badge = new QLabel(t->priorityLabel());
        if      (t->priority == Priority::URGENT)    badge->setObjectName("badgeUrgent");
        else if (t->priority == Priority::IMPORTANT) badge->setObjectName("badgeImportant");
        else                                          badge->setObjectName("badgeStandard");
        badge->setFixedWidth(82);
        badge->setAlignment(Qt::AlignCenter);
        hl->addWidget(badge);

        // Text info
        QVBoxLayout *info = new QVBoxLayout;
        info->setSpacing(3);

        QLabel *lblCat = new QLabel(t->catatan);
        lblCat->setStyleSheet("color:#D8D8EC; font-size:13px; font-weight:600;");
        lblCat->setWordWrap(true);
        info->addWidget(lblCat);

        QLabel *lblWkt = new QLabel("⏰  " + t->waktu);
        lblWkt->setStyleSheet("color:#44445A; font-size:11px; font-family:'Consolas';");
        info->addWidget(lblWkt);

        if (t->selesai && !t->waktuSelesai.isEmpty()) {
            QLabel *lblSel = new QLabel("✓  Selesai: " + t->waktuSelesai);
            lblSel->setStyleSheet("color:#336633; font-size:11px; font-family:'Consolas';");
            info->addWidget(lblSel);
        }

        hl->addLayout(info, 1);

        // Status
        QLabel *lblStatus = new QLabel;
        if (t->selesai && t->telat) {
            lblStatus->setText("TELAT");
            lblStatus->setObjectName("statusTelat");
        } else if (t->selesai) {
            lblStatus->setText("SELESAI");
            lblStatus->setObjectName("statusSelesai");
        } else {
            long sisa = selisihMenit(t->waktu);
            if (sisa < 0)       { lblStatus->setText("LEWAT"); lblStatus->setObjectName("statusLewat"); }
            else if (sisa <= 15){ lblStatus->setText("SEGERA!"); lblStatus->setObjectName("statusSegera"); }
            else                 { lblStatus->setText("Pending"); lblStatus->setObjectName("statusPending"); }
        }
        hl->addWidget(lblStatus);

        // Buttons
        if (!t->selesai) {
            int taskId = t->id;
            QPushButton *btnOk = new QPushButton("✓ Selesai");
            btnOk->setObjectName("btnSuccess");
            btnOk->setCursor(Qt::PointingHandCursor);
            connect(btnOk, &QPushButton::clicked, this, [this, taskId]{
                for (auto &tk : tasks) {
                    if (tk.id == taskId && !tk.selesai) {
                        tk.selesai      = true;
                        tk.waktuSelesai = nowStr();
                        long sisa = selisihMenit(tk.waktu);
                        if (sisa < 0) {
                            tk.telat = true;
                            saveHistoryTelat(tk);
                            trayIcon->showMessage("TodoList ⛔", "Tugas \"" + tk.catatan + "\" ditandai TELAT!",
                                                  QSystemTrayIcon::Warning, 3000);
                        } else {
                            trayIcon->showMessage("TodoList ✓", "Tugas \"" + tk.catatan + "\" selesai tepat waktu!",
                                                  QSystemTrayIcon::Information, 2500);
                        }
                        saveData();
                        updateStats();
                        break;
                    }
                }
                refreshLihat();
            });
            hl->addWidget(btnOk);
        }

        {
            int taskId = t->id;
            QPushButton *btnDel = new QPushButton("✕ Hapus");
            btnDel->setObjectName("btnDanger");
            btnDel->setCursor(Qt::PointingHandCursor);
            connect(btnDel, &QPushButton::clicked, this, [this, taskId]{
                auto reply = QMessageBox::question(this, "Hapus Tugas",
                    "Yakin ingin menghapus tugas ini?",
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    tasks.erase(std::remove_if(tasks.begin(), tasks.end(),
                        [taskId](const Task &tk){ return tk.id == taskId; }), tasks.end());
                    saveData();
                    updateStats();
                    refreshLihat();
                }
            });
            hl->addWidget(btnDel);
        }

        lihatLayout->addWidget(card);
    }
    lihatLayout->addStretch();
}

// ─── Refresh: History ───────────────────────────────────────
void MainWindow::refreshHistory() {
    QLayoutItem *item;
    while ((item = historyLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    QFile f("history_telat.txt");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text) || f.atEnd()) {
        auto *emp = new QLabel("Belum ada tugas yang telat. Keep it up! 👍");
        emp->setObjectName("emptyState");
        emp->setAlignment(Qt::AlignCenter);
        historyLayout->addWidget(emp);
        historyLayout->addStretch();
        return;
    }

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty()) continue;
        QFrame *row = new QFrame;
        row->setObjectName("histRow");
        QHBoxLayout *hl = new QHBoxLayout(row);
        hl->setContentsMargins(14,10,14,10);
        QLabel *lbl = new QLabel(line);
        lbl->setObjectName("histRowText");
        lbl->setWordWrap(true);
        hl->addWidget(lbl);
        historyLayout->addWidget(row);
    }
    historyLayout->addStretch();
}

// ─── Notifikasi timer ───────────────────────────────────────
// Logika:
//   notif15, notif5, notif1, notif0 → one-shot (cukup sekali)
//   notif telat                     → REALTIME: muncul tiap ~5 menit selama belum selesai
void MainWindow::onNotifTick() {
    bool changed = false;
    QDateTime now = QDateTime::currentDateTime();

    for (auto &t : tasks) {
        if (t.selesai) continue;

        // selisih dalam DETIK agar presisi (untuk notif 1 menit)
        QDateTime dl = parseWaktu(t.waktu);
        if (!dl.isValid()) continue;
        qint64 sisaDetik = now.secsTo(dl);       // positif = belum lewat
        qint64 sisaMenit = sisaDetik / 60;

        // ── 15 menit sebelum (one-shot) ─────────────────────
        if (!t.notif15 && sisaMenit <= 15 && sisaMenit > 5) {
            t.notif15 = true; changed = true;
            trayIcon->showMessage("⚡ 15 Menit Lagi!",
                "\"" + t.catatan + "\"\nDeadline: " + t.waktu,
                QSystemTrayIcon::Warning, 6000);
        }

        // ── 5 menit sebelum (one-shot) ───────────────────────
        if (!t.notif5 && sisaMenit <= 5 && sisaMenit > 1) {
            t.notif5 = true; changed = true;
            trayIcon->showMessage("⚠ 5 Menit Lagi!",
                "\"" + t.catatan + "\"\nSegera selesaikan!",
                QSystemTrayIcon::Critical, 6000);
        }

        // ── 1 menit sebelum (one-shot) ───────────────────────
        if (!t.notif1 && sisaDetik <= 60 && sisaDetik > 0) {
            t.notif1 = true; changed = true;
            trayIcon->showMessage("🚨 1 Menit Lagi!",
                "\"" + t.catatan + "\"\nHampir deadline!",
                QSystemTrayIcon::Critical, 7000);
        }

        // ── Tepat deadline (one-shot) ────────────────────────
        if (!t.notif0 && sisaDetik <= 0 && sisaDetik > -60) {
            t.notif0 = true; changed = true;
            trayIcon->showMessage("🔔 DEADLINE SEKARANG!",
                "\"" + t.catatan + "\"\nWaktunya sudah habis!",
                QSystemTrayIcon::Critical, 8000);
        }

        // ── TELAT — REALTIME, muncul tiap 5 menit ───────────
        // Gunakan lastLateNotif: waktu terakhir kirim notif telat
        // Kita simpan di notifLate sebagai flag "sudah pernah lewat"
        // dan pakai lateNotifTime (detik epoch mod interval) untuk
        // throttle — cukup cek apakah sudah >5 menit sejak terakhir
        if (sisaDetik < 0) {  // sudah lewat deadline dan belum selesai
            qint64 mTelat = (-sisaDetik) / 60;  // sudah berapa menit telat
            // Kirim notif: pertama kali, lalu tiap 5 menit berikutnya
            // Kita pakai notifLate sebagai "sudah kirim notif ke-N"
            // Simpan counter di field notifLate (bool→gunakan logika modulo waktu)
            // Cukup: kirim jika (mTelat % 5 == 0) atau (mTelat == 1) —
            // tapi karena timer 10 detik, pakai detik agar tidak double-fire:
            // kirim hanya jika detik telat berada di window 0–10 setelah kelipatan 5 menit
            qint64 sTelat = -sisaDetik;
            bool windowFire = (sTelat % 300) < 11;  // 300 detik = 5 menit, window 10 detik
            bool firstFire  = !t.notifLate && sTelat >= 0;

            if (firstFire || windowFire) {
                if (firstFire) t.notifLate = true;
                changed = true;
                QString pesanTelat;
                if (mTelat < 1)
                    pesanTelat = "\"" + t.catatan + "\"\nBaru saja melewati deadline!";
                else
                    pesanTelat = "\"" + t.catatan + "\"\nSudah telat " + QString::number(mTelat) + " menit!";
                trayIcon->showMessage("⛔ TUGAS TELAT!",
                    pesanTelat,
                    QSystemTrayIcon::Critical, 8000);
            }
        }
    }

    if (changed) {
        saveData();
        if (pages->currentWidget() == pageLihat) refreshLihat();
    }
}

// ─── Stats ──────────────────────────────────────────────────
void MainWindow::updateStats() {
    int pending = 0, selesai = 0, telat = 0;
    for (auto &t : tasks) {
        if (t.selesai) { selesai++; if (t.telat) telat++; }
        else pending++;
    }
    lblStats->setText(QString("%1 pending\n%2 selesai · %3 telat")
        .arg(pending).arg(selesai).arg(telat));
}

// ─── Persist ────────────────────────────────────────────────
void MainWindow::saveData() {
    QFile f("tasks.dat");
    if (!f.open(QIODevice::WriteOnly)) return;
    QDataStream ds(&f);
    ds << tasks.size();
    for (auto &t : tasks) {
        ds << t.id << t.catatan << t.waktu
           << static_cast<int>(t.priority)
           << t.selesai << t.telat << t.waktuSelesai
           << t.notif15 << t.notif5 << t.notif1 << t.notif0 << t.notifLate;
    }
}

void MainWindow::loadData() {
    QFile f("tasks.dat");
    if (!f.open(QIODevice::ReadOnly)) { updateStats(); return; }
    QDataStream ds(&f);
    int n; ds >> n;
    for (int i = 0; i < n; i++) {
        Task t;
        int pri;
        // Baca field wajib
        ds >> t.id >> t.catatan >> t.waktu >> pri
           >> t.selesai >> t.telat >> t.waktuSelesai
           >> t.notif15 >> t.notif5;
        // notif1 adalah field baru — jika file lama (stream habis), default false
        if (!ds.atEnd()) ds >> t.notif1;
        if (!ds.atEnd()) ds >> t.notif0;
        if (!ds.atEnd()) ds >> t.notifLate;
        t.priority = static_cast<Priority>(pri);
        tasks.append(t);
    }
    updateStats();
}

void MainWindow::saveHistoryTelat(const Task &t) {
    QFile f("history_telat.txt");
    if (!f.open(QIODevice::Append | QIODevice::Text)) return;
    QTextStream out(&f);
    out << "[" << t.waktuSelesai << "]  #" << t.id
        << "  " << t.catatan
        << "  |  Deadline: " << t.waktu
        << "  |  Selesai: " << t.waktuSelesai << "\n";
}
