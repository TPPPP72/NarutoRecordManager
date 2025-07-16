#pragma once

#include "..\proto\recordlist.pb.h"
#include "data.h"
#include <fstream>

namespace hexwriter {
inline bool
Write_Record_List(const std::vector<ManagerData::Player_Record> &list,
                  const std::string &path) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  recordlist::RecordList output;
  for (const auto& item : list) {
    auto* record_item = output.add_item();
    record_item->set_timestamp(item.timestamp);
    record_item->set_statu(item.statu);
    record_item->set_datetime(item.datetime);
    record_item->set_is_temp(item.is_temp);
    auto* info = record_item->mutable_info();
    info->set_nickname(item.info.nickname);
    info->set_avatar_url(item.info.avatar_url);
    info->set_area_code(item.info.area_code);
    info->set_area_name(item.info.area_name);
    info->set_rank(item.info.rank);
    for (int ninja : item.info.ninja_number) {
      info->add_ninja_number(ninja);
    }
    for (int fashion : item.info.fashion_number) {
      info->add_fashion_number(fashion);
    }
    auto* avatar = info->mutable_dynamic_avatar();
    avatar->set_avatar_number(item.info.dynamic_avatar.avatar_number);
    avatar->set_avatar_id(item.info.dynamic_avatar.avatar_id);
  }

  std::ofstream ofs(path, std::ios::binary);
  if (!output.SerializeToOstream(&ofs)) {
    return false;
  }
  google::protobuf::ShutdownProtobufLibrary();
  return true;
}
} // namespace hexwriter