#include "json.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <unistd.h>
#include <vector>

#include "Container.h"

using json = nlohmann::json;

// write tests in bash to make sure everything works as expected

std::vector<std::string> split(const std::string &line);
std::string erase_all(const std::string &s, const char charToRemove);

int main(int argc, char *argv[]) {
  std::map<std::string, std::string> flags;
  std::map<std::string, std::string> conf;
  std::map<std::string, Container> containers;
  std::vector<std::string> allArgs(argv, argv + argc);
  for (unsigned int i = 0; i < allArgs.size(); ++i) {
    std::string cur = allArgs.at(i);
    // check to make sure the flag and commands are valid
    if (cur[0] == '-')
      flags.insert(std::pair<std::string, std::string>(
          cur.substr(1, cur.size()),
          i + 1 == allArgs.size() ? "" : allArgs.at(i + 1)));
  }
  std::string command = allArgs.size() > 1 ? allArgs.at(1) : "";
  // check if there is a config file present.  if not, throw an error and
  // provide a link to documentation. could put a placeholder config file
  std::string confDirectory = getenv("XDG_CONFIG_HOME") == NULL
                                  ? (std::string(getenv("HOME")) + "/.config")
                                  : getenv("XDG_CONFIG_HOME");
  std::ifstream f(confDirectory + "/sailor/sailor.json");
  // handle parse exception
  json config = json::parse(f);
  json defaults = config["defaults"];
  for (auto it = defaults.begin(); it != defaults.end(); ++it)
    conf.insert(std::pair<std::string, std::string>(
        it.key(), erase_all(it.value(), '"')));
  json jContainers = config["containers"];
  for (auto it = jContainers.begin(); it != jContainers.end(); ++it) {
    auto curContainer = it.value();
    Container c(conf);
    for (auto cIt = curContainer.begin(); cIt != curContainer.end(); ++cIt) {
      std::string key = cIt.key();
      std::string value = cIt.value();
      if (key == "container_name")
        c.setName(value);
      else if (key == "compose_dir")
        c.setComposeDir(value);
      else if (key == "compose_file_name")
        c.setComposeFilename(value);
      else if (key == "dir_name")
        c.setDirName(value);
    }
    // throw error if no name is found (c.getName() == "")
    containers.insert(std::pair<std::string, Container>(c.getName(), c));
  }

  // sailor update -c <container_name>
  if (command == "update") {
    std::cout << "Updating " << flags.at("c") << "\n";
  }

  else if (command == "debug") {
    std::cout << "Debug output: \n";
    std::cout << "\nConf: \n";
    for (std::map<std::string, std::string>::iterator it = conf.begin();
         it != conf.end(); ++it)
      std::cout << it->first << " : " << it->second << "\n";
    std::cout << "\nContainers: \n";
    for (std::map<std::string, Container>::iterator it = containers.begin();
         it != containers.end(); ++it) {
      Container c = it->second;
      std::cout << "name: " << c.getName() << "\n";
      std::cout << "compose_dir: " << c.getComposeDir() << "\n";
      std::cout << "compose_filename: " << c.getComposeFilename() << "\n";
      std::cout << "dir_name: " << c.getDirName() << "\n";
      std::cout << "path: " << c.getPath() << "\n\n";
    }
  }
}

std::string erase_all(const std::string &s, const char charToRemove) {
  std::string result;
  for (char c : s)
    if (c != charToRemove)
      result += c;
  return result;
}

std::vector<std::string> split(const std::string &line) {
  int equalsLocation = line.find('=');
  std::vector<std::string> v = {line.substr(0, equalsLocation),
                                line.substr(equalsLocation + 1, line.size())};
  return v;
}