#pragma once

#include <iostream>
#include <string>
#include <windows.h>
#include <winnt.h>

inline const std::string RunCommand(const std::string &command) {
  HANDLE hReadPipe, hWritePipe;
  SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

  if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0)) {
    std::cerr << "CreatePipe failed.\n";
    return "";
  }

  STARTUPINFOA si = {sizeof(STARTUPINFOA)};
  PROCESS_INFORMATION pi;
  si.dwFlags |= STARTF_USESTDHANDLES;
  si.hStdOutput = hWritePipe;
  si.hStdError = hWritePipe;
  si.hStdInput = NULL;

  std::string cmdLine = "cmd /C " + command;
  char *cmd = _strdup(cmdLine.c_str());

  if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL,
                      &si, &pi)) {
    std::cerr << "CreateProcess failed.\n";
    CloseHandle(hReadPipe);
    CloseHandle(hWritePipe);
    free(cmd);
    return "";
  }

  CloseHandle(hWritePipe); // Close write end in parent

  char buffer[4096];
  DWORD bytesRead;
  std::string output;

  while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
    if (bytesRead == 0)
      break;
    buffer[bytesRead] = '\0';
    output += buffer;
  }

  CloseHandle(hReadPipe);
  WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  free(cmd);

  return output;
}