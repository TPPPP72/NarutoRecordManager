#pragma once

#include "file.hpp"
#include "winruntime.hpp"
#include <sstream>
#include <string>
#include <vector>
#include <format>

namespace ADB {
inline const std::string
RunADBCommand(const std::string &command,
              std::string SelectedDevice = "no selection") {
  if (SelectedDevice == "no selection")
    return RunCommand(std::format("{} {}", ADB_Path, command));
  return RunCommand(
      std::format("{} -s {} {}", ADB_Path, SelectedDevice, command));
}

inline const std::vector<std::string> GetDevices() {
  std::string output = RunADBCommand("devices");
  std::vector<std::string> devices;
  std::istringstream stream(output);
  std::string line;
  bool start = false;

  while (std::getline(stream, line)) {
    if (!start) {
      if (line.find("List of devices") != std::string::npos) {
        start = true;
      }
      continue;
    }

    if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos)
      continue;

    std::istringstream linestream(line);
    std::string device;
    linestream >> device;

    std::string status;
    linestream >> status;
    if (status == "device") {
      devices.emplace_back(device);
    }
  }
  return devices;
}

inline const std::vector<std::string>
GetRecordFiles(std::string SelectedDevice = "no selection") {
  std::vector<std::string> files;
  std::string command =
      "shell ls "
      "/storage/emulated/0/Android/data/com.tencent.KiHan/files/LocalRecord";
  std::string output = RunADBCommand(command, SelectedDevice);
  std::istringstream stream(output);
  std::string line;
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    std::string file;
    linestream >> file;
    files.emplace_back(file);
  }
  return files;
}

inline const bool PullRemoteFile(const FileManager::FileList &list,
                                 const std::string &file,
                                 const std::string &path) {
  bool flag = false;
  for (const auto &i : list.recordlist) {
    if (i == file) {
      flag = true;
      break;
    }
  }
  for (const auto &i : list.record) {
    if (i == file) {
      flag = true;
      break;
    }
  }
  if (!flag)
    return false;
  std::string result =
      RunADBCommand(std::format("pull "
                                "/storage/emulated/0/Android/data/"
                                "com.tencent.KiHan/files/LocalRecord/{} {}",
                                file, path),
                    list.device_id);
  if (result.find("1 file pulled") != std::string::npos)
    return true;
  return false;
}

inline const bool PushRemoteFile(FileManager::FileList &list,
                                 const std::string &source_id,
                                 const std::string &file) {
  std::string localpath =
      FileManager::Get_Local_Device_TEMP_Path(source_id) + file;
  if (!FileManager::is_local_file_exist(localpath))
    return false;
  std::string result =
      RunADBCommand(std::format("push {} "
                                "/storage/emulated/0/Android/data/"
                                "com.tencent.KiHan/files/LocalRecord/{}",
                                localpath, file),
                    list.device_id);
  if (result.find("1 file pushed") != std::string::npos) {
    if (list.recordlistmatch(file))
      list.recordlist.emplace_back(file);
    else if (list.recordmatch(file))
      list.record.emplace_back(file);
    return true;
  }
  return false;
}

inline const bool PushRemoteFile_Full(FileManager::FileList &list,
                                      const std::string &path) {
  std::string file = path.substr(path.rfind("\\") + 1);
  if (!FileManager::is_local_file_exist(path))
    return false;

  std::string result =
      RunADBCommand(std::format("push {} "
                                "/storage/emulated/0/Android/data/"
                                "com.tencent.KiHan/files/LocalRecord/{}",
                                path, file),
                    list.device_id);
  if (result.find("1 file pushed") != std::string::npos) {
    if (list.recordlistmatch(file))
      list.recordlist.emplace_back(file);
    else if (list.recordmatch(file))
      list.record.emplace_back(file);
    return true;
  }
  return false;
}

inline const bool DeleteRemoteFile(FileManager::FileList &list,
                                   const std::string &file) {

  std::string result =
      RunADBCommand(std::format("shell rm "
                                "/storage/emulated/0/Android/data/"
                                "com.tencent.KiHan/files/LocalRecord/{}",
                                file),
                    list.device_id);
  if (result.length() > 0)
    return false;
  std::erase(list.recordlist, file);
  std::erase(list.record, file);
  return true;
}
} // namespace ADB
