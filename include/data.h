#pragma once

#include <fstream>
#include <string>
#include <vector>

namespace ManagerData {

struct User_Data {
  int id;
  std::string nickname;
  int level;
  std::string avatar_url;
  void outfile(std::string name) {
    std::ofstream o(name);
    o << "id:" << id << std::endl;
    o << "nickname:" << nickname << std::endl;
    o << "level:" << level << std::endl;
    o << "avatar_url" << avatar_url << std::endl;
    o.close();
  };
};

struct Dynamic_Avatar_Info {
  int avatar_number;
  int avatar_id;
};

struct Player_Info {
  std::string nickname;
  std::string avatar_url;
  int area_code;
  std::string area_name;
  std::vector<int> ninja_number;
  int rank;
  std::vector<int> fashion_number;
  Dynamic_Avatar_Info dynamic_avatar;
};

struct Player_Record {
  long long timestamp;
  int statu = 3;
  std::string datetime;
  Player_Info info;
  int is_temp = false;
  void outfile(std::string name) {
    std::ofstream o(name);
    o << "timestamp:" << timestamp << std::endl;
    o << "statu:" << statu << std::endl;
    o << "datetime:" << datetime << std::endl;
    o << "Player_Info:" << std::endl;
    o<<"\tnickname:"<<info.nickname<<std::endl;
    o<<"\tavatar_url:"<<info.avatar_url<<std::endl;
    o<<"\tarea_code:"<<info.area_code<<std::endl;
    o<<"\tarea_name:"<<info.area_name<<std::endl;
    o<<"\tninja_number:";
    for(auto &i:info.ninja_number)
      o<<i<<' ';
    o<<std::endl;
    o<<"\trank:"<<info.rank<<std::endl;
    o<<"\tfashion_number:";
    for(auto &i:info.fashion_number)
      o<<i<<' ';
    o<<std::endl;
    o<<"\tdynamic_avatar:"<<std::endl;
    o<<"\t\tavatar_number:" <<info.dynamic_avatar.avatar_number<<std::endl;
    o<<"\t\tavatar_id:" << info.dynamic_avatar.avatar_id<<std::endl;
    o << "is_temp:" << is_temp << std::endl;
    o.close();
  };
};

} // namespace ManagerData