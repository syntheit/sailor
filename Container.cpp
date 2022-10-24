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

// should all commands be run as sudo?  should this be an option in the config?

void Container::start() const {
  std::string command = "sudo docker-compose -f " + getPath() + " up -d";
  system(command.c_str());
}

void Container::stop() const {
  std::string command = "sudo docker-compose -f " + getPath() + " stop";
  system(command.c_str());
}

void Container::restart() const {
  stop();
  start();
}

void Container::pull() const {
  std::string command = "sudo docker-compose -f " + getPath() + " pull";
  system(command.c_str());
}

void Container::update() const {
  stop();
  pull();
  start();
}