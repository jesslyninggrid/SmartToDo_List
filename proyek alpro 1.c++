#include <stdio.h>      // Untuk printf dan scanf
#include <stdlib.h>     // Untuk system("cls") dan system("pause")
#include <string.h>     // Untuk strlen
#include <ctype.h>      // Untuk tolower
#include <vector>       // Untuk vector 
#include <string>       // Untuk tipe data string
#include <algorithm>    // Untuk sort

using namespace std;

// Struktur data untuk menyimpan satu catatan to-do
struct Catatan {
    int id;             // ID catatan
    string teks;        // Isi catatan
    string prioritas;   // Prioritas
    bool selesai;       // selesai atau belum
};

// Membersihkan layar
void syscls() {
    system("cls");
}

// Menghapus buffer newline di akhir string C
void hapusNewline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

// Menghapus spasi kosong di awal dan akhir string
string trim(string s) {
    size_t awal = s.find_first_not_of(" \t\r\n");               
    size_t akhir = s.find_last_not_of(" \t\r\n");

    if (awal == string::npos) return "";
    return s.substr(awal, akhir - awal + 1);
}

// Mengubah string menjadi huruf kecil semua
string toLower(string s) {
    for (size_t i = 0; i < s.length(); i++) {
        s[i] = tolower((unsigned char)s[i]);
    }
    return s;
}

// Mengecek apakah input prioritas valid
bool prioritasvalid(const string &p) {
    string t = toLower(trim(p));
    return (t == "mendesak" || t == "penting" || t == "biasa");
}

// Merapikan formaat tulisan prioritas
string normalisasiprioritas(string p) {
    p = trim(p);
    p = toLower(p);

    if (p == "mendesak") return "Mendesak";
    if (p == "penting")  return "Penting";
    return "Biasa";
}

// Mengubah prioritas menjadi angka untuk sorting
int nilaiPrioritas(const string &p) {
    string t = toLower(p);
    if (t == "mendesak") return 1;
    if (t == "penting")  return 2;
    return 3;
}

// Mencari index catatan berdasarkan ID
int cariIndexById(const vector<Catatan> &daftar, int id) {
    for (size_t i = 0; i < daftar.size(); i++) {
        if (daftar[i].id == id) return (int)i;
    }
    return -1; // Jika ID tidak ditemukan
}

// Menampilkan semua catatan dalam bentuk tabel
void tampilkanDaftar(const vector<Catatan> &daftar) {
    // Jika daftar kosong, tampilkan pesan
    if (daftar.empty()) {
        printf("\n[Daftar kosong]\n");
        return;
    }

    // Salin data agar urutan asli tidak berubah
    vector<Catatan> urut = daftar;

    // Urutkan: belum selesai dulu, lalu prioritas, lalu ID
    sort(urut.begin(), urut.end(), [](const Catatan &a, const Catatan &b) {
        if (a.selesai != b.selesai) return a.selesai < b.selesai;

        int pa = nilaiPrioritas(a.prioritas);
        int pb = nilaiPrioritas(b.prioritas);
        if (pa != pb) return pa < pb;

        return a.id < b.id;
    });

    // header tabel
    printf("\n===== DAFTAR TO-DO =====\n");
    printf("ID | Status   | Prioritas | Catatan\n");
    printf("------------------------------------------\n");

    // isi catatan satu per satu
    for (size_t i = 0; i < urut.size(); i++) {
        printf("%2d | %-8s | %-9s | %s\n",
               urut[i].id,
               urut[i].selesai ? "Selesai" : "Belum",
               urut[i].prioritas.c_str(),
               urut[i].teks.c_str());
    }
}

int main() {
    vector<Catatan> daftar; // Menyimpan semua catatan
    int nextId = 1;         // ID berikutnya yang akan dipakai
    int pilihan;            // Menyimpan pilihan menu utama

    while (1) {
        syscls(); // Bersihkan layar setiap masuk menu utama

        // MENU UTAMA
        printf("===== MENU TO-DO LIST =====\n");
        printf("1. Buat catatan\n");
        printf("2. Lihat / tandai selesai\n");
        printf("3. Hapus catatan\n");
        printf("4. Keluar\n");
        printf("Pilih menu: ");

        // Validasi input menu
        if (scanf("%d", &pilihan) != 1) {
            printf("Input tidak valid.\n");
            while (getchar() != '\n'); // Bersihkan buffer input
            system("pause");
            continue;
        }
        while (getchar() != '\n'); // Buang sisa buffer newline

        // keluar
        if (pilihan == 4) {
            printf("Program selesai.\n");
            break;
        }

        // MENU 1
        if (pilihan == 1) {
            while (1) {
                syscls();
                printf("===== MENU TO-DO LIST =====\n");
                printf("-> MENU 1: BUAT CATATAN\n\n");

                char buffer[300];
                string teks, prioritas;

                // Input isi catatan
                printf("Masukkan catatan (kosong untuk kembali): ");
                fgets(buffer, sizeof(buffer), stdin);
                hapusNewline(buffer);
                teks = trim(string(buffer));

                // Jika kosong, kembali ke menu utama
                if (teks.empty()) {
                    printf("Kembali ke menu.\n");
                    system("pause");
                    break;
                }

                // Input prioritas, hanya ulangi bagian prioritas jika salah
                while (1) {
                    printf("\nPrioritas [Mendesak/Penting/Biasa]: ");
                    fgets(buffer, sizeof(buffer), stdin);
                    hapusNewline(buffer);
                    prioritas = trim(string(buffer));

                    if (prioritasvalid(prioritas)) {
                        prioritas = normalisasiprioritas(prioritas);
                        break;
                    } else {
                        printf("\n>> Prioritas tidak valid. Ulangi lagi! <<\n");
                    }
                }

                // Simpan catatan baru ke vector
                daftar.push_back({nextId, teks, prioritas, false});
                printf("\nCatatan ditambahkan dengan ID %d.\n", nextId);
                nextId++;

                // Tambah catatan lagi?
                char lagi;
                printf(">> Tambah catatan lagi? (y/n) << : ");
                scanf(" %c", &lagi);
                while (getchar() != '\n');

                if (lagi != 'y' && lagi != 'Y') {
                    break;
                }
            }
        }

        // MENU 2
        else if (pilihan == 2) {
            if (daftar.empty()) {
                syscls();
                printf("Daftar kosong.\n");
                system("pause");
                continue;
            }

            while (1) {
                syscls();
                printf("===== MENU TO-DO LIST =====\n");
                printf("-> MENU 2: LIHAT / TANDAI SELESAI\n");

                tampilkanDaftar(daftar);

                // Input ID yang akan ditandai selesai
                printf("\nMasukkan ID catatan yang selesai (0 = kembali): ");
                int id;
                if (scanf("%d", &id) != 1) {
                    printf("Input tidak valid.\n");
                    while (getchar() != '\n');
                    system("pause");
                    continue;
                }
                while (getchar() != '\n');

                if (id == 0) break;

                // Cari ID dan ubah status selesai
                int idx = cariIndexById(daftar, id);
                if (idx == -1) {
                    printf("ID tidak ditemukan.\n");
                } else {
                    daftar[idx].selesai = true;
                    printf("Catatan ditandai selesai.\n");
                }

                system("pause");
            }
        }

        // MENU 3
        else if (pilihan == 3) {
            if (daftar.empty()) {
                syscls();
                printf("Daftar kosong.\n");
                system("pause");
                continue;
            }

            while (1) {
                syscls();
                printf("===== MENU TO-DO LIST =====\n");
                printf("-> MENU 3: HAPUS CATATAN\n");

                tampilkanDaftar(daftar);

                // Input ID yang akan dihapus
                printf("\nMasukkan ID catatan yang dihapus (0 = kembali): ");
                int id;
                if (scanf("%d", &id) != 1) {
                    printf("Input tidak valid.\n");
                    while (getchar() != '\n');
                    system("pause");
                    continue;
                }
                while (getchar() != '\n');

                if (id == 0) break;

                // Cari ID dan hapus catatan
                int idx = cariIndexById(daftar, id);
                if (idx == -1) {
                    printf("ID tidak ditemukan.\n");
                } else {
                    daftar.erase(daftar.begin() + idx);
                    printf("Catatan dihapus.\n");
                }

                system("pause");
            }
        }

        // Jika pilihan menu tidak sesuai
        else {
            syscls();
            printf("Menu tidak valid.\n");
            system("pause");
        }
    }


}
