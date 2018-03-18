#pragma once

#include "third_party/cfgpath/cfgpath.h"
#include "third_party/tinyxml2.h"
#include "third_party/tixml2ex.h"

class GameSettings {
public:
	int screenSize;
	bool fullscreen;

	GameSettings() {
		this->screenSize = 1;
		this->fullscreen = false;

		this->load();
		this->save();
	}

	std::string dir() {
		char cfgdir[MAX_PATH];
		get_user_config_folder(cfgdir, sizeof(cfgdir), "bfr");
		if (cfgdir[0] == 0) {
			std::cout << "Unable to find home directory." << std::endl;
			return "";
		}
		return cfgdir;
	}

	std::string filename() {
		return this->dir() + "settings.xml";
	}

	void load() {
		tinyxml2::XMLDocument xmlDoc;
		if(!xmlDoc.LoadFile(this->filename().c_str())) {
			std::cout << "GameSettings: load "<<this->filename()<<std::endl;
			tinyxml2::XMLElement *pRoot = xmlDoc.RootElement();

			this->screenSize = pRoot->FirstChildElement("screen_size")->IntAttribute("value");
			this->fullscreen = pRoot->FirstChildElement("fullscreen")->BoolAttribute("value");
		}
	};

	void save() {
		tinyxml2::XMLDocument xmlDoc;
		tinyxml2::XMLElement * pRoot = xmlDoc.NewElement("settings");
		xmlDoc.InsertFirstChild(pRoot);

		tinyxml2::XMLElement * screenSizeEl = xmlDoc.NewElement("screen_size");
		screenSizeEl->SetAttribute("value", this->screenSize);
		pRoot->InsertEndChild(screenSizeEl);

		tinyxml2::XMLElement * fullscreenEl = xmlDoc.NewElement("fullscreen");
		fullscreenEl->SetAttribute("value", this->fullscreen);
		pRoot->InsertEndChild(fullscreenEl);

		std::cout << "GameSettings: save "<<this->filename()<<std::endl;
		xmlDoc.SaveFile(this->filename().c_str());
	};
};