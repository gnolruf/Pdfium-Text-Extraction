#include "pdfpageinfo.h"

namespace textextract {
	namespace {
		cv::Mat mRender;
		PageSize mPageSize;
		PageOrientation mPageOrientation;
		PageRotation mPageRotation;
		std::wstring mRawPageText;
		std::vector<TextBox> mPageWords;
	} // namespace

	// Accessors
	cv::Mat GetPageRender() {
		return mRender;
	}
	PageSize GetPageSize() {
		return mPageSize;
	}
	PageRotation GetPageRotation() {
		return mPageRotation;
	}
	std::vector<TextBox> GetPageWords() {
		return mPageWords;
	}
	std::wstring GetRawPageText() {
		return mRawPageText;
	}
	PageOrientation GetPageOrientation() {
		return mPageOrientation;
	}

	// Mutators
	void ClearPageInfo() {
		mRender.release();
		mPageSize = PageSize();
		mPageRotation = PageRotation::NO_ROTATION;
		mRawPageText.clear();
		mPageWords.clear();
	}
	void SetRender(cv::Mat render) {
		mRender = render;
	}
	void SetPageSize(int width, int height) {
		mPageSize.DeterminePageSize(width, height);
	}
	void SetPageRotation(PageRotation pageRotation) {
		mPageRotation = pageRotation;
	}
	void SetPageWords(std::vector<TextBox> pageWords) {
		mPageWords = pageWords;
	}
	void SetRawPageText(std::wstring rawText) {
		mRawPageText = rawText;
	}
	void SetPageOrientation(PageOrientation pageOrientation) {
		mPageOrientation = pageOrientation;
	}
} // namespace textextract