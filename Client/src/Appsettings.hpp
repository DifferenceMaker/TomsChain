#include <iostream>
#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

struct AppSettings {
	AppSettings() {
		// Load Configuration file for Client
		try {
			YAML::Node config = YAML::LoadFile("config/appsettings.yaml");
			if (config["node"] && config["node"]["type"]) {
				std::cout << "Node Type: " << config["node"]["type"].as<std::string>() << std::endl;
				node.type = config["node"]["type"].as<std::string>();
			}
			else {
				throw std::runtime_error("Node type not found in the configuration file.");
			}
		}
		catch (const YAML::Exception& e) {
			std::cerr << "Error loading YAML file: " << e.what() << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
	};

	struct NodeSettings {
		std::string type;
	} node;
};