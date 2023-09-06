#ifndef TEXT_BOX
#define TEXT_BOX

#include <opencv2/core/types.hpp>

#include "viewutils.h"

namespace textextract {
	class TextBox {
	private:
		// The text contained within the TextBox.
		std::wstring mText;
		// The location of the textbox of a document image.
		cv::Rect mBoundingRect;

	public:
		TextBox() : mBoundingRect(cv::Rect()) {}
		TextBox(cv::Rect bbox, std::wstring txt) :
			mBoundingRect(bbox) {
			SetText(txt);
		}
		/**
		* Set the text for the text region.
		*
		* @params txt The text to be set.
		*/
		void SetText(std::wstring txt) { mText = txt; }
		/**
		* Set the bounds for the text region.
		*
		* @params box Rectangle to set as bounds.
		*/
		void SetBounds(cv::Rect box) { mBoundingRect = box; }
		/**
		* Get the bounding box of the text.
		*
		* @returns The bounds of the text represented as a rectangle.
		*/
		cv::Rect GetBounds() const { return mBoundingRect; }
		/**
		* Get the text of the text box.
		*
		* @returns The text of the text box represented as a string.
		*/
		std::wstring GetText() const { return mText; }
		/**
		* Determines a percentage of the area that the text box overlaps with, given goal region.
		*
		* @param goalRegion Region to determine whether the current text box resides in.
		*
		* @returns The percentage of area from the textbox that overlaps with a goal region.
		*/
		float OverlapPercent(cv::Rect goalRegion);
		/**
		* Rescale Textbox from its original page dimensions, given new page dimensions.
		*
		* @param originalDimensions The original source dimensions from which the TextBoxes were derived.
		* @param rescalingDimensions The page dimensions to rescale to.
		*/
		void RescaleDims(PageDimensions originalDimensions, PageDimensions rescalingDimensions);
	};
} // namespace textextract
#endif