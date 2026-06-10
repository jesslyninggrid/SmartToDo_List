#include "datetimepicker.h"

#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QScrollBar>
#include <QAbstractItemView>

// ─── shared dark style untuk popup ─────────────────
static QString popupStyle() {
    return R"(
        QDialog {
            background: #1A1A24;
            border: 1px solid #33334A;
            border-radius: 12px;
        }
        QPushButton#navBtn {
            background: transparent;
            color: #9999BB;
            border: none;
            font-size: 16px;
            padding: 4px 10px;
            border-radius: 6px;
        }
        QPushButton#navBtn:hover { background: #2A2A3A; color: #E8E8F0; }

        QPushButton#dayBtn {
            background: transparent;
            color: #C0C0D8;
            border: none;
            border-radius: 6px;
            font-size: 12px;
            min-width: 34px;
            min-height: 34px;
        }
        QPushButton#dayBtn:hover    { background: #2A2A3A; }
        QPushButton#dayBtn[today="1"] { color: #7C7CFF; font-weight: 700; }
        QPushButton#dayBtn[selected="1"] {
            background: #4444CC;
            color: #FFFFFF;
            font-weight: 700;
        }
        QPushButton#dayBtn[other="1"]   { color: #44445A; }
        QPushButton#dayBtn:disabled     { color: #2A2A3A; }

        QLabel#monthLabel {
            color: #E8E8F0;
            font-size: 14px;
            font-weight: 700;
        }
        QLabel#dayHeader {
            color: #55556A;
            font-size: 11px;
            font-weight: 600;
            qproperty-alignment: AlignCenter;
        }

        /* Time list */
        QListWidget {
            background: #16161D;
            border: 1px solid #2A2A35;
            border-radius: 8px;
            color: #D0D0E8;
            font-size: 15px;
            outline: none;
        }
        QListWidget::item {
            padding: 6px 0;
            text-align: center;
        }
        QListWidget::item:selected {
            background: #4444CC;
            color: #FFFFFF;
            border-radius: 6px;
        }
        QListWidget::item:hover:!selected { background: #2A2A3A; border-radius: 6px; }

        QScrollBar:vertical {
            background: transparent;
            width: 4px;
        }
        QScrollBar::handle:vertical { background: #2A2A3A; border-radius: 2px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

        QPushButton#confirmBtn {
            background: #4444CC;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            padding: 10px 28px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton#confirmBtn:hover   { background: #5555DD; }
        QPushButton#confirmBtn:pressed { background: #3333BB; }

        QLabel#timeLabel {
            color: #55556A;
            font-size: 12px;
            font-weight: 600;
            letter-spacing: 1px;
            qproperty-alignment: AlignCenter;
        }
        QLabel#separator {
            color: #7C7CFF;
            font-size: 22px;
            font-weight: 700;
            qproperty-alignment: AlignCenter;
        }
    )";
}

// ═══════════════════════════════════════════════════
//  CalendarPopup
// ═══════════════════════════════════════════════════
CalendarPopup::CalendarPopup(QDate current, QWidget *parent)
    : QDialog(parent, Qt::Popup | Qt::FramelessWindowHint)
    , m_date(current.isValid() ? current : QDate::currentDate())
    , m_view(m_date)
{
    setStyleSheet(popupStyle());
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    mainVl = new QVBoxLayout(this);
    mainVl->setContentsMargins(14, 14, 14, 14);
    mainVl->setSpacing(8);

    // ── Header: prev / month-year / next ──
    QHBoxLayout *header = new QHBoxLayout;

    auto *btnPrev = new QPushButton("‹");
    btnPrev->setObjectName("navBtn");
    btnPrev->setCursor(Qt::PointingHandCursor);
    btnPrev->setFocusPolicy(Qt::NoFocus);

    lblMonth = new QLabel;
    lblMonth->setObjectName("monthLabel");
    lblMonth->setAlignment(Qt::AlignCenter);

    auto *btnNext = new QPushButton("›");
    btnNext->setObjectName("navBtn");
    btnNext->setCursor(Qt::PointingHandCursor);
    btnNext->setFocusPolicy(Qt::NoFocus);

    header->addWidget(btnPrev);
    header->addWidget(lblMonth, 1);
    header->addWidget(btnNext);
    mainVl->addLayout(header);

    connect(btnPrev, &QPushButton::clicked, this, &CalendarPopup::prevMonth);
    connect(btnNext, &QPushButton::clicked, this, &CalendarPopup::nextMonth);

    // ── Day headers ──
    QGridLayout *headerGrid = new QGridLayout;
    headerGrid->setSpacing(2);
    const QStringList days = {"Sen","Sel","Rab","Kam","Jum","Sab","Min"};
    for (int i = 0; i < 7; i++) {
        auto *lbl = new QLabel(days[i]);
        lbl->setObjectName("dayHeader");
        headerGrid->addWidget(lbl, 0, i);
    }
    mainVl->addLayout(headerGrid);

    // ── Grid placeholder ──
    gridWidget = new QWidget;
    grid = new QGridLayout(gridWidget);
    grid->setSpacing(2);
    mainVl->addWidget(gridWidget);

    buildGrid();
}

void CalendarPopup::buildGrid() {
    // Clear existing buttons
    QLayoutItem *item;
    while ((item = grid->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // Update month label
    static const QStringList bulan = {
        "Januari","Februari","Maret","April","Mei","Juni",
        "Juli","Agustus","September","Oktober","November","Desember"
    };
    lblMonth->setText(bulan[m_view.month()-1] + "  " + QString::number(m_view.year()));

    QDate first(m_view.year(), m_view.month(), 1);
    // Qt: Monday=1 ... Sunday=7
    int startCol = first.dayOfWeek() - 1; // 0-based (Sen=0)
    int daysInMonth = m_view.daysInMonth();
    QDate today = QDate::currentDate();

    // prev month filler
    QDate prevMonth = m_view.addMonths(-1);
    int prevDays = prevMonth.daysInMonth();
    for (int c = startCol - 1; c >= 0; c--) {
        int d = prevDays - (startCol - 1 - c);
        auto *btn = new QPushButton(QString::number(d));
        btn->setObjectName("dayBtn");
        btn->setProperty("other", true);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setEnabled(false);
        grid->addWidget(btn, 0, c);
    }

    int row = 0, col = startCol;
    for (int d = 1; d <= daysInMonth; d++) {
        QDate thisDate(m_view.year(), m_view.month(), d);
        auto *btn = new QPushButton(QString::number(d));
        btn->setObjectName("dayBtn");
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setCursor(Qt::PointingHandCursor);

        if (thisDate == today)   btn->setProperty("today",    true);
        if (thisDate == m_date)  btn->setProperty("selected", true);
        if (thisDate < today)    btn->setEnabled(false);

        connect(btn, &QPushButton::clicked, this, [this, thisDate]{
            m_date = thisDate;
            emit dateSelected(m_date);
            accept();
        });

        grid->addWidget(btn, row, col);
        col++;
        if (col == 7) { col = 0; row++; }
    }

    // next month filler
    int remaining = (col == 0) ? 0 : 7 - col;
    for (int i = 0; i < remaining; i++) {
        auto *btn = new QPushButton(QString::number(i+1));
        btn->setObjectName("dayBtn");
        btn->setProperty("other", true);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setEnabled(false);
        grid->addWidget(btn, row, col + i);
    }

    adjustSize();
}

void CalendarPopup::prevMonth() {
    m_view = m_view.addMonths(-1);
    buildGrid();
}

void CalendarPopup::nextMonth() {
    m_view = m_view.addMonths(1);
    buildGrid();
}

// ═══════════════════════════════════════════════════
//  TimePopup
// ═══════════════════════════════════════════════════
TimePopup::TimePopup(QTime current, QWidget *parent)
    : QDialog(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setStyleSheet(popupStyle());
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(18, 18, 18, 18);
    vl->setSpacing(12);

    // Title
    QLabel *title = new QLabel("Pilih Waktu");
    title->setObjectName("monthLabel");
    title->setAlignment(Qt::AlignCenter);
    vl->addWidget(title);

    // Hour + separator + Minute
    QHBoxLayout *hl = new QHBoxLayout;
    hl->setSpacing(8);

    // --- Hour list ---
    QVBoxLayout *vHour = new QVBoxLayout;
    QLabel *lblH = new QLabel("JAM");
    lblH->setObjectName("timeLabel");
    lstHour = new QListWidget;
    lstHour->setFixedSize(72, 220);
    lstHour->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    lstHour->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lstHour->setFocusPolicy(Qt::NoFocus);
    for (int h = 0; h < 24; h++) {
        auto *item = new QListWidgetItem(QString("%1").arg(h, 2, 10, QChar('0')));
        item->setTextAlignment(Qt::AlignCenter);
        lstHour->addItem(item);
    }
    lstHour->setCurrentRow(current.isValid() ? current.hour() : 0);
    lstHour->scrollToItem(lstHour->currentItem(), QAbstractItemView::PositionAtCenter);
    vHour->addWidget(lblH);
    vHour->addWidget(lstHour);
    hl->addLayout(vHour);

    // --- Separator ---
    QLabel *sep = new QLabel(":");
    sep->setObjectName("separator");
    sep->setFixedWidth(16);
    hl->addWidget(sep, 0, Qt::AlignVCenter);

    // --- Minute list ---
    QVBoxLayout *vMin = new QVBoxLayout;
    QLabel *lblM = new QLabel("MENIT");
    lblM->setObjectName("timeLabel");
    lstMinute = new QListWidget;
    lstMinute->setFixedSize(72, 220);
    lstMinute->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    lstMinute->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lstMinute->setFocusPolicy(Qt::NoFocus);
    for (int m = 0; m < 60; m++) {
        auto *item = new QListWidgetItem(QString("%1").arg(m, 2, 10, QChar('0')));
        item->setTextAlignment(Qt::AlignCenter);
        lstMinute->addItem(item);
    }
    lstMinute->setCurrentRow(current.isValid() ? current.minute() : 0);
    lstMinute->scrollToItem(lstMinute->currentItem(), QAbstractItemView::PositionAtCenter);
    vMin->addWidget(lblM);
    vMin->addWidget(lstMinute);
    hl->addLayout(vMin);

    vl->addLayout(hl);

    // Confirm button
    auto *btnOk = new QPushButton("✓  Pilih");
    btnOk->setObjectName("confirmBtn");
    btnOk->setFocusPolicy(Qt::NoFocus);
    btnOk->setCursor(Qt::PointingHandCursor);
    connect(btnOk, &QPushButton::clicked, this, &TimePopup::onConfirm);
    vl->addWidget(btnOk);

    adjustSize();
}

void TimePopup::onConfirm() {
    QTime t(lstHour->currentRow(), lstMinute->currentRow());
    emit timeSelected(t);
    accept();
}

QTime TimePopup::selectedTime() const {
    return QTime(lstHour->currentRow(), lstMinute->currentRow());
}

// ═══════════════════════════════════════════════════
//  DateTimePicker
// ═══════════════════════════════════════════════════
DateTimePicker::DateTimePicker(QWidget *parent)
    : QWidget(parent)
    , m_date(QDate::currentDate())
    , m_time(QTime::currentTime().addSecs(3600))
{
    auto *hl = new QHBoxLayout(this);
    hl->setContentsMargins(0,0,0,0);
    hl->setSpacing(10);

    btnDate = new QPushButton;
    btnDate->setObjectName("datePickerBtn");
    btnDate->setFixedHeight(44);
    btnDate->setCursor(Qt::PointingHandCursor);
    btnDate->setFocusPolicy(Qt::NoFocus);

    btnTime = new QPushButton;
    btnTime->setObjectName("timePickerBtn");
    btnTime->setFixedWidth(120);
    btnTime->setFixedHeight(44);
    btnTime->setCursor(Qt::PointingHandCursor);
    btnTime->setFocusPolicy(Qt::NoFocus);

    hl->addWidget(btnDate, 1);
    hl->addWidget(btnTime);

    connect(btnDate, &QPushButton::clicked, this, &DateTimePicker::openDatePicker);
    connect(btnTime, &QPushButton::clicked, this, &DateTimePicker::openTimePicker);

    updateLabels();
}

void DateTimePicker::updateLabels() {
    static const QStringList bulan = {
        "Jan","Feb","Mar","Apr","Mei","Jun",
        "Jul","Agu","Sep","Okt","Nov","Des"
    };
    QString hari;
    switch (m_date.dayOfWeek()) {
        case 1: hari="Senin"; break; case 2: hari="Selasa"; break;
        case 3: hari="Rabu";  break; case 4: hari="Kamis";  break;
        case 5: hari="Jumat"; break; case 6: hari="Sabtu";  break;
        default: hari="Minggu"; break;
    }
    btnDate->setText(QString("📅  %1, %2 %3 %4")
        .arg(hari)
        .arg(m_date.day())
        .arg(bulan[m_date.month()-1])
        .arg(m_date.year()));

    btnTime->setText(QString("🕐  %1:%2")
        .arg(m_time.hour(),   2, 10, QChar('0'))
        .arg(m_time.minute(), 2, 10, QChar('0')));
}

void DateTimePicker::openDatePicker() {
    auto *popup = new CalendarPopup(m_date, this);

    // posisikan di bawah button
    QPoint pos = btnDate->mapToGlobal(QPoint(0, btnDate->height() + 4));
    QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    popup->adjustSize();
    if (pos.x() + popup->width() > screen.right())
        pos.setX(screen.right() - popup->width());
    popup->move(pos);

    connect(popup, &CalendarPopup::dateSelected, this, [this](QDate d){
        m_date = d;
        updateLabels();
    });
    popup->exec();
    popup->deleteLater();
}

void DateTimePicker::openTimePicker() {
    auto *popup = new TimePopup(m_time, this);

    QPoint pos = btnTime->mapToGlobal(QPoint(0, btnTime->height() + 4));
    QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    popup->adjustSize();
    int px = pos.x();
    if (px + popup->width() > screen.right())
        px = screen.right() - popup->width();
    popup->move(px, pos.y());

    connect(popup, &TimePopup::timeSelected, this, [this](QTime t){
        m_time = t;
        updateLabels();
    });
    popup->exec();
    popup->deleteLater();
}

QDateTime DateTimePicker::dateTime() const {
    return QDateTime(m_date, m_time);
}

void DateTimePicker::setDateTime(const QDateTime &dt) {
    m_date = dt.date();
    m_time = dt.time();
    updateLabels();
}
