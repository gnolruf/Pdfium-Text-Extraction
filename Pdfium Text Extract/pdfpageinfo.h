#ifndef PDF_INFO
#define PDF_INFO

#include "textbox.h"

#include <opencv2/imgcodecs.hpp>

namespace textextract {
	/**
	* Get the render data for the current page.
	*
	* @returns Image data as a cv::Mat.
	*/	
	cv::Mat GetPageRender();
	/**
	* Get the default dimensions and page type for the current page.
	*
	* @returns PageSize from set of possible sizes.
	*/
	PageSize GetPageSize();
	/**
	* Get the rotation of a page, in terms of degrees.
	*
	* @returns PageRotation from a set of possible rotations.
	*/
	PageRotation GetPageRotation();
	/**
	* Get the orientation of the page.
	*
	* @returns PageOrientation from a set of possible orientation.
	*/
	PageOrientation GetPageOrientation();
	/**
	* Get the words with their bounds for a page.
	*
	* @returns A vector of TextBoxes.
	*/
	std::vector<TextBox> GetPageWords();
	/**
	* Get the raw unprocessed text for a page.
	*
	* @returns The text on the page as a wstring.
	*/
	std::wstring GetRawPageText();
	/**
	* Clear all information associated with the currently instantiated page.
	*/
	void ClearPageInfo();
	/**
	* Set the image data for the page render.
	*
	* @param render The image data for the page.
	*/
	void SetRender(cv::Mat render);
	/**
	* Set the size of the page.
	*
	* @param width The width of the page render.
	* @param height The height of the page render.
	*/
	void SetPageSize(int width, int height);
	/**
	* Set the rotation for the page.
	*
	* @param pageRotation The rotation for the page.
	*/
	void SetPageRotation(PageRotation pageRotation);
	/**
	* Set the oreintation for the page.
	*
	* @param pageOrientation The orientation for the page.
	*/
	void SetPageOrientation(PageOrientation pageOrientation);
	/**
	* Set the words for the page.
	*
	* @param pageWords The words that come from the PDF for the page.
	*/
	void SetPageWords(std::vector<TextBox> pageWords);
	/**
	* Set the raw text for the page.
	*
	* @param pageWords The words that come from the page.
	*/
	void SetRawPageText(std::wstring rawText);
} // namespace textextract

#endif