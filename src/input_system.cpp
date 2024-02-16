#include "input_system.hpp"

class Input {
private:
	std::set<int> active_keys;
public:
	void press(int key) {
		this->active_keys.insert(key);
	}
	void release(int key) {
		this->active_keys.erase(key);
	}
	std::set<int> get_keys() {
		return this->active_keys;
	}
};