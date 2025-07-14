#pragma once

#include "..\proto\record_part.pb.h"
#include "..\proto\recordlist.pb.h"
#include "codeconvert.hpp"
#include "data.h"
#include <fstream>
#include <vector>

inline std::vector<ManagerData::User_Data>
Get_User_Datas(const std::string &path) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  record::Record loaded;
  std::ifstream ifs(path, std::ios::binary);
  loaded.ParseFromIstream(&ifs);
  const record::Information &information = loaded.information();
  const record::Information_Inner &inner = information.inner();
  std::vector<record::User_Data> user_datas;
  for (int i = 0; i < inner.player_size(); i++) {
    const record::Player_Data &player = inner.player(i);
    if (player.has_userdata()) {
      user_datas.emplace_back(player.userdata());
    }
  }
  std::vector<ManagerData::User_Data> result;
  for (const auto &i : user_datas) {
    result.emplace_back(ManagerData::User_Data{
        i.id(), OctalStringToUTF8(i.nickname()), i.level(), i.avatar_url()});
  }
  google::protobuf::ShutdownProtobufLibrary();
  return result;
}

inline std::vector<ManagerData::Player_Record>
Get_Record_Datas(const std::string &path) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  recordlist::RecordList loaded;
  std::ifstream ifs(path, std::ios::binary);
  loaded.ParseFromIstream(&ifs);
  std::vector<ManagerData::Player_Record> result;
  for (const auto &i : loaded.item()) {
    const recordlist::Player_Info playerinfo = i.info();
    const recordlist::Dynamic_Avatar_Info dynamic_avatar =
        playerinfo.dynamic_avatar();
    ManagerData::Dynamic_Avatar_Info avatar_info{dynamic_avatar.avatar_number(),
                                                 dynamic_avatar.avatar_id()};
    std::vector<int> read_ninja_number;
    for (auto &i : playerinfo.ninja_number())
      read_ninja_number.emplace_back(i);
    std::vector<int> read_fashion_number;
    for (auto &i : playerinfo.fashion_number())
      read_fashion_number.emplace_back(i);
    ManagerData::Player_Info iteminfo{OctalStringToUTF8(playerinfo.nickname()),
                                      playerinfo.avatar_url(),
                                      playerinfo.area_code(),
                                      OctalStringToUTF8(playerinfo.area_name()),
                                      read_ninja_number,
                                      playerinfo.rank(),
                                      read_fashion_number,
                                      avatar_info};
    ManagerData::Player_Record item{i.timestamp(), i.statu(), i.datetime(),
                                    iteminfo, i.is_temp()};
    result.emplace_back(item);
  }
  google::protobuf::ShutdownProtobufLibrary();
  return result;
}
