# Pdfium Text Extraction

This project utilizes Pdfium perform text extraction from PDFs in several methods:

Raw text - the text from a PDF, in reading order, without any information about where the text is located on the page.

Text with bounds - the text of a PDF, with each word being assigned a bounding box of where it is located on its respective page.

## Dependencies

Vcpkg is used to install and link dependencies (except for Pdfium.)

A vcpkg manifest is used to manage dependencies excluding Pdfium, which must be built and linked manually.

OpenCV is used for the Rect class to represent the bounding boxes for each word. It is also used for its Mat class to render a PDF page to an image, for debugging purposes.

Boost Program Options are used to configure command line arguements.

Boost Locale is used for some UTF-8 conversion utilities.

Nlohmann-JSON is used for creating JSON files for text with its bounding boxes.

Zlib and LibPNG are both dependencies used with Pdfium for rendering pages to an image file.

## Compilation Requirements

This project was written and built in Windows and a the text extraction utilities require use of widestrings. While the general logic is applicable to any environment, rewrites will be required to work outside of a windows OS. 