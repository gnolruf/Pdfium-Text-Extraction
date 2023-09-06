#ifndef OUTPUT_UTILS
#define OUTPUT_UTILS
#include <filesystem>

namespace textextract {
	/**
	* Write the result of the text extraction to file.
	*
	* @param writeLocation Path to the write location for the result.
	* @param pageNum Page number of the text extraction result.
	* @param fileName Name of the original file, used as the name for the extraction result file.
	* @param textOnly Boolean indicating whether or not bounds should be omitted from the result.
	*/
	void WriteOutput(std::filesystem::path writeLocation, int pageNum, std::string fileName, bool textOnly);
} // namespace textextract

#endif
