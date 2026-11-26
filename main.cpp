#include <windows.h>
#include <string>
#include <tlhelp32.h>
#include <thread>
#include <chrono>
#include <sstream>

// Global Değişkenler
HWND hStatusText;
HWND hAddressInput;
HWND hValueInput;
HWND hReadBtn;
HWND hWriteBtn;
HWND hOutputText;

std::string targetProcess = "explorer.exe"; // Test için explorer, oyun için değiştir
HANDLE hProcess = NULL;
bool isConnected = false;

// Süreç bulma fonksiyonu
DWORD GetProcessIdByName(const std::string& processName) {
    DWORD processId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &processEntry)) {
            do {
                std::string currentProcessName = processEntry.szExeFile;
                if (currentProcessName.find(processName) != std::string::npos) {
                    processId = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &processEntry));
        }
        CloseHandle(hSnapshot);
    }
    return processId;
}

// Bellek Okuma Fonksiyonu
template <typename T>
T ReadMemory(uintptr_t address) {
    T value = T();
    if (hProcess) {
        ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), nullptr);
    }
    return value;
}

// Bellek Yazma Fonksiyonu
template <typename T>
bool WriteMemory(uintptr_t address, T value) {
    if (hProcess) {
        return WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), nullptr);
    }
    return false;
}

// Pencere Olayları
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            // Butonlara tıklandığında
            if (LOWORD(wParam) == 1) { // Okuma Butonu
                char addrBuf[32];
                GetWindowTextA(hAddressInput, addrBuf, 32);
                uintptr_t address = strtoull(addrBuf, nullptr, 16);
                
                int value = ReadMemory<int>(address);
                std::string result = "Okunan Deger (Int): " + std::to_string(value);
                SetWindowTextA(hOutputText, result.c_str());
            }
            else if (LOWORD(wParam) == 2) { // Yazma Butonu
                char addrBuf[32];
                GetWindowTextA(hAddressInput, addrBuf, 32);
                uintptr_t address = strtoull(addrBuf, nullptr, 16);
                
                char valBuf[32];
                GetWindowTextA(hValueInput, valBuf, 32);
                int value = atoi(valBuf);

                if (WriteMemory<int>(address, value)) {
                    SetWindowTextA(hOutputText, "Bellek basariyla yazildi!");
                } else {
                    SetWindowTextA(hOutputText, "Bellek yazma basarisiz!");
                }
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(52, 211, 153));
            SetBkColor(hdc, RGB(15, 15, 20));
            static HBRUSH hBrush = CreateSolidBrush(RGB(15, 15, 20));
            return (LRESULT)hBrush;
        }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

// Arka Plan Thread'i (Sürekli Süreç Arar)
void ProcessCheckThread(HWND hwnd) {
    while (true) {
        if (!isConnected) {
            DWORD pid = GetProcessIdByName(targetProcess);
            if (pid != 0) {
                hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, FALSE, pid);
                if (hProcess) {
                    isConnected = true;
                    std::string msg = "Baglanti Basarili! PID: " + std::to_string(pid);
                    SetWindowTextA(hStatusText, msg.c_str());
                }
            } else {
                SetWindowTextA(hStatusText, "Oyun Bekleniyor...");
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main() {
    // Pencere Sınıfı
    const char* CLASS_NAME = "Dr4vynMemoryToolClass";
    WNDCLASSA wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(15, 15, 20));
    RegisterClassA(&wc);

    // Ana Pencere
    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "Dr4vyn Memory Tool",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 450, 350,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (hwnd == NULL) return 0;

    // Durum Yazısı
    hStatusText = CreateWindowA("STATIC", "Oyun Bekleniyor...",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 20, 450, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Adres Girişi
    CreateWindowA("STATIC", "Adres (Hex):", WS_VISIBLE | WS_CHILD, 30, 70, 100, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    hAddressInput = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 140, 70, 250, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Değer Girişi
    CreateWindowA("STATIC", "Deger (Int):", WS_VISIBLE | WS_CHILD, 30, 110, 100, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    hValueInput = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 140, 110, 250, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Butonlar
    hReadBtn = CreateWindowA("BUTTON", "Oku", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 140, 150, 100, 30, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
    hWriteBtn = CreateWindowA("BUTTON", "Yaz", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 250, 150, 100, 30, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);

    // Çıktı Metni
    hOutputText = CreateWindowA("STATIC", "", WS_VISIBLE | WS_CHILD | SS_CENTER, 0, 200, 450, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Font Ayarı
    HFONT hFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Consolas");
    SendMessageA(hStatusText, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageA(hOutputText, WM_SETFONT, (WPARAM)hFont, TRUE);

    ShowWindow(hwnd, SW_SHOW);

    // Arka plan thread'ini başlat
    std::thread thread(ProcessCheckThread, hwnd);
    thread.detach();

    // Mesaj Döngüsü
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}
