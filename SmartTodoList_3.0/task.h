#ifndef TASK_H
#define TASK_H

#include <QString>

enum class Priority { URGENT = 1, IMPORTANT = 2, STANDARD = 3 };

struct Task {
    int      id       = 0;
    QString  catatan;
    QString  waktu;           // DD/MM/YYYY HH:MM
    Priority priority = Priority::STANDARD;
    bool     selesai  = false;
    bool     telat    = false;
    QString  waktuSelesai;

    bool     notif15  = false;
    bool     notif5   = false;
    bool     notif0   = false;
    bool     notifLate = false;

    QString priorityLabel() const {
        switch (priority) {
            case Priority::URGENT:    return "URGENT";
            case Priority::IMPORTANT: return "IMPORTANT";
            default:                  return "STANDARD";
        }
    }

    QString priorityColor() const {
        switch (priority) {
            case Priority::URGENT:    return "#FF4C4C";
            case Priority::IMPORTANT: return "#FFB830";
            default:                  return "#4CC9F0";
        }
    }
};

#endif
