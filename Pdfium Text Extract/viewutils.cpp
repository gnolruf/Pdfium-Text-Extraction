#include "viewutils.h"

#include <optional>

namespace textextract {
	// Table of potential page sizes for a PDF.
	static const PageSize pageSizeTable[] = {
	  PageSize(PageType::NONE, PageDimensions(1, 1)),
	  PageSize(PageType::ANSI_LETTER, PageDimensions(612, 792)),
	  PageSize(PageType::ANSI_LEGAL, PageDimensions(612, 1008)),
	  PageSize(PageType::ANSI_LEDGER, PageDimensions(792, 1224)),
	  PageSize(PageType::ANSI_TABLOID, PageDimensions(1224, 792)),
	  PageSize(PageType::ANSI_EXECUTIVE, PageDimensions(522, 756)),
	  PageSize(PageType::ANSI_C, PageDimensions(1584, 1224)),
	  PageSize(PageType::ANSI_D, PageDimensions(2448, 1584)),
	  PageSize(PageType::ANSI_E, PageDimensions(3168, 2448)),
	  PageSize(PageType::ISO_A0, PageDimensions(2384, 3370)),
	  PageSize(PageType::ISO_A1, PageDimensions(1684, 2384)),
	  PageSize(PageType::ISO_A2, PageDimensions(1190, 1684)),
	  PageSize(PageType::ISO_A3, PageDimensions(842, 1190)),
	  PageSize(PageType::ISO_A4, PageDimensions(595, 842)),
	  PageSize(PageType::ISO_A5, PageDimensions(420, 595)),
	  PageSize(PageType::ISO_A6, PageDimensions(298, 420)),
	  PageSize(PageType::ISO_A7, PageDimensions(210, 298)),
	  PageSize(PageType::ISO_A8, PageDimensions(148, 210)),
	};

	void PageSize::DeterminePageSize(int width, int height) {
		PageDimensions pageDim(width, height);
		mPageDimensions = pageDim;

		float widthscale = 0.0, heightscale = 0.0;
		for (int i = 1; i < (sizeof(pageSizeTable) / sizeof(pageSizeTable[0]));
			i++) {
			widthscale = width % pageSizeTable[i].GetPageDimensions().width;
			heightscale = height % pageSizeTable[i].GetPageDimensions().height;
			if (widthscale == 0 && heightscale == 0) {
				mPageType = pageSizeTable[i].mPageType;
				return;
			}
		}
		// If there is no PageSize match, type is set to none
		mPageType = pageSizeTable[0].mPageType;
	}

	void PageSize::swap(PageSize& first, PageSize& second) {
		//using std::swap;
		std::swap(first.mPageType, second.mPageType);
		std::swap(first.mPageDimensions, second.mPageDimensions);
	}

	PageSize& PageSize::operator=(PageSize pageSize) {
		swap(*this, pageSize);
		return *this;
	}
} // namespace textextract