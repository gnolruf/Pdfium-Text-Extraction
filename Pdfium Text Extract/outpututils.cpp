#include "outpututils.h"

#include "pdfpageinfo.h"

#include <fstream>
#include <iostream>
#include <boost/locale.hpp>
#include <nlohmann/json.hpp>

namespace textextract {
	void WriteTextFile(std::string writeLocation) {
		std::ofstream fs(writeLocation);
		auto rawtext = boost::locale::conv::utf_to_utf<char>(GetRawPageText());
		if (!fs) {
			std::cerr << "Error opening the file to write text." << std::endl;
			exit(0);
		}
		fs << rawtext;
		fs.close();
	}

	void WriteJSON(std::string writeLocation) {
		nlohmann::json j;
		j["render_size"] = {
			{"width", std::to_string(GetPageRender().cols)},
			{"height", std::to_string(GetPageRender().rows)},
		};
		nlohmann::json jArray = nlohmann::json::array();
		for (const auto& tb : GetPageWords()) {
			nlohmann::json wordObject;
			wordObject["wordtoken"] = boost::locale::conv::utf_to_utf<char>(tb.GetText());
			wordObject["bounds"] = {
				{"x", std::to_string(tb.GetBounds().x)},
				{"y", std::to_string(tb.GetBounds().y)},
				{"width", std::to_string(tb.GetBounds().width)},
				{"height", std::to_string(tb.GetBounds().height)}
			};
			jArray.push_back(wordObject);
		}
		std::ofstream file(writeLocation);
		if (file.is_open()) {
			file << j.dump(4);
			file << jArray.dump(4);
			file.close();
		}
	}

	void WriteOutput(std::filesystem::path writeLocation, int pageNum, std::string fileName, bool textOnly) {
		std::cout << "writing output for page: " + std::to_string(pageNum) << std::endl;
		if (textOnly) {
			WriteTextFile(writeLocation.string() + "\\" + fileName + "pg" + std::to_string(pageNum) + ".txt");
		}
		else {
			WriteJSON(writeLocation.string() + "\\" + fileName + "pg" + std::to_string(pageNum) + ".json");
		}
	}
} // namespace textextract
