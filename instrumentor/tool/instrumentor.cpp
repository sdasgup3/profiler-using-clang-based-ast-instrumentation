//------------------------------------------------------------------------------
// Tooling sample. Demonstrates:
//
// * How to write a simple source tool using libTooling.
// * How to use RecursiveASTVisitor to find interesting AST nodes.
// * How to use the Rewriter API to rewrite the source code.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include <sstream>
#include <string>

#include "Transforms.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace arcmt;

#define DEBUG 0

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");
std::string filename;
std::string funcname;
ASTContext *Ctx;
clang::LangOptions lopt;

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
  MyASTVisitor(Rewriter &R) : TheRewriter(R) {}

  std::string getFunctionBeginInject() {
#if DEBUG
    return "";
#endif
    std::stringstream retVal;
    retVal
        << "\n{\n"
        << " auto __start_instr__ = "
           "std::chrono::high_resolution_clock::now();"
        << "\n"
        << " instrumentor_utils::bookkeeper::callStack.push(instrumentor_"
           "utils::stackVal(\""
        << funcname << "\",__start_instr__));"
        << "\n"
        << " if (0 == instrumentor_utils::bookkeeper::funcProfMap.count(\""
        << funcname << "\")) {"
        << "\n"
        << "     instrumentor_utils::bookkeeper::funcProfMap.insert(pair<const "
           "string, "
           " instrumentor_utils::profileInfo>(\""
        << funcname << "\", instrumentor_utils::profileInfo(\"" << funcname
        << " \")));"
        << "\n"
        << " }"
        << "\n}\n";
    return retVal.str();
  }

  std::string getFunctionEndInject() {
#if DEBUG
    return "";
#endif
    std::stringstream retVal;
    retVal
        << "\n{\n"
        << " auto __end_instr__ = "
           "std::chrono::high_resolution_clock::now();"
        << "\n"
        << " auto __currFuncInfo__ = "
           "instrumentor_utils::bookkeeper::callStack.top();"
        << "\n"
        << " instrumentor_utils::bookkeeper::callStack.pop();"
        << "\n"
        << " if (__currFuncInfo__.funcname != \"" << funcname << "\") {"
        << "\n"
        << "     assert(0 && \"No entry for the exit!!\");"
        << "\n"
        << " }"
        << "\n"
        << " auto __start_instr__ = __currFuncInfo__.start_timepoint;"
        << "\n"
        << " auto __elapsed_time__ = "
           "double(duration_cast<nanoseconds>(__end_instr__ - "
           "__start_instr__).count());"
        << "\n"
        << " if (false == instrumentor_utils::bookkeeper::callStack.empty()) {"
        << "\n"
        << "     auto __parentFuncInfo__ = "
           "instrumentor_utils::bookkeeper::callStack.top();"
        << "\n"
        << "     auto &__profInfo__ = "
           "instrumentor_utils::bookkeeper::funcProfMap[__parentFuncInfo__."
           "funcname];"
        << "\n"
        << "   __profInfo__.childTime += __elapsed_time__;"
        << "\n"
        << " }"
        << "\n"
        << " auto &__profInfo__ = "
           "instrumentor_utils::bookkeeper::funcProfMap[__currFuncInfo__."
           "funcname];"
        << "\n"
        << " __profInfo__.elapsedTime += __elapsed_time__;"
        << "\n"
        << " if (true == instrumentor_utils::bookkeeper::callStack.empty()) {"
        << "\n"
        << "     cout << setw(20) << \"Name\";"
        << "\n"
        << "     cout << setw(20) << \"Self Time (ns)\";"
        << "\n"
        << "     cout << setw(20) << \"Child Time (ns)\";"
        << "\n"
        << "     cout << endl;"
        << "\n"
        << "     cout << setw(60) << "
           "\"----------------------------------------------\";"
        << "\n"
        << "     cout << endl;"
        << "\n"
        << "   for (auto &p : instrumentor_utils::bookkeeper::funcProfMap) {"
        << "\n"
        << "       cout << setw(20) << p.first;"
        << "\n"
        << "       cout << setw(20) << fixed << p.second.elapsedTime - "
           " p.second.childTime;"
        << "\n"
        << "       cout << setw(20) << fixed << p.second.childTime;"
        << "\n"
        << "       cout << endl;"
        << "\n"
        << "   }"
        << "\n"
        << " }"
        << "\n}\n";

    return retVal.str();
  }

  void dumpLoc(const SourceLocation &sl, SourceManager &sm,
               const std::string msg) {
    llvm::outs() << msg << "\n";
    sl.dump(sm);
    llvm::outs() << "\n";
    llvm::outs() << *(sm.getCharacterData(sl));
    llvm::outs() << "\n";
  }

  void InjectBraces(Stmt *stmt, SourceManager &sm) {
    if (!stmt) {
      return;
    }

    SourceLocation start_insertpoint(stmt->getLocStart());
#if DEBUG
    dumpLoc(start_insertpoint, sm, "start SL");
#endif
    SourceLocation end_begin(stmt->getLocEnd());
#if DEBUG
    dumpLoc(end_begin, sm, "Start of end SL");
#endif

    // Check if he if/else statement has a missing closing parenthesis.
    if ('{' != *sm.getCharacterData(start_insertpoint)) {

      // Insert the opening brace '{'
      if (start_insertpoint.isMacroID()) {
        auto p = sm.getImmediateExpansionRange(start_insertpoint);
        TheRewriter.InsertText(p.first, "{", true, true);
      } else {
        TheRewriter.InsertText(start_insertpoint, "{", true, true);
      }

      // Insert the closing brace '}'
      bool isInValid = false;
      int end_offset = 0;
      std::pair<SourceLocation, SourceLocation> p;

      p.first = end_begin;
      while (p.first.isMacroID()) {
        p = sm.getImmediateExpansionRange(p.first);
      }
#if DEBUG
      dumpLoc(p.second, sm, "recursive end of last token SL");
#endif
      // To find the end of the token starting from p.first.
      while (1) {
        SourceLocation test_end_loc(p.first.getLocWithOffset(end_offset));
        const char *c = sm.getCharacterData(test_end_loc, &isInValid);
        assert(false == isInValid && "Check");
#if DEBUG
        llvm::outs() << "Offset:" << end_offset << " Char:" << *c << "\n";
#endif
        if (';' == *c) {
          break;
        }
        end_offset++;
      }
      SourceLocation end_insertpoint(p.first.getLocWithOffset(end_offset));
#if DEBUG
      dumpLoc(end_insertpoint, sm, "Search result of last ; SL");
#endif
      TheRewriter.InsertTextAfterToken(end_insertpoint, "}");
    }
  }

  // Inject at function return.
  bool VisitReturnStmt(ReturnStmt *s) {
    std::stringstream SSFuncEnd(getFunctionEndInject());
    // ReturnStmt *returnStatement = cast<ReturnStmt>(s);
    // dumpLoc(returnStatement->getLocStart(), TheRewriter.getSourceMgr());
    TheRewriter.InsertText(s->getLocStart(), SSFuncEnd.str(), true, true);
    return true;
  }

  // If statements must have braces.
  bool VisitIfStmt(IfStmt *s) {
#if DEBUG
    llvm::outs() << "Visit If\n";
#endif
    Stmt *Then = s->getThen();
    Stmt *Else = s->getElse();
    if (Then && !isa<CompoundStmt>(Then) && isa<ReturnStmt>(Then)) {
      InjectBraces(Then, TheRewriter.getSourceMgr());
    }
    if (Else && !isa<CompoundStmt>(Else) && isa<ReturnStmt>(Else)) {
      InjectBraces(Else, TheRewriter.getSourceMgr());
    }

    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *f) {
    // Only function definitions (with bodies), not declarations.
    if (f->hasBody()) {
      Stmt *FuncBody = f->getBody();

      // Type name as string
      QualType QT = f->getReturnType();
      std::string TypeStr = QT.getAsString();

      // Function name
      DeclarationName DeclName = f->getNameInfo().getName();
      std::string FuncName = DeclName.getAsString();
      funcname = FuncName;
      Ctx = &(f->getASTContext());

      // Add comment before function.
      std::stringstream SSBefore;
      SSBefore << "// Instrumented "
               << "\n";
      SourceLocation ST = f->getSourceRange().getBegin();
      TheRewriter.InsertText(ST, SSBefore.str(), true, true);

      // Inject the following at function begin.
      std::stringstream SSFuncBegin(getFunctionBeginInject());

      TheRewriter.InsertTextAfterToken(FuncBody->getLocStart(),
                                       SSFuncBegin.str());

      // Inject the following at function end.
      std::stringstream SSFuncEnd(getFunctionEndInject());

      TheRewriter.InsertTextAfter(FuncBody->getLocEnd(), SSFuncEnd.str());
    }

    return true;
  }

private:
  Rewriter &TheRewriter;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : Visitor(R) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  bool HandleTopLevelDecl(DeclGroupRef DR) override {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
      //(*b)->dump();
    }
    return true;
  }

private:
  MyASTVisitor Visitor;
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
  void EndSourceFileAction() override {
    SourceManager &SM = TheRewriter.getSourceMgr();

    // Inject include files at the begining.
    auto &buffer = TheRewriter.getEditBuffer(SM.getMainFileID());
    buffer.InsertText(0, "#include \"instrumentor_utils.h\"\n", false);
#if !DEBUG
    // Now emit the rewritten buffer.
    TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
#endif
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    // llvm::errs() << "** Creating AST consumer for: " << file << "\n";
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};

int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, ToolingSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  // ClangTool::run accepts a FrontendActionFactory, which is then used to
  // create new objects implementing the FrontendAction interface. Here we use
  // the helper newFrontendActionFactory to create a default factory that will
  // return a new MyFrontendAction object every time.
  // To further customize this, we could create our own factory class.
  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}

/************* Trials *****************
          SourceLocation _s(Then->getLocStart());
          SourceLocation _e(Then->getLocStart().getLocWithOffset(2));
          SourceLocation e(clang::Lexer::getLocForEndOfToken(_s, 0, SM,
lopt));

          _s.dump(SM);
          llvm::outs() << "\n\n";
          _e.dump(SM);
          llvm::outs() << "\n\n";
          e.dump(SM);
          llvm::outs() << "\n\n";

          SourceLocation end(
              clang::Lexer::getLocForEndOfToken(end_begin, 0, sm, lopt));
      #if DEBUG
          dumpLoc(end, sm);
      #endif
// clang::Lexer::MeasureTokenLength(stmt->getLocEnd(), sm, lopt);
*********************************************/
