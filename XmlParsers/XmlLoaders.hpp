#pragma once

enum class TextureLoadMode {
	Default,
	WithWhiteMask,
	Button,
	BuildButton,
	WithDirections
};

static std::map<std::string, TextureLoadMode> texLoadModes =
{
	{ "default", TextureLoadMode::Default },
	{ "with_white_mask", TextureLoadMode::WithWhiteMask },
	{ "button", TextureLoadMode::Button },
	{ "build_button", TextureLoadMode::BuildButton },
	{ "with_directions", TextureLoadMode::WithDirections },
};

class TextureLoader {
	TextureManager *texManager;
public:
	std::map<std::string, sf::IntRect> centerRects;

	void setManager(TextureManager *mgr) {
		this->texManager = mgr;
	}

	void getSpecialPix(std::string name, sf::Image &image, int width, int height) {
		sf::Vector2i pix1;
		sf::Vector2i pix2;
		int found = 0;
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				if (found > 1)
					return;
				if (image.getPixel(x, y) == sf::Color(255, 36, 196)) {
#ifdef PARSER_DEBUG
					std::cout << "TextureLoader: " << name << " special pix " << x << "x" << y << std::endl;
#endif
					if (found == 0) {
						pix1 = sf::Vector2i(x, y);
					} else if (found == 1) {
						pix2 = sf::Vector2i(x, y);
						sf::IntRect centerRect(pix1, pix2 - pix1);
#ifdef PARSER_DEBUG
						std::cout << "TextureLoader: " << name << " center rect " << centerRect.left << "x" << centerRect.top << ":" << centerRect.width << "x" << centerRect.height << std::endl;
#endif
						centerRects[name] = centerRect;

						// clear special pixels
						image.setPixel(centerRect.left,centerRect.top, sf::Color(255,255,255,0));
						image.setPixel(centerRect.left+centerRect.width,centerRect.top+centerRect.height, sf::Color(255,255,255,0));
					}

					found++;
				}
			}
		}

		if (found) {

		}
	}

	void loadTextureWithWhiteMask(std::string name, std::string filename) {
		sf::Image img;
		img.loadFromFile(filename);
		this->getSpecialPix(name, img, img.getSize().x, img.getSize().y);
		img.createMaskFromColor(sf::Color::White);
		texManager->load(name, img, sf::IntRect{0, 0, (int)img.getSize().x, (int)img.getSize().y});
	}

	void loadTextureWithDirections(std::string name, std::string imgPath) {
		sf::Image image, outImage;

		image.loadFromFile(imgPath);
		int columnWidth = image.getSize().x / 5;
		int height = image.getSize().y;

		this->getSpecialPix(name, image, columnWidth, height);

		outImage.create(columnWidth * 8, height, sf::Color::Transparent);

		std::vector<sf::Image> directionsImg;

		for (int i = 0; i < 5; i++) {
			sf::Image dirImg;
			dirImg.create(columnWidth, height, sf::Color::Transparent);
			dirImg.copy(image, 0, 0, sf::IntRect(i * columnWidth, 0, columnWidth, height), true);
			directionsImg.push_back(dirImg);
		}

		for (int i = 1; i < 4; i ++) {
			sf::Image dirImg;
			dirImg.create(columnWidth, height, sf::Color::Transparent);
			dirImg.copy(image, 0, 0, sf::IntRect(i * columnWidth, 0, columnWidth, height), true);
			dirImg.flipHorizontally();
			directionsImg.push_back(dirImg);
		}

		for (int i = 0; i < 8; i++) {
			outImage.copy(directionsImg[i], i * columnWidth, 0, sf::IntRect(0, 0, columnWidth, height), true);
		}

		outImage.createMaskFromColor(sf::Color::White);
#ifdef PARSER_DEBUG
		std::cout << "TextureLoader: init dir texture " << name << " " << columnWidth << "x" << height << std::endl;
#endif
		texManager->load(name, outImage, sf::IntRect {0, 0, columnWidth * 8, height});
	}

	void loadButton(std::string name, std::string filename) {
		sf::Image but;
		but.loadFromFile(filename);
		texManager->load(name, but, sf::IntRect{0, 0, (int)but.getSize().x, (int)but.getSize().y / 2});
		texManager->load(name + "_down", but, sf::IntRect{0, (int)(but.getSize().y / 2), (int)but.getSize().x, (int)(but.getSize().y) / 2});
	}

	void loadBuildButton(std::string name, std::string filename) {
		sf::Image but;
		but.loadFromFile(filename);
		int height = (int)but.getSize().y / 5;
		texManager->load(name, but, sf::IntRect{0, 0, (int)but.getSize().x, height});
		texManager->load(name + "_down", but, sf::IntRect{0, height, (int)but.getSize().x, height});
		texManager->load(name + "_building", but, sf::IntRect{0, height * 2, (int)but.getSize().x, height});
		texManager->load(name + "_built", but, sf::IntRect{0, height * 3, (int)but.getSize().x, height});
		texManager->load(name + "_built_down", but, sf::IntRect{0, height * 4, (int)but.getSize().x, height});
	}

	void recParse(std::string name, tinyxml2::XMLElement *element) {
		for (tinyxml2::XMLElement *child : element) {
			std::string cname = name;
			if (child->Attribute("name")) {
				cname = name + "_" + child->Attribute("name");
			}

//			std::cout << "TextureParser: recParse child " << child->Name() << std::endl;

			if (std::string(child->Name()) == "texture")
			{
				if (child->Attribute("path")) {
					std::string path = child->Attribute("path");
					TextureLoadMode mode = TextureLoadMode::Default;
					if (child->Attribute("mode"))
						mode = texLoadModes[child->Attribute("mode")];

					if (child->Attribute("global")) {
						cname = child->Attribute("global");
					}

#ifdef PARSER_DEBUG
					std::cout << "TextureLoader: recParse child " << cname << " " << path << " " << (int)mode << std::endl;
#endif

					switch (mode) {
					case TextureLoadMode::Default:
						texManager->load(cname, path);
						break;
					case TextureLoadMode::WithWhiteMask:
						this->loadTextureWithWhiteMask(cname, path);
						break;
					case TextureLoadMode::Button:
						this->loadButton(cname, path);
						break;
					case TextureLoadMode::BuildButton:
						this->loadBuildButton(cname, path);
						break;
					case TextureLoadMode::WithDirections:
						this->loadTextureWithDirections(cname, path);
						break;
					}
				}
			}

			this->recParse(cname, child);
		}
	}

	void parse(tinyxml2::XMLElement *element) {
		this->recParse(element->Attribute("name"), element);
	}

	std::string getName(tinyxml2::XMLElement *element) {
		return element->Attribute("name");
	}

	void parseFile(std::string filename) {
		tinyxml2::XMLDocument doc;
		doc.LoadFile(filename.c_str());
		this->parse(doc.RootElement());
	}

};

class SoundBufferLoader {
	SoundBufferManager *sndManager;
public:
	void setManager(SoundBufferManager *mgr) {
		this->sndManager = mgr;
	}

	void recParse(std::string name, tinyxml2::XMLElement *element) {
		int i = 0;
		int cnt = 0;
		for (tinyxml2::XMLElement *child : element) {
			std::string nodeName = child->Name();
			if (nodeName == "sound_buffer")
				cnt++;
		}

		if (cnt == 1) {
			tinyxml2::XMLElement *child = element->FirstChildElement("sound_buffer");
			std::string cname = name;
			if (child->Attribute("name")) {
				std::string cName = child->Attribute("name");
				cname = name + "_" + cName;
			}

			if (child->Attribute("global")) {
				cname = child->Attribute("global");
			}

			sndManager->load(cname, child->Attribute("path"));
		} else {
			for (tinyxml2::XMLElement *child : element) {
				std::string nodeName = child->Name();
				std::string cname = name;
				if (child->Attribute("name")) {
					std::string cName = child->Attribute("name");
					cname = name + "_" + cName;
				}

				if (nodeName == "sound_buffer")
				{
					std::string ncname = cname + "_" + std::to_string(i);
					if (child->Attribute("path")) {
						std::string path = child->Attribute("path");

#ifdef PARSER_DEBUG
						std::cout << "SoundBufferLoader: recParse child " << ncname << " " << path << std::endl;
#endif

						if (child->Attribute("global")) {
							ncname = child->Attribute("global");
						}

						sndManager->load(ncname, path);
					}
					i++;
				}
				this->recParse(cname, child);
			}

		}
	}
	void parse(tinyxml2::XMLElement *element) {
		this->recParse(element->Attribute("name"), element);
	}

	void parseFile(std::string filename) {
		tinyxml2::XMLDocument doc;
		doc.LoadFile(filename.c_str());
		this->parse(doc.RootElement());
	}

};
