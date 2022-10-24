#ifndef __Container_h_
#define __Container_h_

#include <map>
#include <string>

#include <iostream>

class Container {
public:
  Container(std::map<std::string, std::string> &defaults);

  std::string getName() const { return name; };
  std::string getComposeDir() const { return compose_dir; };
  std::string getComposeFilename() const { return compose_file_name; };
  std::string getDirName() const { return dir_name; };
  std::string getPath() const;

  void setName(std::string _name);
  void setComposeDir(std::string _compose_dir) { compose_dir = _compose_dir; };
  void setComposeFilename(std::string _compose_file_name) {
    compose_file_name = _compose_file_name;
  };
  void setDirName(std::string _dir_name) { dir_name = _dir_name; };

  void start() const;
  void stop() const;
  void restart() const;
  void pull() const;
  void update() const;

private:
  std::string name;
  std::string compose_dir;
  std::string compose_file_name;
  std::string dir_name;

  bool flat = false;

  void setDefaults(std::map<std::string, std::string> &defaults);
};

#endif