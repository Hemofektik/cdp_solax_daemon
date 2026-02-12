#pragma once
#include "RestService.h"
#include "solax/SerialAdapter.h"

namespace solax
{

struct Config
{
    RestService::Config rest;
    solax::SerialAdapter::Config serialAdapter;
};

Config loadConfig(const std::string& configPath);

}