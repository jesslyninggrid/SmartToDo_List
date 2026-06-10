#ifndef DATETIMEPICKER_H
#define DATETIMEPICKER_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QDialog>
#include <QListWidget>

// ═══════════════════════════════════════════════════
//  CalendarPopup  — pilih tanggal, full click
// ═══════════════════════════════════════════════════
class CalendarPopup : public QDialog {
    Q_OBJECT
public:
    explicit CalendarPopup(QDate current, QWidget *parent = nullptr);
    QDate selectedDate() const { return m_date; }

signals:
    void dateSelected(QDate date);

private:
    void buildGrid();
    void prevMonth();
    void nextMonth();

    QDate        m_date;     // terpilih
    QDate        m_view;     // bulan yg ditampilkan
    QLabel      *lblMonth;
    QGridLayout *grid;
    QWidget     *gridWidget;
    QVBoxLayout *mainVl;
};

// ═══════════════════════════════════════════════════
//  TimePopup  — pilih jam & menit, full click (scroll)
// ═══════════════════════════════════════════════════
class TimePopup : public QDialog {
    Q_OBJECT
public:
    explicit TimePopup(QTime current, QWidget *parent = nullptr);
    QTime selectedTime() const;

signals:
    void timeSelected(QTime time);

private:
    QListWidget *lstHour;
    QListWidget *lstMinute;
    void onConfirm();
};

// ═══════════════════════════════════════════════════
//  DateTimePicker  — widget utama (2 button: date + time)
// ═══════════════════════════════════════════════════
class DateTimePicker : public QWidget {
    Q_OBJECT
public:
    explicit DateTimePicker(QWidget *parent = nullptr);

    QDateTime dateTime() const;
    void      setDateTime(const QDateTime &dt);

private slots:
    void openDatePicker();
    void openTimePicker();

private:
    void updateLabels();

    QDate        m_date;
    QTime        m_time;
    QPushButton *btnDate;
    QPushButton *btnTime;
};

#endif // DATETIMEPICKER_H
