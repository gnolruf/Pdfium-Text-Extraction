#include "textbox.h"

namespace textextract {
	float TextBox::OverlapPercent(cv::Rect goalregion) {
		// the '&' operator between to cv::Rects gives us the
		// overlapping Rect
		cv::Rect match = goalregion & mBoundingRect;
		if (match.area() > 0) {
			// matching expressed as a ratio between the area to match and the match that was found
			return (float)match.area() / (float)goalregion.area();
		} else {
			return 0;
		}
	}

	void TextBox::RescaleDims(PageDimensions originalDimensions, PageDimensions rescalingDimensions) {
		std::vector<TextBox> normalized;
		int x1 = (rescalingDimensions.width * mBoundingRect.x) /
			originalDimensions.width;
		int y1 = (rescalingDimensions.height * mBoundingRect.y) /
			originalDimensions.height;
		int x2 = (rescalingDimensions.width *
			(mBoundingRect.x + mBoundingRect.width)) /
			originalDimensions.width;
		int y2 =(rescalingDimensions.height *
			(mBoundingRect.y + mBoundingRect.height)) /
			originalDimensions.height;
		cv::Point p1(x1, y1);
		cv::Point p2(x2, y2);
		cv::Rect norm(p1, p2);
		this->SetBounds(norm);
	}
} // namespace textextract
