#ifndef TEXT_EXTRACT_UTILS
#define TEXT_EXTRACT_UTILS

#include "pdfium/cpp/fpdf_scopers.h"

#include "textbox.h"

namespace textextract {
	/**
	* Debug the TextBoxes that are found for a page by drawing each Textboxes' coordinates
	* on a copy of the page render.
	* 
	* @param render Render of the page to draw bounding boxes from TextBox to.
	* @param textBoxes TextBoxes to derive bounding boxes from to draw on render.
	*/
	void DebugTextBoxes(const cv::Mat& render, const std::vector<TextBox>& textBoxes);
	/**
	* Rescale the found TextBoxes to fit to the size of the page render.
	* 
	* @param textboxes Vector of TextBoxes to rescale.
	* @param originaldims Original dimensions the TextBoxes were derived from.
	* @param renderDims Dimensions of the render to rescale the TextBoxes to.
	*/
	void RescaleTextBoxes(std::vector<TextBox>& textBoxes, const PageDimensions& originaldims, const PageDimensions& renderDims);
	/**
	* Given a vector of TextBoxes, remove any elements that are potentially watermarks that
	* cover the majority of the page. 
	*
	* @param textBoxes Vector of TextBoxes to rescale.
	*/
	void RemoveWaterMarkText(std::vector<TextBox>& textBoxes);
	/**
	* Get the word tokens from the raw text from a page.
	*
	* @param rawText Text to derive tokens from.
	* 
	* @returns vector of wstring word tokens.
	*/
	std::vector<std::wstring> GetWordTokens(std::wstring rawText);
	/**
	* Given a vector of word tokens, find their colerlated bounding boxes
	* from the FPDF_TEXTPAGE that the word tokens originate from.
	*
	* @param wordTokens Tokens to derive bounds from.
	* @param textPage FPDF_TEXTPAGE that bounds ar derived from.
	* @param pageWidth width of the page.
	* @param pageHeight height of the page.
	* 
	* @returns vector of wstring word tokens.
	*/
	std::vector<TextBox> GetTextBoxesFromTokens(const std::vector<std::wstring>& wordTokens, const FPDF_TEXTPAGE textPage,
		const int pageWidth, const int pageHeight);
} // namespace textextract
#endif