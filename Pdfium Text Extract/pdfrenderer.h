#ifndef PDF_RENDER
#define PDF_RENDER

#include <map>

#if defined PDF_ENABLE_SKIA && !defined _SKIA_SUPPORT_
#define _SKIA_SUPPORT_
#endif

#include "pdfium/fpdf_dataavail.h"
#include "pdfium/fpdf_edit.h"
#include "pdfium/fpdf_ext.h"
#include "pdfium/fpdf_formfill.h"
#include "pdfium/fpdf_text.h"
#include "pdfium/fpdfview.h"
#include "image_diff_png.h"
#include "load_support.h"
#include "pdfpageinfo.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef PDF_ENABLE_V8
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"
#endif // PDF_ENABLE_V8

#ifdef PDF_ENABLE_SKIA
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkStream.h"
#endif

#ifdef _WIN32
#define access   _access
#define snprintf _snprintf
#define R_OK     4
#endif

namespace textextract {

	struct FPDF_FORMFILLINFO_PDFiumTest : public FPDF_FORMFILLINFO {
		// Hold a map of the currently loaded pages.
		std::map<int, FPDF_PAGE> loaded_pages;

		// Hold a pointer of FPDF_FORMHANDLE so that PDFium app hooks can
		// make use of it.
		FPDF_FORMHANDLE form_handle;
	};

	class PdfRenderer {
	private:
		// Number of pages of the PDF.
		int mPageCount;
		// Boolean indicating whether or not mFileBuffer was successfully loaded.
		bool mBufferedLoaded = false;
		// Filepath to the PDF to be rendered
		std::string mFilePath;
		// Buffer that holds the PDF file data.
		std::vector<char> mFileBuffer;
		/**
		* Initialize the PDF rendering engine.
		*
		* @returns True if the library successfully initialized.
		*/
		bool init();
		/**
		* Assuming the PDF is successfully initialized, determine its page count.
		*/
		void DeterminePageCount();

	public:
		PdfRenderer(std::string pdfpath);
		~PdfRenderer();
		/**
		* Get the information about a page, such as text, default dimensions, rotation, and render.
		* Sets the information in PageInfo for accessibilty outside of PdfRenderer class.
		* 
		* @param pageNumber Page number from PDF to derive information from.
		* @param dpi Dots Per Inch metric used for rendering page to a desired resolution.
		*/
		void GetPageInfo(int pageNumber, int dpi);
		/**
		* Get the pagecount for the current PDF.
		*
		* @returns the pagecount as an integer.
		*/
		int GetPageCount() { return mPageCount; }
		/**
		* Get the loading status of the PDF buffer
		*
		* @returns True if buffer is loaded.
		*/
		bool BufferLoaded() { return mBufferedLoaded; }
	};
} // namespace textextract
#endif
