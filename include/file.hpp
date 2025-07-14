#pragma once

#include <direct.h>
#include <filesystem>
#include <io.h>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

const std::string ADB_Path = ".\\bin\\adb.exe";
const std::string TEMP_Path = ".\\temp\\";
const std::string EXPORT_Path = ".\\export\\";

namespace FileManager {

// for each device
struct FileList {
public:
  std::vector<std::string> record;
  std::vector<std::string> recordlist;
  std::string device_id;
  FileList(const std::vector<std::string> &files, const std::string &id) {
    for (const auto &i : files) {
      if (recordmatch(i))
        record.emplace_back(i);
      else if (recordlistmatch(i))
        recordlist.emplace_back(i);
    }
    device_id = id;
  }
  FileList() { device_id = "no device"; }

  const bool recordmatch(const std::string &filename) {
    std::regex pattern(R"(^\d{4}_\d{2}_\d{2}T\d{2}_\d{2}_\d{2}$)");
    return std::regex_match(filename, pattern);
  }
  const bool recordlistmatch(const std::string &filename) {
    std::regex pattern(R"(^LocalRecordList_JueDou_\d+$)");
    return std::regex_match(filename, pattern);
  }
};

inline const bool is_local_file_exist(const std::string &path) {
  return (_access(path.c_str(), F_OK) != -1);
}

inline const void local_system_init() {
  if (_access(TEMP_Path.c_str(), F_OK) == -1)
    _mkdir(TEMP_Path.c_str());
  if (_access(EXPORT_Path.c_str(), F_OK) == -1)
    _mkdir(EXPORT_Path.c_str());
}

inline const void local_system_clear() {
  for (const auto &entry : std::filesystem::directory_iterator(TEMP_Path))
    std::filesystem::remove_all(entry.path());
}

inline FileList& GetListByDeviceID(std::vector<FileList> &lists,
                                  const std::string &id) {
  for (auto &item : lists) {
    if (item.device_id == id)
      return item;
  }
  throw std::runtime_error("Device_ID Not Found");
}

} // namespace FileManager
