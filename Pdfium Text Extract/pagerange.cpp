#include "pagerange.h"

#include <vector>
#include <iostream>
#include <sstream>

namespace textextract {
	// Split the potential range string into its seperate tokens.
	std::vector<std::string> split(const std::string& s, char delimiter) {
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(s);
		while (std::getline(tokenStream, token, delimiter)) {
			tokens.push_back(token);
		}
		return tokens;
	}

	// Clean the range string and remove any spaces.
	std::string removeSpaces(std::string str) {
		str.erase(remove(str.begin(), str.end(), ' '), str.end());
		return str;
	}

	// Determine if the token is a valid number.
	bool isValidNumber(const std::string& str) {
		if (str.empty()) return false;
		return str.find_first_not_of("0123456789") == std::string::npos;
	}

	void PageRange::ParsePageRange(const std::string& pageRange, int pageCount) {
		std::string cleaned = removeSpaces(pageRange);
		try {
			// Single page
			if (cleaned.find('-') == std::string::npos) {
				if (cleaned == "0") {
					firstpage = 1;
					lastpage = pageCount;
					return;
				}
				if (isValidNumber(cleaned)) {
					int singlepage = std::stoi(cleaned);
					if (singlepage > pageCount) {
						throw singlepage;
					}
					firstpage = singlepage;
					lastpage = singlepage;
					return;
				}
				else {
					throw cleaned;
				}
			}
			// Multiple pages
			else {
				std::vector<std::string> parts = split(cleaned, '-');
				if (parts.size() != 2) {
					throw cleaned;
				}
				if (!isValidNumber(parts[0]) || !isValidNumber(parts[1])) {
					throw cleaned;
				}

				int start = std::stoi(parts[0]);
				int end = std::stoi(parts[1]);
				if (start > pageCount || start < 1) throw start;
				if (end > pageCount || end < 1) throw end;
				firstpage = start;
				lastpage = end;
			}
		}
		catch (std::string invalidRange) {
			std::cerr << "Invaild page range: " + invalidRange << std::endl;
			std::cerr << "Page range must either be a single positive integer, or range written as <first_page>-<last_page>" << std::endl;
			std::exit(EXIT_FAILURE);
		}
		catch (int pageNum) {
			std::cerr << "Invalid page index: " + std::to_string(pageNum) << std::endl;
			std::cerr << "Page Index is outside number of pages in pdf." << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}
} // namespace textextract
