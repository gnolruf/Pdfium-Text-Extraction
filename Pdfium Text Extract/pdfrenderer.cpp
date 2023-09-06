#define _CRT_SECURE_NO_WARNINGS
#include "pdfrenderer.h"

#include "textextractutils.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>

using namespace image_diff_png;

namespace textextract {
#pragma region TextExtraction
	std::vector<TextBox> GetTextWithBounds(FPDF_PAGE page) {
		int pagewidth = GetPageSize().GetPageDimensions().width;
		int pageheight = GetPageSize().GetPageDimensions().height;
		if ((pagewidth < pageheight) && GetPageOrientation() == PageOrientation::LANDSCAPE) {
			std::swap(pagewidth, pageheight);
		}
		FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
		std::vector<std::wstring> wordtokens = GetWordTokens(GetRawPageText());
		std::vector<TextBox> pagewordtextboxes =
			GetTextBoxesFromTokens(wordtokens, textpage, pagewidth, pageheight);
		RemoveWaterMarkText(pagewordtextboxes);
		if (GetPageRender().rows != GetPageSize().GetPageDimensions().height
			&& GetPageRender().cols != GetPageSize().GetPageDimensions().width) {
			PageDimensions renderdims(GetPageRender().cols, GetPageRender().rows);
			RescaleTextBoxes(pagewordtextboxes, GetPageSize().GetPageDimensions(), renderdims);
		}
		return pagewordtextboxes;
	}

	std::wstring GetTextRaw(FPDF_PAGE page) {
		std::wstring rawtext;
		FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);

		for (int i = 0; i < FPDFText_CountChars(textpage); i++) {
			rawtext += FPDFText_GetUnicode(textpage, i);
		}

		return rawtext;
	}

#pragma endregion TextExtraction

	// Indicates what PDF utilities are not supported by the renderer.
	void UnsupportedHandler(UNSUPPORT_INFO*, int type) {
		std::string feature = "Unknown";
		switch (type) {
		case FPDF_UNSP_DOC_XFAFORM: feature = "XFA"; break;
		case FPDF_UNSP_DOC_PORTABLECOLLECTION:
			feature = "Portfolios_Packages";
			break;
		case FPDF_UNSP_DOC_ATTACHMENT:
		case FPDF_UNSP_ANNOT_ATTACHMENT: feature = "Attachment"; break;
		case FPDF_UNSP_DOC_SECURITY: feature = "Rights_Management"; break;
		case FPDF_UNSP_DOC_SHAREDREVIEW: feature = "Shared_Review"; break;
		case FPDF_UNSP_DOC_SHAREDFORM_ACROBAT:
		case FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM:
		case FPDF_UNSP_DOC_SHAREDFORM_EMAIL: feature = "Shared_Form"; break;
		case FPDF_UNSP_ANNOT_3DANNOT: feature = "3D"; break;
		case FPDF_UNSP_ANNOT_MOVIE: feature = "Movie"; break;
		case FPDF_UNSP_ANNOT_SOUND: feature = "Sound"; break;
		case FPDF_UNSP_ANNOT_SCREEN_MEDIA:
		case FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA: feature = "Screen"; break;
		case FPDF_UNSP_ANNOT_SIG: feature = "Digital_Signature"; break;
		}
		std::string msg = "Unsupported feature: " + feature + ".";
		printf("%s\n", msg.c_str());
	}

	bool PdfRenderer::init() {

		FPDF_LIBRARY_CONFIG config;
		config.version = 2;
		config.m_pUserFontPaths = nullptr;
		config.m_pIsolate = nullptr;
		config.m_v8EmbedderSlot = 0;

		FPDF_InitLibraryWithConfig(&config);

		UNSUPPORT_INFO unsupported_info;
		memset(&unsupported_info, '\0', sizeof(unsupported_info));
		unsupported_info.version = 1;
		unsupported_info.FSDK_UnSupport_Handler = UnsupportedHandler;

		FSDK_SetUnSpObjProcessHandler(&unsupported_info);

		return true;
	}

	static FPDF_FORMFILLINFO_PDFiumTest*
		ToPDFiumTestFormFillInfo(FPDF_FORMFILLINFO* form_fill_info) {
		return static_cast<FPDF_FORMFILLINFO_PDFiumTest*>(form_fill_info);
	}

	void Add_Segment(FX_DOWNLOADHINTS* hints, size_t offset, size_t size) {}

	FPDF_BOOL Is_Data_Avail(FX_FILEAVAIL* avail, size_t offset, size_t size) {
		return true;
	}

	static bool CheckDimensions(int stride, int width, int height) {
		if (stride < 0 || width < 0 || height < 0) return false;
		if (height > 0 && width > INT_MAX / height) return false;
		return true;
	}

	static cv::Mat
		RenderPage(const void* buffer_void, int stride, int width, int height) {
		if (!CheckDimensions(stride, width, height)) exit(1);

		std::vector<byte> png_encoding;
		const unsigned char* buffer =
			static_cast<const unsigned char*>(buffer_void);
		if (!image_diff_png::EncodeBGRAPNG(
			buffer, width, height, stride, false, &png_encoding)) {
			std::string errMsg = "Failed to convert bitmap to PNG.";
			fprintf(stderr, "%s\n", errMsg.c_str());
			exit(1);
		}

		cv::Mat data_mat(png_encoding, true);
		return cv::imdecode(data_mat, 0);
	}

	PageDimensions CalculateDimensions(int dpi) {
		PageDimensions dims;
		float physical_width = GetPageSize().GetPageDimensions().width / 72.0f;
		float physical_height = GetPageSize().GetPageDimensions().height / 72.0f;
		dims.width = static_cast<int>(physical_width * dpi);
		dims.height = static_cast<int>(physical_height * dpi);
		return dims;
	}

	cv::Mat GetPageRender(FPDF_FORMHANDLE& form, FPDF_PAGE page, int dpi) {
		cv::Mat renderedpage;
		PageDimensions renderedpagedims = CalculateDimensions(dpi);
		int width = renderedpagedims.width;
		int height = renderedpagedims.height;
		int alpha = FPDFPage_HasTransparency(page) ? 1 : 0;
		FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, alpha);
		if (bitmap) {
			FPDF_DWORD fill_color = alpha ? 0x00000000 : 0xFFFFFFFF;
			FPDFBitmap_FillRect(bitmap, 0, 0, width, height, fill_color);
			FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, FPDF_ANNOT);
			FPDF_FFLDraw(form, bitmap, page, 0, 0, width, height, 0, FPDF_ANNOT);
			int stride = FPDFBitmap_GetStride(bitmap);
			const char* buffer =
				reinterpret_cast<const char*>(FPDFBitmap_GetBuffer(bitmap));
			renderedpage = RenderPage(buffer, stride, width, height);
			FPDFBitmap_Destroy(bitmap);
		}
		else {
			std::string errMsg = "Page was too large to be rendered.";
			fprintf(stderr, "%s\n", errMsg.c_str());
		}
		return renderedpage;
	}

	// Using the form data to retrieve data specific for a page.
	FPDF_PAGE
		GetPageForIndex(FPDF_FORMFILLINFO* param, FPDF_DOCUMENT doc, int index) {
		FPDF_FORMFILLINFO_PDFiumTest* form_fill_info =
			ToPDFiumTestFormFillInfo(param);
		auto& loaded_pages = form_fill_info->loaded_pages;

		auto iter = loaded_pages.find(index);
		if (iter != loaded_pages.end()) return iter->second;

		FPDF_PAGE page = FPDF_LoadPage(doc, index);
		if (!page) return nullptr;

		FPDF_FORMHANDLE& form_handle = form_fill_info->form_handle;

		FORM_OnAfterLoadPage(page, form_handle);
		FORM_DoPageAAction(page, form_handle, FPDFPAGE_AACTION_OPEN);

		loaded_pages[index] = page;
		return page;
	}

	void DeterminePageInfo(
		FPDF_DOCUMENT doc, FPDF_FORMHANDLE& form,
		FPDF_FORMFILLINFO_PDFiumTest& form_fill_info, const int page_index, int dpi) {
		FPDF_PAGE page = GetPageForIndex(&form_fill_info, doc, page_index);
		if (page) {
			int pagerotation = FPDFPage_GetRotation(page);
			SetPageRotation(PageRotation(FPDFPage_GetRotation(page)));
			SetPageOrientation(pagerotation == 1 || pagerotation == 3 ?
				PageOrientation::LANDSCAPE : PageOrientation::PORTRAIT);
			SetPageSize(
				FPDF_GetPageWidth(page), FPDF_GetPageHeight(page));
			FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);
			SetRender(GetPageRender(form, page, dpi));
			SetRawPageText(GetTextRaw(page));
			SetPageWords(GetTextWithBounds(page));

			form_fill_info.loaded_pages.erase(page_index);

			FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_CLOSE);
			FORM_OnBeforeClosePage(page, form);
			FPDFText_ClosePage(text_page);
			FPDF_ClosePage(page);
		}
	}

	void PdfRenderer::DeterminePageCount() {
		if (!mFileBuffer.empty()) {
			FPDF_DOCUMENT document = FPDF_LoadMemDocument(mFileBuffer.data(), mFileBuffer.size(), nullptr);
			if (!document) {
				FPDF_DestroyLibrary();
			}
			mPageCount = FPDF_GetPageCount(document);

			FPDF_CloseDocument(document);
		}
		else {
			FPDF_DestroyLibrary();
		}
	}

	// Public
	PdfRenderer::PdfRenderer(std::string pdfpath) {
		if (init()) {
			size_t bufferlength = 0;
			std::unique_ptr<char, pdfium::FreeDeleter> filebuffer =
				GetFileContents(pdfpath.c_str(), &bufferlength);
			if (filebuffer == nullptr) return;
			std::vector<char> data(filebuffer.get(), filebuffer.get() + bufferlength);
			mFileBuffer = data;
			mBufferedLoaded = true;
			DeterminePageCount();
		}
	}

	PdfRenderer::~PdfRenderer() {
		mBufferedLoaded = false;
		FPDF_DestroyLibrary();
	}

	void PdfRenderer::GetPageInfo(int pagenumber, int dpi) {
		if (!mFileBuffer.empty()) {
			FPDF_FORMFILLINFO_PDFiumTest form_callbacks = {};
#ifdef PDF_ENABLE_XFA
			form_callbacks.version = 2;
#else // PDF_ENABLE_XFA
			form_callbacks.version = 1;
#endif // PDF_ENABLE_XFA
			form_callbacks.FFI_GetPage = GetPageForIndex;

			DocLoader loader(mFileBuffer.data(), mFileBuffer.size());
			FPDF_FILEACCESS file_access;
			memset(&file_access, '\0', sizeof(file_access));
			file_access.m_FileLen = static_cast<unsigned long>(mFileBuffer.size());
			file_access.m_GetBlock = DocLoader::GetBlock;
			file_access.m_Param = &loader;

			FX_FILEAVAIL file_avail;
			memset(&file_avail, '\0', sizeof(file_avail));
			file_avail.version = 1;
			file_avail.IsDataAvail = Is_Data_Avail;

			FX_DOWNLOADHINTS hints;
			memset(&hints, '\0', sizeof(hints));
			hints.version = 1;
			hints.AddSegment = Add_Segment;

			ScopedFPDFAvail pdf_avail(FPDFAvail_Create(&file_avail, &file_access));

			ScopedFPDFDocument doc;

			int nRet = PDF_DATA_NOTAVAIL;
			bool bIsLinearized = false;
			if (FPDFAvail_IsLinearized(pdf_avail.get()) == PDF_LINEARIZED) {
				doc.reset(FPDFAvail_GetDocument(pdf_avail.get(), nullptr));
				if (doc) {
					while (nRet == PDF_DATA_NOTAVAIL)
						nRet = FPDFAvail_IsDocAvail(pdf_avail.get(), &hints);

					if (nRet == PDF_DATA_ERROR) {
						std::string errMsg =
							"Unknown error in checking if doc was available.";
						fprintf(stderr, "%s\n", errMsg.c_str());
						FPDF_CloseDocument(doc.get());
					}
					nRet = FPDFAvail_IsFormAvail(pdf_avail.get(), &hints);
					if (nRet == PDF_FORM_ERROR || nRet == PDF_FORM_NOTAVAIL) {
						std::string errMsg = "Error " + std::to_string(nRet) +
							" was returned in checking if form was available.";
						fprintf(stderr, "%s\n", errMsg.c_str());
						FPDF_CloseDocument(doc.get());
					}
					bIsLinearized = true;
				}
			}
			else {
				doc.reset(FPDF_LoadCustomDocument(&file_access, nullptr));
			}

			if (!doc) {
				unsigned long err = FPDF_GetLastError();
				std::string errMsg = "Load pdf docs unsuccessful: ";
				switch (err) {
				case FPDF_ERR_SUCCESS: errMsg += "Success"; break;
				case FPDF_ERR_UNKNOWN: errMsg += "Unknown error"; break;
				case FPDF_ERR_FILE:
					errMsg += "File not found or could not be opened";
					break;
				case FPDF_ERR_FORMAT:
					errMsg += "File not in PDF format or corrupted";
					break;
				case FPDF_ERR_PASSWORD:
					errMsg += "Password required or incorrect password";
					break;
				case FPDF_ERR_SECURITY:
					errMsg += "Unsupported security scheme";
					break;
				case FPDF_ERR_PAGE:
					errMsg += "Page not found or content error";
					break;
				default: errMsg += "Unknown error " + std::to_string(err);
				}
				fprintf(stderr, "%s\n", errMsg.c_str());
			}

			(void)FPDF_GetDocPermissions(doc.get());

			FPDF_FORMHANDLE form =
				FPDFDOC_InitFormFillEnvironment(doc.get(), &form_callbacks);
			form_callbacks.form_handle = form;

#ifdef PDF_ENABLE_XFA
			int doc_type = DOCTYPE_PDF;
			if (
				FPDF_HasXFAField(doc, &doc_type) && doc_type != DOCTYPE_PDF &&
				!FPDF_LoadXFA(doc)) {
				fprintf(stderr, "LoadXFA unsuccessful, continuing anyway.\n");
			}
#endif // PDF_ENABLE_XFA
			FPDF_SetFormFieldHighlightColor(form, 0, 0xFFE4DD);
			FPDF_SetFormFieldHighlightAlpha(form, 100);

			FORM_DoDocumentJSAction(form);
			FORM_DoDocumentOpenAction(form);

			if (bIsLinearized) {
				nRet = PDF_DATA_NOTAVAIL;
				while (nRet == PDF_DATA_NOTAVAIL)
					nRet = FPDFAvail_IsPageAvail(pdf_avail.get(), pagenumber, &hints);

				if (nRet == PDF_DATA_ERROR) {
					std::string errMsg = "Unknown error in checking if page " +
						std::to_string(pagenumber) + " is available.";
					fprintf(stderr, "%s\n", errMsg.c_str());
					FPDFDOC_ExitFormFillEnvironment(form);
					FPDF_CloseDocument(doc.get());
				}
			}
			DeterminePageInfo(doc.get(), form, form_callbacks, pagenumber, dpi);
			FORM_DoDocumentAAction(form, FPDFDOC_AACTION_WC);

			FPDFDOC_ExitFormFillEnvironment(form);
		}
	}
} // namespace textextract