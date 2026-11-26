#include <iostream>
#include <windows.h>
#include <string>
#include <tlhelp32.h>
#include <algorithm>
#include <iomanip>

using namespace std;

string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

DWORD GetProcessIdByName(const string& processName) {
    DWORD processId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        string targetLower = toLower(processName);
        if (Process32First(hSnapshot, &processEntry)) {
            do {
                string currentProcessName = processEntry.szExeFile;
                string currentLower = toLower(currentProcessName);
                if (currentLower.find(targetLower) != string::npos) {
                    processId = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &processEntry));
        }
        CloseHandle(hSnapshot);
    }
    return processId;
}

// Bellekten okuma şablonu
template <typename T>
T ReadMemory(HANDLE hProcess, uintptr_t address) {
    T value = T();
    ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), nullptr);
    return value;
}

// Belleğe yazma şablonu
template <typename T>
bool WriteMemory(HANDLE hProcess, uintptr_t address, T value) {
    return WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), nullptr);
}

int main() {
    string targetProcess;
    cout << "Hedef Oyunun EXE Adini Girin (Ornegin: explorer): ";
    cin >> targetProcess;

    DWORD pid = GetProcessIdByName(targetProcess);
    if (pid == 0) {
        cout << "Hata: Process bulunamadi! Oyun acik mi?" << endl;
        return 1;
    }

    cout << "Process Bulundu! PID: " << pid << endl;
    cout << "Bellege baglaniliyor..." << endl;

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        cout << "Hata: Process'e baglanilamadi! (Yonetici olarak calistirmayi deneyin)" << endl;
        return 1;
    }

    cout << "Baglanti basarili!" << endl;

    // İşlem seçimi
    int choice;
    cout << "Islem secin (1: Oku, 2: Yaz): ";
    cin >> choice;

    uintptr_t address;
    cout << "Adresi Hex olarak girin (Ornegin: 0x00400000): ";
    cin >> hex >> address;

    if (choice == 1) {
        // Okuma (Integer)
        int value = ReadMemory<int>(hProcess, address);
        cout << "Adresteki Deger (Int): " << dec << value << endl;
    } else if (choice == 2) {
        // Yazma (Integer)
        int newValue;
        cout << "Yeni Degeri girin (Int): ";
        cin >> dec >> newValue;

        if (WriteMemory<int>(hProcess, address, newValue)) {
            cout << "Bellek basariyla yazildi!" << endl;
        } else {
            cout << "Bellek yazma basarisiz! (Korumali alan olabilir)" << endl;
        }
    }

    CloseHandle(hProcess);
    return 0;
}