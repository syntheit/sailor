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

#include "Container.hpp"

using json = nlohmann::json;

// write tests in bash to make sure everything works as expected

std::vector<std::string> split(const std::string &line);
std::string erase_all(const std::string &s, const char charToRemove);
void containerAction(const std::string &containerName,
                     const std::string &actionName, const std::string &action,
                     int containerNameCount, Container c);
bool isValidContainerCommand(std::string &command);

int main(int argc, char *argv[]) {

  std::string version = "v0.1.4";
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
        c.setComposeDir(value); // make sure that directory exists
      else if (key == "compose_file_name")
        c.setComposeFilename(value); // check that file exists
      else if (key == "dir_name")
        c.setDirName(value);
    }
    // throw error if no name is found (c.getName() == "")
    containers.insert(std::pair<std::string, Container>(c.getName(), c));
  }

  // if there's a container and only one argument, assume the arg is the
  // container name

  // sailor update -c <container_name>

  // sailor ls to list all containers

  // command to add cron job to schedule actions
  // for all or individual containers

  // option to enable logging to a file
  // (is there an XDG standard on where to log, like /var/log/something)

  // add ability to check a container's status

  // add docs on how to install sailor using the binary
  // add check-update command to update sailor

  // add quick-update command to update a container only if there's a newer
  // version available to pull (this might mean going into the docker-compose
  // files themeslves and checking the versions of those images)

  // add version command

  // add ls -a option to show full path (and status?)
  if (command == "ls" && flags.size() == 0) {
    std::map<std::string, Container>::iterator it;
    for (it = containers.begin(); it != containers.end(); ++it)
      std::cout << it->first << "\n";
  } else if (command == "ls" && flags.count("a") == 1) {
    std::map<std::string, Container>::iterator it;
    for (it = containers.begin(); it != containers.end(); ++it)
      std::cout << it->first << " : " << it->second.getPath() << "\n";
  } else if (command == "ls" && (flags.size() > 1 || flags.count("a") != 1)) {
    std::cerr << "Unexpected argument given\n";
  } else if (command == "--version" || command == "--v") {
    std::cout << "Sailor " << version << "\n";
  } else if (command == "update" && flags.count("c") == 1) {
    std::string containerName = flags.at("c");
    containerAction(containerName, "Updating", "update",
                    containers.count(containerName),
                    containers.at(containerName));
  } else if (command == "start" && flags.count("c") == 1) {
    std::string containerName = flags.at("c");
    containerAction(containerName, "Starting", "start",
                    containers.count(containerName),
                    containers.at(containerName));
  } else if (command == "stop" && flags.count("c") == 1) {
    std::string containerName = flags.at("c");
    containerAction(containerName, "Stopping", "stop",
                    containers.count(containerName),
                    containers.at(containerName));
  } else if (command == "restart" && flags.count("c") == 1) {
    std::string containerName = flags.at("c");
    containerAction(containerName, "Restarting", "restart",
                    containers.count(containerName),
                    containers.at(containerName));
  } else if (command == "pull" && flags.count("c") == 1) {
    // the container probably needs to be off before a pull
    std::string containerName = flags.at("c");
    containerAction(containerName, "Pulling", "pull",
                    containers.count(containerName),
                    containers.at(containerName));
  } else if (command == "update" && flags.count("a") == 1) {
    // the container probably needs to be off before a pull
    std::cout << "Updating all " << containers.size() << " containers\n";
    std::map<std::string, Container>::iterator it;
    for (it = containers.begin(); it != containers.end(); ++it) {
      std::string containerName = it->first;
      containerAction(containerName, "Updating", "update",
                      containers.count(containerName), it->second);
    }
  } else if (command == "start" && flags.count("a") == 1) {
    std::cout << "Starting all " << containers.size() << " containers\n";
    std::map<std::string, Container>::iterator it;
    for (it = containers.begin(); it != containers.end(); ++it) {
      std::string containerName = it->first;
      containerAction(containerName, "Starting", "start",
                      containers.count(containerName), it->second);
    }
  } else if (command == "stop" && flags.count("a") == 1) {
    std::cout << "Stopping all " << containers.size() << " containers\n";
    std::map<std::string, Container>::iterator it;
    for (it = containers.begin(); it != containers.end(); ++it) {
      std::string containerName = it->first;
      containerAction(containerName, "Stopping", "stop",
                      containers.count(containerName), it->second);
    }
  } else if (command == "restart" && flags.count("a") == 1) {
    std::cout << "Restarting all " << containers.size() << " containers\n";
    std::map<std::string, Container>::iterator it;
    for (it = containers.begin(); it != containers.end(); ++it) {
      std::string containerName = it->first;
      containerAction(containerName, "Restarting", "restart",
                      containers.count(containerName), it->second);
    }
  } else if (command == "pull" && flags.count("a") == 1) {
    std::cout << "Pulling all " << containers.size() << " containers\n";
    std::map<std::string, Container>::iterator it;
    for (it = containers.begin(); it != containers.end(); ++it) {
      std::string containerName = it->first;
      containerAction(containerName, "Pulling", "pull",
                      containers.count(containerName), it->second);
    }
  } else if (isValidContainerCommand(command) && flags.count("c") == 0 &&
             flags.count("a") == 0) {
    std::cerr << "No container name was provided.  Please specify the "
                 "container name with -c <container>\n";
  } else if (command == "debug") {
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

void containerAction(const std::string &containerName,
                     const std::string &actionName, const std::string &action,
                     int containerNameCount, Container c) {
  if (containerNameCount == 1) {
    std::cout << actionName << " " << c.getName() << "\n";
    c.func(action);
  } else if (containerNameCount <= 0)
    std::cout << "Container " << containerName
              << " is not configured.  Check sailor.json";
  else if (containerNameCount > 1)
    std::cout << "Container name " << containerName
              << " has duplicates.  Check sailor.json";
}

bool isValidContainerCommand(std::string &command) {
  return command == "update" || command == "start" || command == "stop" ||
         command == "restart" || command == "pull";
}