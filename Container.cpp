#include "Container.h"

Container::Container(std::map<std::string, std::string> &defaults) {
  setDefaults(defaults);
};

void Container::setName(std::string _name) {
  if (dir_name.empty() && !flat)
    dir_name = _name;
  name = _name;
}

void Container::setDefaults(std::map<std::string, std::string> &defaults) {
  setComposeDir(defaults.at("compose_dir"));
  setComposeFilename(defaults.at("compose_file_name"));
  flat = defaults.count("flat") == 1 && defaults.at("flat") == "true" ? true
                                                                      : false;
}

std::string Container::getPath() const {
  return flat ? compose_dir + "/" + compose_file_name
              : compose_dir + "/" + dir_name + "/" + compose_file_name;
}