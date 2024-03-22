#include "common.hpp"

class Config {
private:
	json config;
public:
	void load(std::string file_name="config.json") {
		std::ifstream file(data_path() + "/" + file_name);
		config = json::parse(file);
	}
};