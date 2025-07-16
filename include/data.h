#pragma once

#include <fstream>
#include <string>
#include <vector>

namespace ManagerData {

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
};

struct Ninja_Resource {
  int fashion_resource;
};

struct Ninja_Basic {
  int ninja_id;
  Ninja_Resource ninja_resource;
};

struct Ninja {
  Ninja_Basic basic;
};

struct Game {
  std::vector<Ninja> ninja;
};

struct User_Data_Private {
  int area_code;
};

struct User_Data {
  long long user_id;
  std::string nickname;
  int level;
  int score;
  Game game;
  std::string avatar_url;
  std::string area_name;
  User_Data_Private private_info;
  int rank;
  Dynamic_Avatar_Info dynamic_avatar;
};

struct Player_Data {
  int pos1, pos2;
  User_Data userdata;
};

struct Information_Inner {
  std::vector<Player_Data> players;
};

struct Information {
  Information_Inner inner;
};

struct Settle_Information_Inner {
  int statu = 3;
};

struct Settle_Information {
  Settle_Information_Inner inner;
};

struct Record {
  Information information;
  Settle_Information settle_information;
};

} // namespace ManagerData

namespace Setting {
struct Data {
  std::string Export_Path;
};
} // namespace Setting