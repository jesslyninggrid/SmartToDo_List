#ifndef TASK_H
#define TASK_H

#include <QString>

enum class Priority { URGENT = 1, IMPORTANT = 2, STANDARD = 3 };

struct Task {
    int      id           = 0;
    QString  catatan;
    QString  waktu;          // "dd/MM/yyyy HH:mm"
    Priority priority     = Priority::STANDARD;

    bool     selesai      = false;
    bool     telat        = false;
    QString  waktuSelesai;

    // notif flags — sekali tembak, kecuali notifLate yg realtime
    bool     notif15      = false;   // sudah kirim notif 15 menit sebelum
    bool     notif5       = false;   // sudah kirim notif 5 menit sebelum
    bool     notif1       = false;   // sudah kirim notif 1 menit sebelum
    bool     notif0       = false;   // sudah kirim notif tepat deadline
    bool     notifLate    = false;   // sudah kirim notif pertama kali telat
    // notifLate TIDAK dipakai sebagai one-shot lagi — lihat logika baru

    QString priorityLabel() const {
        switch (priority) {
            case Priority::URGENT:    return "URGENT";
            case Priority::IMPORTANT: return "IMPORTANT";
            default:                  return "STANDARD";
        }
    }
};

#endif
