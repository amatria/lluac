#include <lluac/Config.inc>
#include <lluac/Lexer.h>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/WithColor.h>

static llvm::cl::OptionCategory LluacArgsCategory{"Compiler Options"};
static llvm::cl::opt<std::string> InputFilenameOpt{
    llvm::cl::Positional, llvm::cl::desc("<input file>"), llvm::cl::Required};

static void PrintVersion(llvm::raw_ostream &outs) {
  outs << llvm::formatv("lluac v{0}.{1}.{2}\n", LLUAC_VERSION_MAJOR,
                        LLUAC_VERSION_MINOR, LLUAC_VERSION_PATCH);
}

int main(int argc, char **argv) {
  llvm::InitLLVM X{argc, argv};
  llvm::setBugReportMsg("PLEASE submit a bug report to " LLUAC_BUG_REPORT_URL
                        " and include the crash backtrace, preprocessed "
                        "source, and associated run script.\n");

  llvm::cl::SetVersionPrinter(PrintVersion);
  llvm::cl::HideUnrelatedOptions(LluacArgsCategory);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  llvm::SourceMgr sourceMgr;
  {
    llvm::StringRef inputFilename = InputFilenameOpt.getValue();
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> inputFile =
        llvm::MemoryBuffer::getFile(inputFilename, true);
    if (std::error_code errorCode = inputFile.getError()) {
      llvm::WithColor::error(llvm::errs())
          << "could not open input file '" << inputFilename
          << "': " << errorCode.message() << '\n';
      return 1;
    }
    sourceMgr.AddNewSourceBuffer(std::move(inputFile.get()), {});
  }

  lluac::Lexer lexer{sourceMgr};
  for (;;) {
    lluac::Lexer::Result result = lexer.lex();
    if (result.Token == lluac::Lexer::Token::EndOfFile) {
      break;
    }
  }

  return 0;
}
