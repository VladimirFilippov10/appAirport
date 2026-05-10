// appAirport.cpp : Аэропорт - Система управления рейсами
//

#include "framework.h"
#include "appAirport.h"
#include <commctrl.h>
#include "Aircraft.h"
#include "Flight.h"
#include "Ticket.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <codecvt>
#include <locale>
#include <set>

#pragma comment(lib, "comctl32.lib")

#define MAX_LOADSTRING 100

// ID контролов
#define IDC_LIST_FLIGHTS       1001
#define IDC_LIST_RESULTS       1002
#define IDC_BTN_ADD_FLIGHT     1003
#define IDC_BTN_DEL_FLIGHT     1004
#define IDC_BTN_SAVE           1005
#define IDC_BTN_LOAD           1006
#define IDC_BTN_CLEAR          1007
#define IDC_BTN_CHECK_SEATS    1008
#define IDC_BTN_FIND_DEST      1009
#define IDC_BTN_MAX_PRICE      1010
#define IDC_BTN_NEAREST        1011
#define IDC_BTN_SAVE_QUERY     1012
#define IDC_BTN_SELL_TICKET    1013
#define IDC_EDIT_FLIGHT_NUM    1014
#define IDC_EDIT_DEST          1015
#define IDC_EDIT_DEPART        1016
#define IDC_EDIT_TIME          1017
#define IDC_EDIT_DURATION      1018
#define IDC_EDIT_SEATS         1019
#define IDC_EDIT_QUERY_FLIGHT  1020
#define IDC_EDIT_QUERY_DEST    1021
#define IDC_EDIT_TICKET_FLIGHT 1022
#define IDC_EDIT_PASSENGER     1023
#define IDC_EDIT_PRICE         1024
#define IDC_EDIT_TICKET_CASHIER 1025
#define IDC_COMBO_AIRCRAFT     1026

// Глобальные переменные
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND g_hWnd;

// Данные приложения
std::vector<Aircraft> aircrafts;
std::vector<Flight> flights;
std::vector<Ticket> tickets;
std::vector<std::string> lastQueryResults;

// Файлы
const std::string AIRCRAFTS_FILE = "aircrafts.txt";
const std::string FLIGHTS_FILE = "flights.txt";
const std::string TICKETS_FILE = "tickets.txt";

// Хэндлы контролов
HWND hListFlights, hListResults;
HWND hEditFlightNum, hEditDest, hEditDepart, hEditTime, hEditDuration, hEditSeats;
HWND hEditQueryFlight, hEditQueryDest;
HWND hEditTicketFlight, hEditPassenger, hEditPrice, hEditTicketCashier;
HWND hComboAircraft;

// Объявления функций
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void CreateControls(HWND hWnd);
void RefreshFlightsList();
void RefreshResultsListW(const std::vector<std::wstring>& results);
void UpdateAircraftCombo();
void LoadAircrafts();
void LoadData();
void SaveData();
void LoadTickets();
void SaveTickets();
void AddFlight();
void DeleteFlight();
void SellTicket();
void QueryFreeSeats();
void QueryFlightsByDestination();
void QueryMaxTicketPrice();
void QueryNearestFlight();
void SaveQueryResult();
void DebugShowLoadedFlights();
std::time_t ParseDateTime(const std::string& str);
std::string WCharToString(const wchar_t* wstr);
std::wstring StringToWString(const std::string& str);
std::string WStringToString(const std::wstring& wstr);
double SafeStod(const std::string& str, double defaultValue = 0.0);
int SafeStoi(const std::string& str, int defaultValue = 0);
std::string TrimString(const std::string& str);

// ========== ТОЧКА ВХОДА ==========
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_APPAIRPORT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_APPAIRPORT));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPAIRPORT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_APPAIRPORT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;
    g_hWnd = CreateWindowW(szWindowClass, L"Аэропорт - Система управления рейсами",
        WS_OVERLAPPEDWINDOW | WS_VSCROLL,  // Добавлена вертикальная прокрутка
        CW_USEDEFAULT, 0, 1500, 950, nullptr, nullptr, hInstance, nullptr);
    if (!g_hWnd) return FALSE;

    // Устанавливаем диапазон прокрутки
    SCROLLINFO si = { sizeof(SCROLLINFO) };
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = 400;  // Высота содержимого
    si.nPage = 900; // Высота окна
    SetScrollInfo(g_hWnd, SB_VERT, &si, TRUE);

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    return TRUE;
}

// ========== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ==========
std::string TrimString(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// ========== ЗАГРУЗКА САМОЛЁТОВ ==========
void LoadAircrafts() {
    aircrafts.clear();

    std::ifstream file(AIRCRAFTS_FILE);
    if (!file.is_open()) {
        std::ofstream newFile(AIRCRAFTS_FILE);
        if (newFile.is_open()) {
            newFile << "Boeing 737-800;Среднемагистральный;180\n";
            newFile << "Airbus A320;Среднемагистральный;160\n";
            newFile << "Sukhoi Superjet 100;Ближнемагистральный;100\n";
            newFile << "Boeing 777-300ER;Дальнемагистральный;300\n";
            newFile << "Airbus A380;Широкофюзеляжный;500\n";
            newFile.close();
        }
        file.open(AIRCRAFTS_FILE);
    }

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::vector<std::string> parts;
            std::stringstream ss(line);
            std::string part;
            while (std::getline(ss, part, ';')) {
                parts.push_back(part);
            }

            if (parts.size() >= 3) {
                aircrafts.push_back(Aircraft(parts[0], parts[1], SafeStoi(parts[2], 150)));
            }
        }
        file.close();
    }

    if (aircrafts.empty()) {
        aircrafts.push_back(Aircraft("Boeing 737-800", "Среднемагистральный", 180));
        aircrafts.push_back(Aircraft("Airbus A320", "Среднемагистральный", 160));
        aircrafts.push_back(Aircraft("Sukhoi Superjet 100", "Ближнемагистральный", 100));
    }
}

// ========== ОБНОВЛЕНИЕ КОМБОБОКСА ==========
void UpdateAircraftCombo() {
    SendMessage(hComboAircraft, CB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < aircrafts.size(); i++) {
        std::string display = aircrafts[i].getName() + " (" + aircrafts[i].getCategory() + ", " + std::to_string(aircrafts[i].getSeats()) + " мест)";
        std::wstring wdisplay = StringToWString(display);
        SendMessage(hComboAircraft, CB_ADDSTRING, 0, (LPARAM)wdisplay.c_str());
    }
    SendMessage(hComboAircraft, CB_SETCURSEL, 0, 0);
}

// ========== ПРЕОБРАЗОВАНИЕ КОДИРОВОК ==========
std::string WCharToString(const wchar_t* wstr) {
    if (!wstr) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (len <= 1) return "";
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], len, NULL, NULL);
    return result;
}

std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
    if (size <= 0) return L"";
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &result[0], size);
    return result;
}

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (size <= 1) return "";
    std::string result(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, NULL, NULL);
    return result;
}

// ========== СОЗДАНИЕ ИНТЕРФЕЙСА ==========
void CreateControls(HWND hWnd) {
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icex);

    // Таблица рейсов (7 колонок)
    hListFlights = CreateWindowW(WC_LISTVIEW, NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        10, 10, 1000, 320, hWnd, (HMENU)IDC_LIST_FLIGHTS, hInst, NULL);

    LVCOLUMNW lvc = { LVCF_TEXT | LVCF_WIDTH };
    const wchar_t* cols[] = { L"Номер рейса", L"Самолёт", L"Назначение", L"Отправление", L"Время вылета", L"Длит.", L"Мест" };
    int widths[] = { 120, 220, 160, 160, 180, 70, 70 };
    for (int i = 0; i < 7; i++) {
        lvc.cx = widths[i];
        lvc.pszText = const_cast<wchar_t*>(cols[i]);
        ListView_InsertColumn(hListFlights, i, &lvc);
    }

    // Панель добавления рейса
    CreateWindowW(L"STATIC", L"ДОБАВЛЕНИЕ РЕЙСА", WS_CHILD | WS_VISIBLE | SS_CENTER,
        1030, 10, 420, 30, hWnd, NULL, hInst, NULL);

    CreateWindowW(L"STATIC", L"Номер рейса:", WS_CHILD | WS_VISIBLE, 1030, 50, 100, 30, hWnd, NULL, hInst, NULL);
    hEditFlightNum = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1140, 50, 280, 30, hWnd, (HMENU)IDC_EDIT_FLIGHT_NUM, hInst, NULL);

    CreateWindowW(L"STATIC", L"Назначение (куда):", WS_CHILD | WS_VISIBLE, 1030, 90, 130, 30, hWnd, NULL, hInst, NULL);
    hEditDest = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1170, 90, 250, 30, hWnd, (HMENU)IDC_EDIT_DEST, hInst, NULL);

    CreateWindowW(L"STATIC", L"Отправление (откуда):", WS_CHILD | WS_VISIBLE, 1030, 130, 140, 30, hWnd, NULL, hInst, NULL);
    hEditDepart = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1180, 130, 240, 30, hWnd, (HMENU)IDC_EDIT_DEPART, hInst, NULL);

    CreateWindowW(L"STATIC", L"Время (ГГГГ-ММ-ДД ЧЧ:ММ):", WS_CHILD | WS_VISIBLE, 1030, 170, 200, 30, hWnd, NULL, hInst, NULL);
    hEditTime = CreateWindowW(L"EDIT", L"2025-05-10 08:00", WS_CHILD | WS_VISIBLE | WS_BORDER, 1240, 170, 180, 30, hWnd, (HMENU)IDC_EDIT_TIME, hInst, NULL);

    CreateWindowW(L"STATIC", L"Длительность (мин):", WS_CHILD | WS_VISIBLE, 1030, 210, 130, 30, hWnd, NULL, hInst, NULL);
    hEditDuration = CreateWindowW(L"EDIT", L"120", WS_CHILD | WS_VISIBLE | WS_BORDER, 1170, 210, 120, 30, hWnd, (HMENU)IDC_EDIT_DURATION, hInst, NULL);

    CreateWindowW(L"STATIC", L"Кол-во мест:", WS_CHILD | WS_VISIBLE, 1030, 250, 100, 30, hWnd, NULL, hInst, NULL);
    hEditSeats = CreateWindowW(L"EDIT", L"50", WS_CHILD | WS_VISIBLE | WS_BORDER, 1140, 250, 120, 30, hWnd, (HMENU)IDC_EDIT_SEATS, hInst, NULL);

    // ComboBox для выбора самолёта
    CreateWindowW(L"STATIC", L"Самолёт:", WS_CHILD | WS_VISIBLE, 1030, 290, 100, 30, hWnd, NULL, hInst, NULL);
    hComboAircraft = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        1140, 290, 350, 150, hWnd, (HMENU)IDC_COMBO_AIRCRAFT, hInst, NULL);
    SendMessage(hComboAircraft, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    // Кнопки
    CreateWindowW(L"BUTTON", L"+ ДОБАВИТЬ РЕЙС", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        1030, 335, 180, 40, hWnd, (HMENU)IDC_BTN_ADD_FLIGHT, hInst, NULL);
    CreateWindowW(L"BUTTON", L"- УДАЛИТЬ РЕЙС", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        1225, 335, 180, 40, hWnd, (HMENU)IDC_BTN_DEL_FLIGHT, hInst, NULL);

    // Панель управления
    CreateWindowW(L"BUTTON", L"ЗАГРУЗИТЬ ДАННЫЕ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 345, 160, 40, hWnd, (HMENU)IDC_BTN_LOAD, hInst, NULL);
    CreateWindowW(L"BUTTON", L"СОХРАНИТЬ ДАННЫЕ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        180, 345, 160, 40, hWnd, (HMENU)IDC_BTN_SAVE, hInst, NULL);
    CreateWindowW(L"BUTTON", L"ОЧИСТИТЬ ВСЁ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        350, 345, 160, 40, hWnd, (HMENU)IDC_BTN_CLEAR, hInst, NULL);

    // Запросы
    CreateWindowW(L"STATIC", L"ЗАПРОСЫ", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 400, 700, 30, hWnd, NULL, hInst, NULL);

    CreateWindowW(L"BUTTON", L"Свободные места", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 440, 150, 35, hWnd, (HMENU)IDC_BTN_CHECK_SEATS, hInst, NULL);
    CreateWindowW(L"STATIC", L"Номер рейса:", WS_CHILD | WS_VISIBLE, 175, 445, 90, 30, hWnd, NULL, hInst, NULL);
    hEditQueryFlight = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 275, 445, 160, 30, hWnd, (HMENU)IDC_EDIT_QUERY_FLIGHT, hInst, NULL);

    CreateWindowW(L"BUTTON", L"По направлению", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 485, 150, 35, hWnd, (HMENU)IDC_BTN_FIND_DEST, hInst, NULL);
    CreateWindowW(L"STATIC", L"Город (куда):", WS_CHILD | WS_VISIBLE, 175, 490, 100, 30, hWnd, NULL, hInst, NULL);
    hEditQueryDest = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 285, 490, 150, 30, hWnd, (HMENU)IDC_EDIT_QUERY_DEST, hInst, NULL);

    CreateWindowW(L"BUTTON", L"Макс.цена билета", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 530, 150, 35, hWnd, (HMENU)IDC_BTN_MAX_PRICE, hInst, NULL);
    CreateWindowW(L"BUTTON", L"Ближайший рейс", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 575, 150, 35, hWnd, (HMENU)IDC_BTN_NEAREST, hInst, NULL);
    CreateWindowW(L"BUTTON", L"СОХРАНИТЬ РЕЗУЛЬТАТ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 625, 200, 40, hWnd, (HMENU)IDC_BTN_SAVE_QUERY, hInst, NULL);

    // Таблица результатов
    CreateWindowW(L"STATIC", L"РЕЗУЛЬТАТЫ ЗАПРОСОВ", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 680, 700, 30, hWnd, NULL, hInst, NULL);
    hListResults = CreateWindowW(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | LVS_REPORT, 10, 715, 1000, 100, hWnd, (HMENU)IDC_LIST_RESULTS, hInst, NULL);
    lvc.cx = 980;
    lvc.pszText = const_cast<wchar_t*>(L"Результат");
    ListView_InsertColumn(hListResults, 0, &lvc);

    // Продажа билетов
    CreateWindowW(L"STATIC", L"ПРОДАЖА БИЛЕТОВ", WS_CHILD | WS_VISIBLE | SS_CENTER, 1030, 400, 420, 30, hWnd, NULL, hInst, NULL);
    CreateWindowW(L"STATIC", L"Номер рейса:", WS_CHILD | WS_VISIBLE, 1030, 445, 100, 30, hWnd, NULL, hInst, NULL);
    hEditTicketFlight = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1140, 445, 280, 30, hWnd, (HMENU)IDC_EDIT_TICKET_FLIGHT, hInst, NULL);
    CreateWindowW(L"STATIC", L"Пассажир (ФИО):", WS_CHILD | WS_VISIBLE, 1030, 485, 110, 30, hWnd, NULL, hInst, NULL);
    hEditPassenger = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1150, 485, 310, 30, hWnd, (HMENU)IDC_EDIT_PASSENGER, hInst, NULL);
    CreateWindowW(L"STATIC", L"Цена (руб):", WS_CHILD | WS_VISIBLE, 1030, 525, 100, 30, hWnd, NULL, hInst, NULL);
    hEditPrice = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 1140, 525, 200, 30, hWnd, (HMENU)IDC_EDIT_PRICE, hInst, NULL);
    CreateWindowW(L"STATIC", L"Касса №:", WS_CHILD | WS_VISIBLE, 1030, 565, 100, 30, hWnd, NULL, hInst, NULL);
    hEditTicketCashier = CreateWindowW(L"EDIT", L"001", WS_CHILD | WS_VISIBLE | WS_BORDER, 1140, 565, 120, 30, hWnd, (HMENU)IDC_EDIT_TICKET_CASHIER, hInst, NULL);
    CreateWindowW(L"BUTTON", L"ПРОДАТЬ БИЛЕТ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1140, 610, 240, 45, hWnd, (HMENU)IDC_BTN_SELL_TICKET, hInst, NULL);

    CreateWindowW(L"STATIC", L"Совет: Для запросов Макс.цена и Ближайший рейс используйте номер рейса и город из полей выше",
        WS_CHILD | WS_VISIBLE, 10, 830, 1000, 30, hWnd, NULL, hInst, NULL);

    // Загружаем данные
    LoadAircrafts();
    UpdateAircraftCombo();
    LoadData();
    LoadTickets();
    DebugShowLoadedFlights();
}

// ========== ОБНОВЛЕНИЕ ТАБЛИЦЫ РЕЙСОВ ==========
void RefreshFlightsList() {
    ListView_DeleteAllItems(hListFlights);
    for (size_t i = 0; i < flights.size(); i++) {
        std::wstring num = StringToWString(flights[i].getNumber());
        std::wstring aircraft = StringToWString(flights[i].getAircraft() ? flights[i].getAircraft()->getName() : "Нет");
        std::wstring dest = StringToWString(flights[i].getDestination());
        std::wstring depart = StringToWString(flights[i].getDeparturePoint());
        std::wstring time = StringToWString(flights[i].getDepartureTimeStr());
        std::wstring dur = std::to_wstring(flights[i].getDurationMinutes());
        std::wstring seats = std::to_wstring(flights[i].getFreeSeats());

        LVITEMW lvi = { LVIF_TEXT, (int)i, 0 };
        lvi.pszText = (LPWSTR)num.c_str();
        ListView_InsertItem(hListFlights, &lvi);
        ListView_SetItemText(hListFlights, (int)i, 1, (LPWSTR)aircraft.c_str());
        ListView_SetItemText(hListFlights, (int)i, 2, (LPWSTR)dest.c_str());
        ListView_SetItemText(hListFlights, (int)i, 3, (LPWSTR)depart.c_str());
        ListView_SetItemText(hListFlights, (int)i, 4, (LPWSTR)time.c_str());
        ListView_SetItemText(hListFlights, (int)i, 5, (LPWSTR)dur.c_str());
        ListView_SetItemText(hListFlights, (int)i, 6, (LPWSTR)seats.c_str());
    }
}

// ========== ОБНОВЛЕНИЕ ТАБЛИЦЫ РЕЗУЛЬТАТОВ ==========
void RefreshResultsListW(const std::vector<std::wstring>& results) {
    ListView_DeleteAllItems(hListResults);
    for (size_t i = 0; i < results.size(); i++) {
        LVITEMW lvi = { LVIF_TEXT, (int)i, 0 };
        lvi.pszText = const_cast<LPWSTR>(results[i].c_str());
        ListView_InsertItem(hListResults, &lvi);
    }
}

// ========== ОТЛАДОЧНАЯ ФУНКЦИЯ ==========
void DebugShowLoadedFlights() {
    std::vector<std::wstring> results;
    results.push_back(L"=== ЗАГРУЖЕННЫЕ РЕЙСЫ ===");
    for (const auto& f : flights) {
        results.push_back(L"Рейс: " + StringToWString(f.getNumber()) +
            L" | Откуда: " + StringToWString(f.getDeparturePoint()) +
            L" | Куда: " + StringToWString(f.getDestination()) +
            L" | Мест: " + std::to_wstring(f.getFreeSeats()) +
            L" | Время: " + StringToWString(f.getDepartureTimeStr()));
    }
    if (flights.empty()) {
        results.push_back(L"Нет загруженных рейсов!");
    }
    RefreshResultsListW(results);
}

// ========== ЗАГРУЗКА/СОХРАНЕНИЕ ==========
void LoadData() {
    flights.clear();
    std::ifstream file(FLIGHTS_FILE);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::vector<std::string> parts;
            std::stringstream ss(line);
            std::string part;
            while (std::getline(ss, part, ';')) {
                parts.push_back(part);
            }

            if (parts.size() >= 7) {
                Flight flight;
                flight.setNumber(parts[0]);

                for (auto& ac : aircrafts) {
                    if (ac.getName() == parts[1]) {
                        flight.setAircraft(&ac);
                        break;
                    }
                }

                // parts[2] - отправление (откуда), parts[3] - назначение (куда)
                flight.setDeparturePoint(TrimString(parts[2]));  // Откуда
                flight.setDestination(TrimString(parts[3]));     // Куда
                flight.setDepartureTime(ParseDateTime(parts[4]));
                flight.setDurationMinutes(SafeStoi(parts[5], 120));
                flight.setFreeSeats(SafeStoi(parts[6], 50));
                flights.push_back(flight);
            }
        }
        file.close();
    }
    RefreshFlightsList();
}

void SaveData() {
    std::ofstream file(FLIGHTS_FILE);
    for (const auto& f : flights) {
        std::string aircraftName = f.getAircraft() ? f.getAircraft()->getName() : "";
        file << f.getNumber() << ";" << aircraftName << ";"
            << f.getDeparturePoint() << ";"   // Откуда
            << f.getDestination() << ";"      // Куда
            << f.getDepartureTimeStr() << ";"
            << f.getDurationMinutes() << ";" << f.getFreeSeats() << std::endl;
    }
    file.close();
}

void LoadTickets() {
    tickets.clear();
    std::ifstream file(TICKETS_FILE);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::vector<std::string> parts;
            std::stringstream ss(line);
            std::string part;
            while (std::getline(ss, part, ';')) {
                parts.push_back(part);
            }

            if (parts.size() >= 4) {
                Ticket ticket;
                ticket.setCashierNumber(parts[0]);
                ticket.setFlightNumber(parts[1]);
                ticket.setPassengerName(parts[2]);
                ticket.setPrice(SafeStod(parts[3], 0.0));
                tickets.push_back(ticket);
            }
        }
        file.close();
    }
}

void SaveTickets() {
    std::ofstream file(TICKETS_FILE);
    for (const auto& t : tickets) {
        file << t.getCashierNumber() << ";" << t.getFlightNumber() << ";"
            << t.getPassengerName() << ";" << t.getPrice() << std::endl;
    }
    file.close();
}

// ========== ДОБАВЛЕНИЕ РЕЙСА ==========
void AddFlight() {
    wchar_t buffer[256];
    GetWindowTextW(hEditFlightNum, buffer, 256);
    std::string number = TrimString(WCharToString(buffer));
    GetWindowTextW(hEditDest, buffer, 256);
    std::string dest = TrimString(WCharToString(buffer));        // Куда (назначение)
    GetWindowTextW(hEditDepart, buffer, 256);
    std::string depart = TrimString(WCharToString(buffer));      // Откуда (отправление)
    GetWindowTextW(hEditTime, buffer, 256);
    std::string timeStr = TrimString(WCharToString(buffer));
    GetWindowTextW(hEditDuration, buffer, 256);
    int duration = _wtoi(buffer);
    GetWindowTextW(hEditSeats, buffer, 256);
    int seats = _wtoi(buffer);

    int selIndex = (int)SendMessage(hComboAircraft, CB_GETCURSEL, 0, 0);
    if (selIndex == CB_ERR) selIndex = 0;
    Aircraft* selectedAircraft = &aircrafts[selIndex];

    if (number.empty() || dest.empty() || depart.empty()) {
        MessageBoxW(g_hWnd, L"Заполните все поля!", L"Ошибка", MB_OK);
        return;
    }

    if (duration <= 0) duration = 120;
    if (seats <= 0) seats = selectedAircraft->getSeats();

    Flight flight;
    flight.setNumber(number);
    flight.setAircraft(selectedAircraft);
    flight.setDestination(dest);         // Куда
    flight.setDeparturePoint(depart);    // Откуда
    flight.setDepartureTime(ParseDateTime(timeStr));
    flight.setDurationMinutes(duration);
    flight.setFreeSeats(seats);

    flights.push_back(flight);
    SaveData();
    RefreshFlightsList();

    SetWindowTextW(hEditFlightNum, L"");
    SetWindowTextW(hEditDest, L"");
    SetWindowTextW(hEditDepart, L"");
    SetWindowTextW(hEditDuration, L"");
    SetWindowTextW(hEditSeats, L"");
    MessageBoxW(g_hWnd, L"Рейс добавлен!", L"Успех", MB_OK);
}

void DeleteFlight() {
    int sel = ListView_GetSelectionMark(hListFlights);
    if (sel < 0) {
        MessageBoxW(g_hWnd, L"Выберите рейс для удаления!", L"Ошибка", MB_OK);
        return;
    }
    wchar_t buffer[256];
    ListView_GetItemText(hListFlights, sel, 0, buffer, 256);
    std::string number = TrimString(WCharToString(buffer));

    for (size_t i = 0; i < flights.size(); i++) {
        if (flights[i].getNumber() == number) {
            flights.erase(flights.begin() + i);
            break;
        }
    }
    SaveData();
    RefreshFlightsList();
    MessageBoxW(g_hWnd, L"Рейс удалён!", L"Успех", MB_OK);
}

void SellTicket() {
    wchar_t buffer[256];
    GetWindowTextW(hEditTicketFlight, buffer, 256);
    std::string flightNum = TrimString(WCharToString(buffer));
    GetWindowTextW(hEditPassenger, buffer, 256);
    std::string passenger = TrimString(WCharToString(buffer));
    GetWindowTextW(hEditPrice, buffer, 256);
    double price = _wtof(buffer);
    GetWindowTextW(hEditTicketCashier, buffer, 256);
    std::string cashier = TrimString(WCharToString(buffer));

    if (flightNum.empty() || passenger.empty() || price <= 0) {
        MessageBoxW(g_hWnd, L"Заполните все поля!", L"Ошибка", MB_OK);
        return;
    }

    Flight* found = nullptr;
    for (auto& f : flights) {
        if (f.getNumber() == flightNum) {
            found = &f;
            break;
        }
    }
    if (!found) {
        MessageBoxW(g_hWnd, L"Рейс не найден!", L"Ошибка", MB_OK);
        return;
    }
    if (found->getFreeSeats() <= 0) {
        MessageBoxW(g_hWnd, L"Нет свободных мест!", L"Ошибка", MB_OK);
        return;
    }

    found->setFreeSeats(found->getFreeSeats() - 1);

    Ticket ticket;
    ticket.setCashierNumber(cashier);
    ticket.setFlightNumber(flightNum);
    ticket.setPassengerName(passenger);
    ticket.setPrice(price);
    tickets.push_back(ticket);

    SaveData();
    SaveTickets();
    RefreshFlightsList();

    SetWindowTextW(hEditTicketFlight, L"");
    SetWindowTextW(hEditPassenger, L"");
    SetWindowTextW(hEditPrice, L"");
    MessageBoxW(g_hWnd, L"Билет продан!", L"Успех", MB_OK);
}

// ========== ЗАПРОСЫ ==========
void QueryFreeSeats() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryFlight, buffer, 256);
    std::string flightNum = TrimString(WCharToString(buffer));
    std::vector<std::wstring> results;

    for (const auto& f : flights) {
        if (f.getNumber() == flightNum) {
            results.push_back(L"Рейс " + StringToWString(flightNum) + L" - свободных мест: " + std::to_wstring(f.getFreeSeats()));
            lastQueryResults.clear();
            for (const auto& r : results) {
                lastQueryResults.push_back(WStringToString(r));
            }
            RefreshResultsListW(results);
            return;
        }
    }
    results.push_back(L"Рейс " + StringToWString(flightNum) + L" не найден");
    lastQueryResults.clear();
    for (const auto& r : results) {
        lastQueryResults.push_back(WStringToString(r));
    }
    RefreshResultsListW(results);
}

void QueryFlightsByDestination() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryDest, buffer, 256);
    std::string dest = TrimString(WCharToString(buffer));

    std::vector<std::wstring> results;
    results.push_back(L"Поиск города: [" + StringToWString(dest) + L"]");
    results.push_back(L"Всего рейсов: " + std::to_wstring(flights.size()));

    bool found = false;
    for (const auto& f : flights) {
        std::string flightDest = TrimString(f.getDestination());

        results.push_back(L"Проверка рейса " + StringToWString(f.getNumber()) +
            L" назначение: [" + StringToWString(flightDest) + L"]");

        if (flightDest == dest) {
            results.push_back(L">>> НАЙДЕН! Рейс " + StringToWString(f.getNumber()) +
                L" - свободных мест: " + std::to_wstring(f.getFreeSeats()));
            found = true;
        }
    }

    if (!found) {
        results.push_back(L"Нет рейсов в [" + StringToWString(dest) + L"]");
        // Показываем все доступные направления
        std::set<std::string> uniqueDests;
        for (const auto& f : flights) {
            uniqueDests.insert(TrimString(f.getDestination()));
        }
        results.push_back(L"Доступные направления:");
        for (const auto& d : uniqueDests) {
            results.push_back(L"  - " + StringToWString(d));
        }
    }

    lastQueryResults.clear();
    for (const auto& r : results) {
        lastQueryResults.push_back(WStringToString(r));
    }
    RefreshResultsListW(results);
}

void QueryMaxTicketPrice() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryFlight, buffer, 256);
    std::string flightNum = TrimString(WCharToString(buffer));
    std::vector<std::wstring> results;
    double maxPrice = -1;
    std::string maxPricePassenger;

    for (const auto& t : tickets) {
        if (t.getFlightNumber() == flightNum && t.getPrice() > maxPrice) {
            maxPrice = t.getPrice();
            maxPricePassenger = t.getPassengerName();
        }
    }

    results.push_back(L"Поиск макс.цены для рейса: " + StringToWString(flightNum));
    results.push_back(L"Всего билетов: " + std::to_wstring(tickets.size()));

    if (maxPrice >= 0) {
        results.push_back(L">>> Макс. цена: " + std::to_wstring(maxPrice) + L" руб.");
        results.push_back(L"Пассажир: " + StringToWString(maxPricePassenger));
    }
    else {
        results.push_back(L"Нет проданных билетов на рейс " + StringToWString(flightNum));
    }

    lastQueryResults.clear();
    for (const auto& r : results) {
        lastQueryResults.push_back(WStringToString(r));
    }
    RefreshResultsListW(results);
}

void QueryNearestFlight() {
    wchar_t buffer[256];
    GetWindowTextW(hEditQueryDest, buffer, 256);
    std::string dest = TrimString(WCharToString(buffer));
    std::vector<std::wstring> results;
    std::time_t now = std::time(nullptr);
    const Flight* nearest = nullptr;
    time_t minDiff = -1;

    results.push_back(L"Поиск ближайшего рейса в: " + StringToWString(dest));
    char timeBuf[26];
    ctime_s(timeBuf, sizeof(timeBuf), &now);
    results.push_back(L"Текущее время: " + StringToWString(std::string(timeBuf)));

    for (const auto& f : flights) {
        if (TrimString(f.getDestination()) == dest && f.getFreeSeats() > 0) {
            time_t diff = std::difftime(f.getDepartureTime(), now);
            results.push_back(L"Рейс " + StringToWString(f.getNumber()) +
                L" через " + std::to_wstring(diff / 60) + L" мин");
            if (diff > 0 && (nearest == nullptr || diff < minDiff)) {
                nearest = &f;
                minDiff = diff;
            }
        }
    }

    if (nearest) {
        results.push_back(L">>> Ближайший рейс: " + StringToWString(nearest->getNumber()) +
            L" через " + std::to_wstring(minDiff / 60) + L" мин");
    }
    else {
        results.push_back(L"Нет рейсов в " + StringToWString(dest));
    }

    lastQueryResults.clear();
    for (const auto& r : results) {
        lastQueryResults.push_back(WStringToString(r));
    }
    RefreshResultsListW(results);
}

void SaveQueryResult() {
    if (lastQueryResults.empty()) {
        MessageBoxW(g_hWnd, L"Нет результатов для сохранения!", L"Ошибка", MB_OK);
        return;
    }
    std::ofstream file("query_result.txt");
    if (file.is_open()) {
        for (const auto& line : lastQueryResults) {
            file << line << std::endl;
        }
        file.close();
        MessageBoxW(g_hWnd, L"Результат сохранён в файл query_result.txt", L"Успех", MB_OK);
    }
    else {
        MessageBoxW(g_hWnd, L"Ошибка сохранения файла!", L"Ошибка", MB_OK);
    }
}

// ========== ВСПОМОГАТЕЛЬНЫЕ ==========
std::time_t ParseDateTime(const std::string& str) {
    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
    if (ss.fail()) {
        return (std::time_t)0;
    }
    return std::mktime(&tm);
}

double SafeStod(const std::string& str, double defaultValue) {
    if (str.empty()) return defaultValue;
    char* endPtr = nullptr;
    double result = strtod(str.c_str(), &endPtr);
    if (endPtr == str.c_str() || *endPtr != '\0') return defaultValue;
    return result;
}

int SafeStoi(const std::string& str, int defaultValue) {
    if (str.empty()) return defaultValue;
    char* endPtr = nullptr;
    long result = strtol(str.c_str(), &endPtr, 10);
    if (endPtr == str.c_str() || *endPtr != '\0') return defaultValue;
    return (int)result;
}

// ========== ОБРАБОТЧИК СООБЩЕНИЙ ==========
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateControls(hWnd);
        break;
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDM_ABOUT: DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About); break;
        case IDM_EXIT: DestroyWindow(hWnd); break;
        case IDC_BTN_ADD_FLIGHT: AddFlight(); break;
        case IDC_BTN_DEL_FLIGHT: DeleteFlight(); break;
        case IDC_BTN_LOAD: LoadData(); LoadTickets(); RefreshFlightsList(); DebugShowLoadedFlights(); break;
        case IDC_BTN_SAVE: SaveData(); SaveTickets(); break;
        case IDC_BTN_CLEAR: flights.clear(); tickets.clear(); SaveData(); SaveTickets(); RefreshFlightsList(); DebugShowLoadedFlights(); break;
        case IDC_BTN_CHECK_SEATS: QueryFreeSeats(); break;
        case IDC_BTN_FIND_DEST: QueryFlightsByDestination(); break;
        case IDC_BTN_MAX_PRICE: QueryMaxTicketPrice(); break;
        case IDC_BTN_NEAREST: QueryNearestFlight(); break;
        case IDC_BTN_SAVE_QUERY: SaveQueryResult(); break;
        case IDC_BTN_SELL_TICKET: SellTicket(); break;
        default: return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_DESTROY:
        SaveData();
        SaveTickets();
        PostQuitMessage(0);
        break;
    case WM_VSCROLL: {
        SCROLLINFO si = { sizeof(SCROLLINFO) };
        si.fMask = SIF_ALL;
        GetScrollInfo(hWnd, SB_VERT, &si);
        int yPos = si.nPos;

        switch (LOWORD(wParam)) {
        case SB_LINEUP:      yPos -= 10; break;
        case SB_LINEDOWN:    yPos += 10; break;
        case SB_PAGEUP:      yPos -= 100; break;
        case SB_PAGEDOWN:    yPos += 100; break;
        case SB_THUMBTRACK:  yPos = si.nTrackPos; break;
        default: break;
        }

        yPos = max(0, min(yPos, si.nMax - (int)si.nPage + 10));
        if (yPos != si.nPos) {
            SetScrollPos(hWnd, SB_VERT, yPos, TRUE);
            ScrollWindow(hWnd, 0, si.nPos - yPos, NULL, NULL);
            UpdateWindow(hWnd);
        }
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG: return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}