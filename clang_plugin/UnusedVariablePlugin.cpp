#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
//#include "clang/Frontend/PluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
// MaybeUnusedAttr не распознается компилятором, в связи с этим был использован
// UnusedAttr

using namespace clang;

namespace {
class AttributeVisitor : public RecursiveASTVisitor<AttributeVisitor> {
public:
  AttributeVisitor(Rewriter &R, ASTContext &C) : TheRewriter(R), Context(C) {}

  bool VisitVarDecl(VarDecl *D) {
    if (D->isUsed())
      return true;

    SourceLocation Loc = D->getLocation();
    if (Loc.isInvalid() || Context.getSourceManager().isInSystemHeader(Loc))
      return true;

    // Добавление [[gnu::unused]] к неиспользуемой переменной
    if (!D->hasAttr<UnusedAttr>()) {
      auto *unusedAttr = UnusedAttr::CreateImplicit(Context, Loc);
      D->addAttr(unusedAttr);
    }

    return true;
  }

private:
  Rewriter &TheRewriter;
  ASTContext &Context;
};

class AttributeConsumer : public ASTConsumer {
public:
  AttributeConsumer(Rewriter &R, ASTContext &C) : Visitor(R, C) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  AttributeVisitor Visitor;
};

class AttributeAction : public PluginASTAction {
protected:
  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    return true;
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<AttributeConsumer>(TheRewriter, CI.getASTContext());
  }

  void EndSourceFileAction() override {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());

    llvm::errs() << "Finished processing file.\n";
  }

private:
  Rewriter TheRewriter;
};
} // namespace

static FrontendPluginRegistry::Add<AttributeAction>
    X("add-unused-attribute",
      "Add [[gnu::unused]] attribute to unused variables");