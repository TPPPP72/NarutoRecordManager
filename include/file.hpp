#pragma once

#include "data.h"
#include "json.hpp"
#include "path.hpp"
#include "setting.hpp"
#include "tools.hpp"
#include <direct.h>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>
#include <shlobj.h>

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
  if (_access(Get_Local_Root_Path().c_str(), F_OK) == -1)
    _mkdir(Get_Local_Root_Path().c_str());
  if (_access(Get_TEMP_Path().c_str(), F_OK) == -1)
    _mkdir(Get_TEMP_Path().c_str());
  if (_access(utf8_to_gbk(Setting::GetData().Export_Path).c_str(), F_OK) == -1)
    _mkdir(utf8_to_gbk(Setting::GetData().Export_Path).c_str());
}

inline const void local_system_clear() {
  for (const auto &entry : std::filesystem::directory_iterator(Get_TEMP_Path()))
    std::filesystem::remove_all(entry.path());
}

inline std::string Get_Local_Device_TEMP_Path(const std::string &device_id) {
  std::string new_id = device_id;
  for (auto &i : new_id)
    if (i == ':')
      i = '_';
  if (_access((Get_TEMP_Path() + new_id).c_str(), F_OK) == -1)
    _mkdir((Get_TEMP_Path() + new_id).c_str());
  return Get_TEMP_Path() + new_id + "\\";
}

inline bool Is_Local_Device_MAP_TEMP_Exists(const std::string &device_id) {
  return (_access((Get_Local_Device_TEMP_Path(device_id) + "map").c_str(),
                  F_OK) != -1);
}

inline std::string Get_Local_Device_MAP_TEMP(const std::string &device_id) {
  return Get_Local_Device_TEMP_Path(device_id) + "map";
}

struct Data {
  std::string p1;
  std::string p2;
  std::string statu;
  std::string belong;
};

struct FileData {
  std::string file;
  Data data;
};

inline void to_json(nlohmann::json &j, const Data &d) {
  j = nlohmann::json{
      {"1P", d.p1}, {"2P", d.p2}, {"statu", d.statu}, {"belong", d.belong}};
}

inline void from_json(const nlohmann::json &j, Data &d) {
  j.at("1P").get_to(d.p1);
  j.at("2P").get_to(d.p2);
  j.at("statu").get_to(d.statu);
  j.at("belong").get_to(d.belong);
}

inline void to_json(nlohmann::json &j, const FileData &r) {
  j.emplace("file", r.file);
  j.emplace("data", r.data);
}

inline void from_json(const nlohmann::json &j, FileData &r) {
  j.at("file").get_to(r.file);
  j.at("data").get_to(r.data);
}

inline nlohmann::json Get_File_List_Json(const std::string &device_id) {
  std::ifstream rd(Get_Local_Device_MAP_TEMP(device_id));
  nlohmann::json j;
  rd >> j;
  rd.close();
  return j;
}

inline bool Write_File_List_Json(const nlohmann::json &j,
                                 const std::string &device_id) {
  std::ofstream rd(Get_Local_Device_MAP_TEMP(device_id));
  if (!rd.is_open())
    return false;
  rd << j;
  rd.close();
  return true;
}

inline const void local_system_clear(const std::string &device_id) {
  for (const auto &entry : std::filesystem::directory_iterator(
           Get_Local_Device_TEMP_Path(device_id)))
    std::filesystem::remove_all(entry.path());
}

inline FileList &GetListByDeviceID(std::vector<FileList> &lists,
                                   const std::string &id) {
  for (auto &item : lists) {
    if (item.device_id == id)
      return item;
  }
  throw std::runtime_error("Device_ID Not Found");
}

inline bool Copy(const FileManager::FileList &list, const std::string &file,
                     const std::string &path) {
  std::string src = Get_Local_Device_TEMP_Path(list.device_id) + file;
  std::string dst = path + file;
  bool result = CopyFileA(src.c_str(), dst.c_str(), false);
  return result != 0;
}

} // namespace FileManager
