#pragma once

#include "..\proto\record_part.pb.h"
#include "..\proto\recordlist.pb.h"
#include "tools.hpp"
#include "data.h"
#include <fstream>
#include <vector>

namespace hexreader {
inline ManagerData::Record Get_Record(const std::string &path) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  record::Record loaded;
  std::ifstream ifs(path, std::ios::binary);
  loaded.ParseFromIstream(&ifs);
  // information
  std::vector<ManagerData::Player_Data> players;
  for (const auto &item : loaded.information().inner().player()) {
    ManagerData::Game game;
    for (const auto &ninja_item : item.userdata().game().ninja()) {
      game.ninja.emplace_back(ManagerData::Ninja{
          ninja_item.basic().ninja_id(),
          ninja_item.basic().ninja_resource().fashion_resource()});
    }
    ManagerData::Player_Data player{
        item.pos1(),
        item.pos2(),
        {item.userdata().user_id(),
         OctalStringToUTF8(item.userdata().nickname()),
         item.userdata().level(),
         item.userdata().score(),
         game,
         item.userdata().avatar_url(),
         OctalStringToUTF8(item.userdata().area_name()),
         item.userdata().private_().area_code(),
         item.userdata().rank(),
         {item.userdata().dynamic_avatar().avatar_number(),
          item.userdata().dynamic_avatar().avatar_id()}}};
    players.emplace_back(player);
  }
  return {players, loaded.settle_information().inner().statu()};
}

inline std::vector<ManagerData::Player_Record>
Get_Record_List(const std::string &path) {
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

} // namespace hexreader