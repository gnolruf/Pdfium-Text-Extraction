#include "pagerange.h"
#include "pdfrenderer.h"
#include "outpututils.h"
#include "textextractutils.h"

#include <iostream>

#undef snprintf
#include <boost/program_options.hpp>

using namespace textextract;
namespace po = boost::program_options;

po::options_description GetOptions(bool& textonly) {
	po::options_description desc("Options");
	desc.add_options()
		("help,h", "Produce help message.")
		("filepath,f", po::value<std::string>(), "Path to the PDF file to process.")
		("dpi, dpi", po::value<int>()->default_value(300), "Resolution of page render in dots per inch.")
		("pagerange,p", po::value<std::string>()->default_value("0"), "Range of pages to process, all pages processed by default.")
		("outputlocation,o", po::value<std::string>()->default_value(""), "Path to the output directory to write results to.")
		("TEXT_ONLY", po::bool_switch(&textonly), "Only write text without bounds to file.");
	return desc;
}

int main(int argc, char* argv[]) {
	bool textonly = false;
	auto desc = GetOptions(textonly);
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << "\n";
		return 1;
	}

	std::filesystem::path filepath(vm["filepath"].as<std::string>());
	std::filesystem::path outputpath = vm["outputlocation"].as<std::string>() == "" ?
		filepath.parent_path() : vm["outputlocation"].as<std::string>();
	
	PdfRenderer pdf(filepath.string());
	if (!pdf.BufferLoaded()) {
		std::cerr << "Failed to load the PDF from path." << std::endl;
		exit(EXIT_FAILURE);
	}

	PageRange pages(vm["pagerange"].as<std::string>(), pdf.GetPageCount());
	for (int i = pages.firstpage - 1; i < pages.lastpage; i++) {
		pdf.GetPageInfo(i, vm["dpi"].as<int>());
#ifdef _DEBUG
		DebugTextBoxes(GetPageRender(), GetPageWords());
#endif // DEBUG
		WriteOutput(outputpath, i + 1, filepath.stem().string(), textonly);
		ClearPageInfo();
	}
	return 0;
}