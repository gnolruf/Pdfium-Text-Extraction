#include "textextractutils.h"

#include "load_support.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace textextract {
	// Determine the standard deviation of a collection of TextBox bounding boxes.
	double RectAreaStdDev(const std::vector<TextBox>& textBoxRects, double meanRectArea) {
		if (textBoxRects.size() < 2) {
			return 0.0;
		}

		double varianceSum = 0.0;
		for (const auto& rect : textBoxRects) {
			varianceSum += pow(rect.GetBounds().area() - meanRectArea, 2);
		}
		return sqrt(varianceSum / (double)textBoxRects.size());
	}

	// Determine the average area of a collection of TextBox bounding boxes.
	double AverageRectArea(const std::vector<TextBox>& textBoxRects) {
		if (textBoxRects.empty()) return 0;
		double avgrectarea = 0.0;
		for (const auto& rect : textBoxRects) {
			if (rect.GetBounds().area() != 0) {
				avgrectarea += rect.GetBounds().area();
			}
		}
		avgrectarea /= (double)textBoxRects.size();
		return avgrectarea;
	}

	// Determine if a bounding box signifigantly overlaps another.
	bool BoundsOverlaps(TextBox toCheck, const std::vector<TextBox>& charRects, int omitIdx) {
		for (int i = 0; i < charRects.size(); i++) {
			if (i != omitIdx) {
				if (toCheck.OverlapPercent(charRects[i].GetBounds()) > .95f) {
					return true; 
				}
			}
		}
		return false;
	}

	void RemoveWaterMarkText(std::vector<TextBox>& textBoxes) {
		double meanarea = AverageRectArea(textBoxes);
		double stddev = RectAreaStdDev(textBoxes, meanarea);
		double anomalycutoff = stddev * 3;
		double lowerlimit = meanarea - anomalycutoff;
		double upperlimit = meanarea + anomalycutoff;
		std::vector<TextBox> correctedtextboxes;
		for (auto it = textBoxes.begin(); it != textBoxes.end(); ) {
			if (it->GetBounds().area() > upperlimit || it->GetBounds().area() < lowerlimit) {
				if (BoundsOverlaps(*it, textBoxes, std::distance(textBoxes.begin(), it))) {
					it = textBoxes.erase(it);
					continue;
				}
			}
			++it;
		}
	}

	void CleanToken(std::wstring& token) {
		// Looking for groups of greater than three for common decorator characters
		const int REPEATING_CHAR_LIMIT = 3;
		const std::wstring decorationchars = L"-+_=|~";

		size_t pos = 0;
		while (pos < token.size()) {
			if (decorationchars.find(token[pos]) != std::wstring::npos) {
				size_t run_start = pos;
				while (pos + 1 < token.size() && token[pos] == token[pos + 1]) {
					++pos;
				}
				if (pos - run_start + 1 >= REPEATING_CHAR_LIMIT || token.size() == 1) {
					token.erase(run_start, pos - run_start + 1);
					pos = run_start;
				}
				else {
					++pos;
				}
			}
			else {
				++pos;
			}
		}
	}

	// Clean up any empty string tokens, or tokens that may be comprised of decorators.
	void CleanUpTokens(std::vector<std::wstring>& tokens) {
		for (auto& token : tokens) {
			CleanToken(token);
		}

		// Remove tokens that are empty strings
		tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
			[](const std::wstring& str) { return str.empty(); }),
			tokens.end());
	}

	// Remove certain escape characters from the raw text that may be carried into tokens.
	std::wstring CleanText(const std::wstring& text) {
		std::wstring cleaned = text;

		// Replace certain problematic sequences
		std::replace(cleaned.begin(), cleaned.end(), '\n', ' ');
		std::replace(cleaned.begin(), cleaned.end(), '\t', ' ');
		std::replace(cleaned.begin(), cleaned.end(), '\r', ' ');
		
		return cleaned;
	}

	std::vector<std::wstring> GetWordTokens(std::wstring rawText) {
		std::vector<std::wstring> wordtokens;
		std::wstring processedpagetext = CleanText(rawText);
	
		std::wstring::const_iterator start = processedpagetext.begin(), end = processedpagetext.end();

		while (start != end) {
			std::wstring::const_iterator wordStart = std::find_if_not(start, end, iswspace);
			std::wstring::const_iterator wordEnd = std::find_if(wordStart, end, iswspace);

			if (wordStart != end) {
				wordtokens.push_back(std::wstring(wordStart, wordEnd));
			}

			start = wordEnd;
		}

		CleanUpTokens(wordtokens);

		return wordtokens;
	}

	cv::Rect NormalizeRect(
		const cv::Rect& toNormalize, const int& angle, const int& pageWidth,
		const int& pageHeight) {
		cv::Point p1;
		cv::Point p2;
		cv::Rect correctedrect;
		switch (angle) {
		case 0:
			p1 = cv::Point(toNormalize.x, pageHeight - toNormalize.y);
			p2 = cv::Point(toNormalize.br().x, pageHeight - toNormalize.br().y);
			correctedrect = cv::Rect(p1, p2);
			break;
		case -180:
		case 180:
			p1 = cv::Point(toNormalize.x, toNormalize.y);
			p2 = cv::Point(toNormalize.br().x, toNormalize.br().y);
			correctedrect = cv::Rect(p1, p2);
			break;
		case -90:
		case 90:
		case -270:
		case 270:
			p1 = cv::Point(toNormalize.y, toNormalize.x);
			p2 = cv::Point(toNormalize.br().y, toNormalize.br().x);
			correctedrect = cv::Rect(p1, p2);
			break;
		default:
			p1 = cv::Point(toNormalize.x, pageHeight - toNormalize.y);
			p2 = cv::Point(toNormalize.br().x, pageHeight - toNormalize.br().y);
			correctedrect = cv::Rect(p1, p2);
			break;
		}
		return correctedrect;
	}

	std::vector<TextBox> GetTextBoxesFromTokens(
		const std::vector<std::wstring>& wordTokens, const FPDF_TEXTPAGE textPage,
		const int pageWidth, const int pageHeight) {
		std::vector<int> usedindexes;
		std::vector<TextBox> tokentextboxes;
		for (const auto& token : wordTokens) {
			std::unique_ptr<unsigned short, pdfium::FreeDeleter> searchtoken = 
				GetFPDFWideString(token);
			FPDF_SCHHANDLE search =
				FPDFText_FindStart(textPage, searchtoken.get(), 2, 0);
			if (search != nullptr) {
				FPDFText_FindNext(search);
				int startindex = FPDFText_GetSchResultIndex(search);
				int searchcount = FPDFText_GetSchCount(search);
				while (std::find(usedindexes.begin(), usedindexes.end(), startindex) !=
					usedindexes.end()) {
					if (FPDFText_FindNext(search)) {
						startindex = FPDFText_GetSchResultIndex(search);
						searchcount = FPDFText_GetSchCount(search);
					}
					else
						break;
				}
				usedindexes.push_back(startindex);
				int currboxindex = startindex;
				cv::Rect combinedrect;
				int angle = 0;
				while (currboxindex < (startindex + searchcount)) {
					double left = 0, top = 0, right = 0, bottom = 0;
					FPDFText_GetCharBox(
						textPage, currboxindex, &left, &right, &bottom, &top);
					angle = FPDFText_GetCharAngle(textPage, currboxindex) *
						(180.0 / 3.141592653589793238463);
					cv::Point pt1(left, top);
					cv::Point pt2(right, bottom);
					cv::Rect charrect(pt1, pt2);
					if (charrect.width == 0 && charrect.height != 0) charrect.width = 1;
					else if (charrect.width != 0 && charrect.height == 0) charrect.height = 1;
					combinedrect |= charrect;
					currboxindex++;
					usedindexes.push_back(currboxindex);
				}
				// We assume angle will be the same for all char boxes
				TextBox tb(
					NormalizeRect(combinedrect, angle, pageWidth, pageHeight),
					token);
				tokentextboxes.push_back(tb);
			}
			FPDFText_FindClose(search);
		}
		return tokentextboxes;
	}

	void RescaleTextBoxes(std::vector<TextBox>& textBoxes, const PageDimensions& originalDims, const PageDimensions& renderDims) {
		for (auto& tb : textBoxes) {
			tb.RescaleDims(originalDims, renderDims);
		}
	}

	void DebugTextBoxes(const cv::Mat& render, const std::vector<TextBox>& textBoxes) {
		cv::Mat image = render.clone();
		for (auto& tb : textBoxes) {
			cv::rectangle(image, tb.GetBounds(), cv::Scalar(0, 0, 0), 2);
		}
		cv::imshow("result", image);
		cv::waitKey(0);
		image.release();
	}
}
