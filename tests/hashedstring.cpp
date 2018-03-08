#include <iostream>
#include "third_party/entt/entt.hpp"



int main(int argc, char *argv[]) {
	if (argc > 1) {
		std::string arg = argv[1];
		std::cout << arg << std::endl;
		switch(entt::HashedString(argv[1])) {
			case entt::HashedString("key"):
			std::cout << "FOUND KEY" <<std::endl;
			break;
			case entt::HashedString("value"):
			std::cout << "FOUND VALUE" <<std::endl;
			break;
		}
		
	}

};