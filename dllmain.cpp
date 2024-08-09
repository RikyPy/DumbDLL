#include "pch.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

// Helper function to get the current process name
std::string GetCurrentProcessName() {
    char processName[MAX_PATH];
    GetModuleFileNameA(NULL, processName, MAX_PATH);
    std::string fullPath = processName;
    std::size_t pos = fullPath.find_last_of("\\/");
    if (pos != std::string::npos) {
        return fullPath.substr(pos + 1);
    }
    return fullPath;
}

// Function to write text to the Notepad window
void WriteTextToNotepad(const std::string& text) {
    HWND hwndNotepad = FindWindowA("Notepad", NULL);
    if (hwndNotepad) {
        SetForegroundWindow(hwndNotepad);
        for (char c : text) {
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = 0;
            input.ki.wScan = c;
            input.ki.dwFlags = KEYEVENTF_UNICODE;
            input.ki.time = 0;
            input.ki.dwExtraInfo = 0;
            SendInput(1, &input, sizeof(INPUT));
            Sleep(50);
        }
    }
    else {
        std::cerr << "Notepad not found!" << std::endl;
    }
}

// Function to start a console for user input
void StartConsole() {
    AllocConsole();

    // Redirect standard input/output streams to the console
    FILE* consoleInput;
    FILE* consoleOutput;
    FILE* consoleError;

    freopen_s(&consoleInput, "CONIN$", "r", stdin);
    freopen_s(&consoleOutput, "CONOUT$", "w", stdout);
    freopen_s(&consoleError, "CONOUT$", "w", stderr);

    std::cout << "Console for entering text to write to Notepad:\n";
    std::cout << "Type 'exit' to quit the console.\n";
}

// Function to handle user input in the console
void HandleUserInput() {
    std::string userInput;
    while (true) {
        std::cout << "Enter text to write (or type 'exit' to quit): ";
        std::getline(std::cin, userInput);
        if (userInput == "exit") {
            std::cout << "Exiting console..." << std::endl;
            break;
        }
        std::cout << "Writing text to Notepad: " << userInput << std::endl;
        WriteTextToNotepad(userInput);
    }

    FreeConsole(); // Cleanup the console
}

// Function to ensure the message box appears in the foreground
void ShowMessageBoxInForeground(const std::string& message) {
    // Create a hidden window to use for showing the message box
    HWND hwndMessageBox = CreateWindowA("STATIC", "DLL Injected", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (hwndMessageBox) {
        SetForegroundWindow(hwndMessageBox); // Bring the hidden window to the foreground
        MessageBoxA(hwndMessageBox, message.c_str(), "DLL Injected", MB_OK | MB_ICONINFORMATION);
        DestroyWindow(hwndMessageBox); // Cleanup the hidden window after displaying the message box
    }
}

// Main function of the DLL (DllMain)
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        // Start the console immediately
        StartConsole();

        // Log and show the message box
        std::string processName = GetCurrentProcessName();
        std::string message = "Hi from " + processName;
        std::cout << "Showing message box: " << message << std::endl;
        ShowMessageBoxInForeground(message);

        // Wait a short moment to ensure Notepad is ready
        Sleep(1000);

        // Write "Hi!" to the Notepad window
        std::cout << "Writing 'Hi!' to Notepad" << std::endl;
        WriteTextToNotepad("Hi!");

        // Start the console input handler
        std::thread consoleThread(HandleUserInput);
        consoleThread.detach(); // Allow the console to run in the background
    }

    return TRUE;
}
