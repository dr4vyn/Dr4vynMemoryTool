# Dr4vyn Memory Tool

C++ ile yazılmış basit bir bellek okuma/yazma aracı.
Windows API'leri (OpenProcess, ReadProcessMemory, WriteProcessMemory) kullanılarak geliştirilmiştir.

## Özellikler
- Süreç bulma (Process ID)
- Bellekten değer okuma (ReadProcessMemory)
- Belleğe değer yazma (WriteProcessMemory)

## Kullanım
1. Programı derleyin: `g++ main.cpp -o MemoryTool.exe`
2. Çalıştırın: `./MemoryTool.exe`
