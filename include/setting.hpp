#pragma once

#include "data.h"
#include "json.hpp"
#include <fstream>
#include <format>
#include "path.hpp"

namespace Setting{
    inline Data GetData(){
        std::ifstream rd(std::format("{}settings.json",FileManager::Get_Local_Root_Path()));
        if(!rd.is_open()){
            return Data{std::format("{}export",FileManager::Get_Local_Root_Path())};
        }
        nlohmann::ordered_json json;
        rd>>json;
        rd.close();
        return Data{json["Export_Path"]};
    }
    inline bool WriteData(const Data& data){
        std::ofstream wt(std::format("{}settings.json",FileManager::Get_Local_Root_Path()));
        if(!wt.is_open())
          return false;
        nlohmann::ordered_json json;
        json["Export_Path"]=data.Export_Path;
        wt<<json;
        wt.close();
        return true;
    }
}