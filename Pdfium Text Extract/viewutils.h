#ifndef VIEW_UTILS
#define VIEW_UTILS

namespace textextract {
	enum class PageOrientation { NONE = 0, PORTRAIT = 1, LANDSCAPE = 2 };

	// Page rotation is assumed to be a clockwise rotation.
	enum class PageRotation {
		NO_ROTATION = 0,
		CLOCKWISE_90 = 1,
		CLOCKWISE_180 = 2,
		CLOCKWISE_270 = 3
	};

	// Page sizes are derived from the ANSI standard page sizes
	// for US and Canada, and the ISO standard page sizes.
	// https://en.wikipedia.org/wiki/Paper_size
	enum class PageType {
		NONE = 0,
		ANSI_LETTER = 1,
		ANSI_LEGAL = 2,
		ANSI_LEDGER = 3,
		ANSI_TABLOID = 4,
		ANSI_EXECUTIVE = 5,
		ANSI_C = 6,
		ANSI_D = 7,
		ANSI_E = 8,
		ISO_A0 = 9,
		ISO_A1 = 10,
		ISO_A2 = 11,
		ISO_A3 = 12,
		ISO_A4 = 13,
		ISO_A5 = 14,
		ISO_A6 = 15,
		ISO_A7 = 16,
		ISO_A8 = 17,
	};

	// Each page dimension is specified in
	// points (1/72 inch, always rounded off).
	// https://en.wikipedia.org/wiki/Point_(typography)
	struct PageDimensions {
		int width;
		int height;
		PageDimensions() : width(0), height(0) {}
		PageDimensions(int w, int h) : width(w), height(h) {}
	};

	/**
	* @brief Class that describes the size of a page, in terms of dimensions and page type.
	*/
	class PageSize {
	private:
		// What type the page is given a set of page dimensions for all possible types.
		PageType mPageType;
		// The dimensions of the page.
		PageDimensions mPageDimensions;
		/**
		* Swap operation for PageSize assignment.
		*
		* @param first First PageSize object in swap operation.
		* @param second Second PageSize object in swap operation.
		*/
		void swap(PageSize& first, PageSize& second);

	public:
		PageSize() : mPageType(PageType::NONE), mPageDimensions(0, 0) {}
		PageSize(PageType pt, PageDimensions pd) :
			mPageType(pt), mPageDimensions(pd) {}
		/**
		* Get the type of page.
		*
		* @returns The PageType for the page.
		*/
		const PageType GetType() const { return mPageType; }
		/**
		* Get the dimensions of page.
		*
		* @returns The PageDimensions for the page.
		*/
		const PageDimensions GetPageDimensions() const { return mPageDimensions; }
		/**
		* PageSize assiment operator
		*
		* @param pageSize page size for new assigned value.
		* @returns Reassigned PageSize.
		*/
		PageSize& operator=(PageSize pageSize);
		/**
		* Determine the applicable standard page size, given some width and height.
		*
		* @param width The width of the page.
		* @param height The width of the page.
		*/
		void DeterminePageSize(int width, int height);
	};
} // namespace textextract
#endif