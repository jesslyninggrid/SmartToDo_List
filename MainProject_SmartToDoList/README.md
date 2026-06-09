# TodoList Qt — Cara Build & Jalankan

## Persyaratan
- **Qt 5.15** atau **Qt 6.x** (download: https://www.qt.io/download-qt-installer)
- Kompiler: MinGW (Windows) atau GCC/Clang (Linux/Mac)

---

## Cara Build (Qt Creator — PALING MUDAH)

1. Buka **Qt Creator**
2. File → Open File or Project → pilih `TodoList.pro`
3. Pilih kit (Desktop Qt 5.15 / Qt 6)
4. Klik tombol ▶ **Run** (Ctrl+R)

Hasil `.exe` ada di folder `build-TodoList-Desktop-Release/`

---

## Cara Build (Command Line / Git Bash)

Pastikan Qt sudah di PATH, lalu:

```bash
cd /path/ke/folder/todoqt

# Generate Makefile
qmake TodoList.pro -spec win32-g++ CONFIG+=release

# Compile
mingw32-make -j4

# Jalankan
./release/TodoList.exe
```

Atau di Linux/Mac:
```bash
qmake TodoList.pro
make -j4
./TodoList
```

---

## Deploy ke PC lain (Windows)

Setelah build, jalankan di folder hasil build:
```bash
windeployqt TodoList.exe
```
Ini akan menyalin semua DLL Qt yang dibutuhkan.

---

## File Data
- `tasks.dat`          — data tugas (dibuat otomatis)
- `history_telat.txt`  — log tugas telat (dibuat otomatis)

Kedua file ini ada di folder yang sama dengan `.exe`
