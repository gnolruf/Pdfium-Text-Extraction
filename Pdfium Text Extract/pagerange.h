#ifndef PAGE_RANGE
#define PAGE_RANGE

#include <string>

namespace textextract {
	struct PageRange {
		// Page number of the first page of the range to be processed.
		int firstpage = 0;
		// Page number of the last page of the range to be processed.
		int lastpage = 0;

		/**
		* Get the render data for the current page.
		*
		* @param pageRange String representing the range of pages to parse.
		* @param pageCount total page count for the document, to ensure page range is logical.
		*/
		void ParsePageRange(const std::string& pageRange, int pageCount);

		PageRange(std::string pageRange, int pageCount) { ParsePageRange(pageRange, pageCount); }
	};
} // namespace textextract
#endif
