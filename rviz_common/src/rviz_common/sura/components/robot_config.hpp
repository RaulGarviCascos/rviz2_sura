#pragma once 
#include <QString>
#include <rviz_common/config.hpp>

struct RobotConfig {
  QString ip;
  QString user;
  QString password;
  QString robot_name;
  QString file_path;

  void save(rviz_common::Config config) const
  {
    
    config.mapSetValue("RobotName", robot_name);
    config.mapSetValue("Ip",ip);
    config.mapSetValue("UserName",user);
    config.mapSetValue("UserPass",password);
    config.mapSetValue("FilePath",file_path);
  }

  void load(const rviz_common::Config &config)
  {
    QString temp;
    if (config.mapGetString("RobotName", &temp)) robot_name = temp;
    if (config.mapGetString("Ip",        &temp)) ip         = temp;
    if (config.mapGetString("UserName",  &temp)) user       = temp;
    if (config.mapGetString("UserPass",  &temp)) password   = temp;
    if (config.mapGetString("FilePath",  &temp)) file_path   = temp;
  }
};