#pragma once
#include <string>
#include <shlobj.h>

namespace FileManager {

const std::string ADB_Path = ".\\bin\\adb.exe";

inline std::string Get_Local_Root_Path(){
  char path[MAX_PATH];
  SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path);
  return std::string(path) + "\\NarutoRecordManager\\";
}

inline std::string Get_TEMP_Path() {
  char path[MAX_PATH];
  SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path);
  return std::string(path) + "\\NarutoRecordManager\\temp\\";
}

}