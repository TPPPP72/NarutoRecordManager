#pragma once

#include "data.h"
#include "json.hpp"
#include <fstream>

namespace Setting{
    inline Data GetData(){
        std::ifstream rd("settings.json");
        if(!rd.is_open()){
            return Data{".\\export\\"};
        }
        nlohmann::ordered_json json;
        rd>>json;
        rd.close();
        return Data{json["Export_Path"]};
    }
    inline bool WriteData(const Data& data){
        std::ofstream wt("settings.json");
        if(!wt.is_open())
          return false;
        nlohmann::ordered_json json;
        json["Export_Path"]=data.Export_Path;
        wt<<json;
        wt.close();
        return true;
    }
}