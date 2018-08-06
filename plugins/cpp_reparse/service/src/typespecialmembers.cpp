#include <algorithm>
#include <iterator>
#include <sstream>
#include <utility>

#include <boost/algorithm/string/regex.hpp>

#include <clang/AST/RecursiveASTVisitor.h>

#include <util/logutil.h>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <cppparser/filelocutil.h>

#include "typespecialmembers.h"

namespace
{

using namespace cc;
using namespace cc::service::language;
using namespace clang;
using namespace llvm;

typedef odb::query<model::File> FileQuery;

typedef std::map<void*, std::string> ASTNodeToIdentifierMapTy;

template <typename It, typename ElemFn>
void printSeparatedList(std::ostream& os_,
                        const std::string& separator_,
                        It begin_, It end_, ElemFn fn_)
{
  while (begin_ != end_)
  {
    fn_(*begin_);

    ++begin_;
    if (begin_ != end_)
      os_ << separator_;
  }
}

void printType(std::ostream& os_, const QualType& type_)
{
  auto printQualifiers = [&os_](const QualType& type)
  {
    if (type.isConstQualified())
      os_ << "const ";
    if (type.isVolatileQualified())
      os_ << "volatile ";
    if (type.isRestrictQualified())
      os_ << "restrict ";
  };

  if (type_->isReferenceType())
    printQualifiers(cast<ReferenceType>(type_)->getPointeeTypeAsWritten());
  else
    printQualifiers(type_);

  os_ << type_.getBaseTypeIdentifier()->getName().str();

  if (type_->isLValueReferenceType())
    os_ << "&";
  else if (type_->isRValueReferenceType())
    os_ << "&&";
}

void printParameters(std::ostream& os_,
                     ASTNodeToIdentifierMapTy& argMap_,
                     FunctionDecl::param_iterator begin_,
                     FunctionDecl::param_iterator end_)
{
  // Set up a mapping for the ParmVarDecls to a dummy name. In the AST,
  // the implicit parameters might not have a name.
  assert(argMap_.empty() && "printParameters sets up argument-name bindings"
                            "but was given a non-empty map.");
  {
    size_t argNum = 1;
    FunctionDecl::param_iterator begin = begin_;

    while (begin != end_)
    {
      ParmVarDecl& var = **begin;

      if (!var.getName().empty())
        argMap_[&var] = var.getName().str();
      else
      {
        argMap_[&var] = "arg" + std::to_string(argNum);
      }

      ++argNum;
      ++begin;
    }

    // Conventionally if there is one argument, it could just be called "rhs".
    if (argMap_.size() == 1)
      argMap_.begin()->second = "rhs";
  }

  printSeparatedList(os_, ", ", begin_, end_,
    [&](ParmVarDecl* var) {
      printType(os_, var->getType());
      if (var->isParameterPack())
        os_ << "...";
      os_ << ' ';
      os_ << argMap_[var];
    });
}

/**
 * Calculate the position where the function's body (or a constructor
 * initializer list if there is any) begins.
 */
model::Position getFunctionBodyOffset(const std::string& code_)
{
  // Typically, out-of-line implicit methods look like this:
  //    const SomeTypeName & SomeTypeName::operator = (Type1& arg1) {}
  // or
  //    SomeTypeName::SomeTypeName(const Type1& arg1, Type2&& arg2) : _f(f_) {}
  // The common part of these expressions is that they contain a :: and some
  // parentheses before a : (if constructor init list) or a { (the body).

  // Simply strip the potential return value and class name, and then seek
  // until the first opening parenthesis (either for argument list or
  // operator()).
  std::size_t pos = code_.find("::");
  pos = code_.find('(', pos);
  std::size_t parenDepth = 1;
  bool found = false;
  char lastStringLiteralStart = '\0';
  while (!found)
  {
    ++pos;

    // Try to match other parentheses or the needed : or { characters.
    // But parameters can contain "::" sequences (like std::string), or {s in
    // the argument list. And it can contain more '(' and ')' because, e.g.
    // function pointer arguments.
    pos = code_.find_first_of("():{'\"", pos);
    assert(pos != std::string::npos && "Syntax error while matching, invalid "
                                       "input given.");

    switch (code_.at(pos))
    {
      case '(':
        if (lastStringLiteralStart == '\0')
          ++parenDepth;
        break;
      case ')':
        if (lastStringLiteralStart == '\0')
          --parenDepth;
        break;
      case '\'':
      case '"':
        if (lastStringLiteralStart == '\0')
          lastStringLiteralStart = code_.at(pos);
        else if (code_.at(pos) == lastStringLiteralStart &&
                 code_.at(pos - 1) != '\\')
          lastStringLiteralStart = '\0';
        break;
      case ':':
      case '{':
        if (parenDepth == 0 && lastStringLiteralStart == '\0')
          // We are interested in the first : or { on the top level.
          found = true;
        break;
      default:
        assert(std::string("Matched a character that was not searched for.")
                 .empty());
    }
  }

  std::size_t newLines = static_cast<std::size_t>(
    std::count(code_.begin(), code_.begin() + pos, '\n'));
  std::size_t newStartColumn;

  std::size_t lastNewLinePos = code_.find_last_of('\n', pos);
  if (lastNewLinePos == std::string::npos)
    newStartColumn = pos + 1; // Convert 0-based position to 1-based column.
  else
    newStartColumn = pos - lastNewLinePos;

  model::Position ret;
  ret.line = newLines;
  ret.column = newStartColumn;
  return ret;
}

/**
 * Collects the implementation of special members of a C++ Record from the AST.
 */
class SpecialMemberCollector
  : public RecursiveASTVisitor<SpecialMemberCollector>
{
  typedef RecursiveASTVisitor<SpecialMemberCollector> Base;

public:

  /**
   * Represents a single special member of a record in visitation.
   */
  struct SpecialMember
  {
    /**
     * A special member's full source text can be composed of multiple strings,
     * instances of TextComponent represent one such substring.
     */
    struct TextComponent
    {
      enum struct Mapping
      {
        Unmapped, /*!< The source code of the special member is generated by
          the visitor, it is not found explicitly in any file. */
        OnFile, /*!< The source code is explicitly available in the file and
          range specified by the instance. */
        ThroughDeclaration /*!< The location of the source code is unknown, but
          can be resolved via CodeCompass' database. */
      };
      Mapping _mapping;

      /**
       * The text of the source code substring. This only contains valuable
       * data post-resolution or if the code is "Unmapped".
       */
      std::string _text;

      /**
       * Path of the file where the text can be read from.
       */
      std::string _file;

      /**
       * The range in the file where the text can be found.
       */
      model::Range _range;

      /**
       * Whether to include an extra ; at the end of _text.
       */
      bool _needsExtraLineEndingSemicolon = false;
      /**
       * Whether to remove any existing ; at the end of _text.
       */
      bool _stripExistingLineEndingSemicolon = false;
      /**
       * Whether to put the entire _text between comment sequences.
       */
      bool _shouldBeCommentedOut = false;
      /**
       * Whether the function's signature should be stripped from _text.
       */
      bool _stripSignature = false;
    };

    enum struct Kind
    {
      Unknown = 0,
      Ctor, /*!< The member is either the default constructor (no arguments),
        or a user-specified custom constructor overload. */
      CopyCtor,
      MoveCtor,
      Asg, /*!< Used for any 'operator=' overloads that are not copy or move
        assignment operators. */
      CopyAsg,
      MoveAsg,
      Dtor
    };
    Kind _kind;

    clang::AccessSpecifier _visibility;
    std::vector<TextComponent> _components;

    /**
     * Specifies if the special member's source code cannot be fetched
     * explicitly in any manner and still need AST-based pretty printing.
     * This member is set to false after visitation has concluded.
     */
    bool _needsImplicitBodyResolution = false;
    ASTNodeToIdentifierMapTy _argumentNames;

    /**
     * The ordering operator of SpecialMembers set the ordering importance
     * based on kind.
     */
    bool operator < (const SpecialMember& rhs) const
    {
      return _kind < rhs._kind;
    }
  };

  typedef std::map<clang::AccessSpecifier,
    std::vector<SpecialMember>>SpecialMemberMap;

  /**
   * Initialize the member-to-code pretty printer that will search the AST for
   * the record with the given name.
   */
  SpecialMemberCollector(ASTContext& AST_, const std::string& recordName_)
    : _locUtil(AST_.getSourceManager()),
      _recordName(recordName_),
      _cxxRecordDecl(nullptr),
      _recordNameDecl(nullptr),
      _currentSpecialMember(nullptr)
  {
    TraverseDecl(AST_.getTranslationUnitDecl());
  }

  bool found() const
  {
    return _cxxRecordDecl;
  }

  SpecialMemberMap& getSpecialMembers()
  {
    return _specialMembers;
  }

  bool shouldVisitImplicitCode() const { return true; }

  bool TraverseCXXRecordDecl(CXXRecordDecl* cxxRecordDecl_)
  {
    if (!found() && cxxRecordDecl_->getName() != _recordName)
      // Make sure to visit the potential inner records to find a match.
      return Base::TraverseCXXRecordDecl(cxxRecordDecl_);

    if (found() && cxxRecordDecl_ != _cxxRecordDecl)
    {
      if (!_recordNameDecl && cxxRecordDecl_->getName() == _recordName &&
          cxxRecordDecl_->isImplicit())
        // In the Clang AST, every CXXRecordDecl contain another one with the
        // same name but with the "implicit" tag, which refers the source
        // location of the record's name itself. (It also scopes the record's
        // name so members of the record to legally use the type name.)
        _recordNameDecl = cxxRecordDecl_;

      // Do not visit other member records.
      return true;
    }

    if (!cxxRecordDecl_->hasDefinition())
      // Do not do anything with forward declarations of the record being
      // searched for. (Note: the _recordNameDecl in the previous branch is
      // exempt from this, in that case, that member should be visited!)
      return true;

    _cxxRecordDecl = cxxRecordDecl_;
    Base::TraverseCXXRecordDecl(cxxRecordDecl_);

    // Ensure that the result container is properly ordered, as it is populated.
    for (auto& v : _specialMembers)
      std::stable_sort(v.second.begin(), v.second.end());

    return false; // Don't visit anything after the found record.
  }

  bool TraverseCXXMethodDecl(CXXMethodDecl* method_)
  {
    if (!found())
      return true;

    if (!method_->isCopyAssignmentOperator() &&
        !method_->isMoveAssignmentOperator() &&
        method_->getOverloadedOperator() != OO_Equal)
      // Don't visit non-special member methods. (TraverseCXXMethodDecl is NOT
      // called for constructors and destructors, only their respectively typed
      // traversers!)
      return true;

    return TraverseASTNode(method_,
      [&]{ return Base::TraverseCXXMethodDecl(method_); });
  }

  bool TraverseCXXConstructorDecl(CXXConstructorDecl* ctor_)
  {
    if (!found())
      return true;

    return TraverseASTNode(ctor_,
      [&]{ return Base::TraverseCXXConstructorDecl(ctor_); });
  }

  bool TraverseCXXDestructorDecl(CXXDestructorDecl* dtor_)
  {
    if (!found())
      return true;

    return TraverseASTNode(dtor_,
      [&]{ return Base::TraverseCXXDestructorDecl(dtor_); });
  }

  bool VisitCXXMethodDecl(CXXMethodDecl* method_)
  {
    if (!found())
      return true;

    // QUESTION: How to ensure that special methods have body even if they are
    // unused in the current TU?

    if (method_->getOverloadedOperator() == OO_Equal)
    {
      if (method_->isCopyAssignmentOperator())
        _currentSpecialMember->_kind = SpecialMember::Kind::CopyAsg;
      else if (method_->isMoveAssignmentOperator())
        _currentSpecialMember->_kind = SpecialMember::Kind::MoveAsg;
      else
        _currentSpecialMember->_kind = SpecialMember::Kind::Asg;
    }
    _currentSpecialMember->_visibility = method_->getAccess();

    SpecialMember::TextComponent source;

    if (!method_->isImplicit())
    {
      SourceManager& manager = method_->getASTContext().getSourceManager();
      SourceRange range = method_->getSourceRange();
      source._mapping = SpecialMember::TextComponent::Mapping::OnFile;
      source._file = manager.getFilename(range.getBegin());
      _locUtil.setRange(range.getBegin(), range.getEnd(), source._range);

      if (method_->isDeletedAsWritten())
      {
        // In this case, add the source range just as so without any further
        // decoration - but as opposed to an explicitly defaulted method, the
        // range of the Decl does NOT contain the closing ";".
        _currentSpecialMember->_components.emplace_back(std::move(source));
      }
      else if (!method_->isExplicitlyDefaulted())
      {
        // If the method is not written as " = default" then a body or a forward
        // declaration exists in the code. This includes the case where the user
        // explicitly deletes ("= delete") the method.
        if (method_->hasInlineBody())
        {
          // If the method has an in-line body defined, it should be presented
          // as-is.
          _currentSpecialMember->_components.emplace_back(std::move(source));
        }
        else
        {
          // Store the forward declaration. The semicolon will be removed,
          // because the next fragment will contain the body copied over from
          // another source location.
          source._stripExistingLineEndingSemicolon = true;
          _currentSpecialMember->_components.emplace_back(std::move(source));

          // If the method does not have an inline body, it might still be in
          // the same file, defined out-of-line.
          auto* defDecl = dyn_cast_or_null<CXXMethodDecl>(
            method_->getDefinition());
          if (defDecl && defDecl != method_)
          {
            // Store the actual definition's range.
            SpecialMember::TextComponent source;
            source._mapping = SpecialMember::TextComponent::Mapping::OnFile;
            SourceRange range = defDecl->getSourceRange();
            source._file = manager.getFilename(range.getBegin());
            _locUtil.setRange(range.getBegin(), range.getEnd(),
                              source._range);

            // The out-of-line definition's range contains a function signature.
            source._stripSignature = true;

            _currentSpecialMember->_components.emplace_back(std::move(source));
          }
          else
          {
            // If it is not in this file and only found via the linker at
            // compilation, postpone the resolution. The body will come from the
            // same forward declaration that we already stored.
            SpecialMember::TextComponent source;

            const SpecialMember::TextComponent& forwardDeclComponent =
              _currentSpecialMember->_components.back();
            source._file = forwardDeclComponent._file;
            source._range = forwardDeclComponent._range;

            source._mapping =
              SpecialMember::TextComponent::Mapping::ThroughDeclaration;
            source._stripSignature = true;

            _currentSpecialMember->_components.emplace_back(std::move(source));
          }
        }
      }
      else
      {
        // Show the declaration line as a comment.
        source._shouldBeCommentedOut = true;
        _currentSpecialMember->_components.emplace_back(std::move(source));

        // Create a new text component for the definition, which body will be
        // printed just as if the default method was implicitly defined.
        SpecialMember::TextComponent sourceBody;
        sourceBody._mapping = SpecialMember::TextComponent::Mapping::Unmapped;
        _currentSpecialMember->_components.emplace_back(std::move(sourceBody));
        _currentSpecialMember->_needsImplicitBodyResolution = true;
      }
    }
    else
    {
      source._mapping = SpecialMember::TextComponent::Mapping::Unmapped;
      _currentSpecialMember->_needsImplicitBodyResolution = true;

      if (method_->isUsed())
        _currentSpecialMember->_components.emplace_back(std::move(source));
      else
      {
        // Only forward declaration of the implicit method exists.
        if (method_->isDeleted())
        {
          // QUESTION: Good to have to show WHY? See SemaDeclCXX.cpp in Clang!
        }

        _currentSpecialMember->_components.emplace_back(std::move(source));
      }
    }

    if (isa<CXXConstructorDecl>(method_) || isa<CXXDestructorDecl>(method_))
      // Ctors and dtors have their own visitors, so the body-AST-to-text
      // logic for MethodDecls does not apply.
      return true;

    implicitBodyTraverseWrapper([&] {
      printType(_workStream, method_->getReturnType());
      _workStream << " operator=("; // Only assignment operators are traversed.
      printParameters(_workStream,
        _currentSpecialMember->_argumentNames,
        method_->param_begin(), method_->param_end());
      _workStream << ')';

      if (Stmt* body = method_->getBody())
      {
        _workStream << '\n';
        TraverseStmt(body);
      }
      else
      {
        if (method_->isDeleted())
          _workStream << " = delete";
        _workStream << ';';
      }
    });

    return true;
  }

  bool VisitCXXConstructorDecl(CXXConstructorDecl* ctor_)
  {
    if (!found())
      return true;

    if (ctor_->isCopyConstructor())
      _currentSpecialMember->_kind = SpecialMember::Kind::CopyCtor;
    else if (ctor_->isMoveConstructor())
      _currentSpecialMember->_kind = SpecialMember::Kind::MoveCtor;
    else
      _currentSpecialMember->_kind = SpecialMember::Kind::Ctor;

    implicitBodyTraverseWrapper([&] {
      _workStream << _cxxRecordDecl->getName().str() << '(';
      printParameters(_workStream,
        _currentSpecialMember->_argumentNames,
        ctor_->param_begin(), ctor_->param_end());
      _workStream << ')';

      if (ctor_->getNumCtorInitializers())
      {
        _workStream << "\n  : ";

        printSeparatedList(_workStream, "\n  , ",
          ctor_->init_begin(), ctor_->init_end(),
          [&](CXXCtorInitializer* init) {
            if (const clang::Type* baseTy = init->getBaseClass())
              _workStream << baseTy->getAsCXXRecordDecl()->getName().str();
            else if (const FieldDecl* field = init->getMember())
              _workStream << field->getName().str();

            _workStream << '(';

            Expr* initExpr = init->getInit();
            if (auto* defInit = dyn_cast<CXXDefaultInitExpr>(initExpr))
            {
              // Initialization literals in the record's definition result in
              // a CXXDefaultInitExpr to be written as initializer. We can
              // retrieve the actual initialization expression by binding
              // through.
              if (auto* initializer =
                  defInit->getField()->getInClassInitializer())
                TraverseStmt(initializer);
            }
            else
              // Otherwise, let the traverser figure out what is used.
              TraverseStmt(initExpr);

            _workStream << ')';
          });
      }

      auto* body = dyn_cast_or_null<CompoundStmt>(ctor_->getBody());
      if (body)
      {
        _workStream << '\n';
        TraverseStmt(body);
      }

      if (!ctor_->getNumCtorInitializers() && !body)
      {
        if (ctor_->isDeleted())
          _workStream << " = delete";

        _workStream << ';';
      }
    });

    return true;
  }

  bool VisitCXXDestructorDecl(CXXDestructorDecl* dtor_)
  {
    if (!found())
      return true;

    _currentSpecialMember->_kind = SpecialMember::Kind::Dtor;

    implicitBodyTraverseWrapper([&] {
      _workStream << '~' << _cxxRecordDecl->getName().str() << "()";

      if (Stmt* body = dtor_->getBody())
      {
        _workStream << '\n';
        TraverseStmt(body);
      }
      else
      {
        if (dtor_->isDeleted())
          _workStream << " = delete";

        _workStream << ';';
      }
    });

    return true;
  }

  bool TraverseCompoundStmt(CompoundStmt* stmt_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    _workStream << "{\n";
    bool b = Base::TraverseCompoundStmt(stmt_);
    _workStream << "}\n";
    return b;
  }

  bool TraverseBinAssign(BinaryOperator* binAsg_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    _workStream << "  ";
    TraverseStmt(binAsg_->getLHS());
    _workStream << " = ";
    TraverseStmt(binAsg_->getRHS());
    _workStream << ";\n";

    return true;
  }

  bool TraverseCXXOperatorCallExpr(CXXOperatorCallExpr* opExpr_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    if (opExpr_->getOperator() == OO_Equal)
    {
      _workStream << "  ";
      TraverseStmt(opExpr_->getArg(0));
      _workStream << " = ";

      auto* rhs = opExpr_->getArg(0);
      if (rhs->isXValue())
        _workStream << "std::move(";
      TraverseStmt(rhs);
      if (rhs->isXValue())
        _workStream << ')';

      _workStream << ";\n";
    }

    return true;
  }

  bool TraverseCXXMemberCallExpr(CXXMemberCallExpr* member_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    _workStream << "  ";

    if (member_->getMethodDecl()->getOverloadedOperator() == OO_Equal)
    {
      auto* memberOperator = dyn_cast<MemberExpr>(member_->getCallee());
      TraverseStmt(memberOperator->getBase()); // LHS of the operator call.
      _workStream << " = ";

      auto* rhs = member_->getArg(0);
      if (rhs->isXValue())
        _workStream << "std::move(";
      TraverseStmt(rhs);
      if (rhs->isXValue())
        _workStream << ')';
    }

    _workStream << ";\n";

    return true;
  }

  bool TraverseIntegerLiteral(IntegerLiteral* lit_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    _workStream << lit_->getValue().toString(10, false);
    return Base::TraverseIntegerLiteral(lit_);
  }

  bool TraverseReturnStmt(ReturnStmt* return_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    _workStream << "  return ";
    bool b = Base::TraverseReturnStmt(return_);
    _workStream << ";\n";
    return b;
  }

  bool TraverseMemberExpr(MemberExpr* member_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    if (!isa<CXXThisExpr>(member_->getBase()))
    {
      // Only print if the member's base (such as "rhs") if is not a local
      // member, as - at least in implicit methods! - the "this->" is redundant.
      TraverseStmt(member_->getBase());
      _workStream << (member_->isArrow() ? "->" : ".");
    }

    if (auto* resolvedField = dyn_cast<FieldDecl>(member_->getMemberDecl()))
      _workStream << resolvedField->getIdentifier()->getName().str();

    return true;
  }

  bool TraverseCXXConstructExpr(CXXConstructExpr* cons_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    // Construction initializer lists either just refer to the other member
    // (this is usually the case with built-in types), or they call a
    // constructor. Putting "std::move()" into an initializer is only viable
    // if the move constructor is called.
    if (cons_->getNumArgs() && cons_->getArg(0)->isXValue())
      _workStream << "std::move(";

    bool b = Base::TraverseCXXConstructExpr(cons_);

    if (cons_->getNumArgs() && cons_->getArg(0)->isXValue())
      _workStream << ')';

    return b;
  }

  bool TraverseDeclRefExpr(DeclRefExpr* ref_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    // In generated methods, the DeclRefs point back to the ParmDecl of the
    // function, but this does not have a name in the AST.
    _workStream << _currentSpecialMember->_argumentNames[ref_->getFoundDecl()];

    return Base::TraverseDeclRefExpr(ref_);
  }

  bool TraverseImplicitCastExpr(ImplicitCastExpr* castExpr_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    if (castExpr_->getCastKind() == CK_UncheckedDerivedToBase)
    {
      // Up-casts to the base class can be explicitly represented in the code
      // as "static_cast", although they happen automatically. If the cast
      // happens through multiple hops, write only the last type the expression
      // was cast to, just as how it'd be written in a source code.
      const CXXBaseSpecifier& base = **(castExpr_->path_end() - 1);

      _workStream << "static_cast<"
                  << base.getType()->getAsCXXRecordDecl()->getName().str()
                  << ">(";
      bool b = Base::TraverseImplicitCastExpr(castExpr_);
      _workStream << ')';
      return b;
    }

    return Base::TraverseImplicitCastExpr(castExpr_);
  }

  bool TraverseUnaryDeref(UnaryOperator* derefOp_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    _workStream << '*';
    return TraverseStmt(derefOp_->getSubExpr());
  }

  bool TraverseCXXThisExpr(CXXThisExpr*)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return true;

    _workStream << "this";
    return true;
  }

  /**
   * Fetch the found record's header line (e.g. "class X : public B") with the
   * same semantics as SpecialMembers are formatted.
   */
  SpecialMember getRecordHead()
  {
    assert(found() && "Cannot get record's head text if record was not found.");

    SpecialMember ret;
    SpecialMember::TextComponent name;
    SourceManager& manager = _cxxRecordDecl->getASTContext().getSourceManager();
    SourceRange range = _recordNameDecl->getSourceRange();
    name._mapping = SpecialMember::TextComponent::Mapping::OnFile;
    name._file = manager.getFilename(range.getBegin());
    _locUtil.setRange(range.getBegin(), range.getEnd(), name._range);
    ret._components.emplace_back(std::move(name));

    return ret;
  }

private:
  cc::parser::FileLocUtil _locUtil;

  /**
   * The name of the record the visitor is searching for.
   */
  const std::string& _recordName;

  /**
   * The Clang AST Node corresponding to the found record.
   */
  CXXRecordDecl* _cxxRecordDecl;

  /**
   * The Clang AST Node corresponding to the location of the found record's
   * name.
   */
  CXXRecordDecl* _recordNameDecl;

  /**
   * The special member that is being visited right now (if not nullptr).
   */
  std::unique_ptr<SpecialMember> _currentSpecialMember;

  /**
   * A stream that is used by body generating visitors to build the source code
   * string optimally.
   */
  std::ostringstream _workStream;

  /**
   * The special members group by visibility that had been collected by the
   * traversal.
   */
  SpecialMemberMap _specialMembers;

  template <class ClangASTNode, typename BaseTraverseCallback>
  bool TraverseASTNode(ClangASTNode* /* node_ */, BaseTraverseCallback base_)
  {
    _currentSpecialMember = std::make_unique<SpecialMember>();
    _currentSpecialMember->_kind =
      SpecialMember::Kind::Unknown;
    _currentSpecialMember->_visibility = AS_none;

    bool b = base_();

    // Insert the visitor's result into the list ordered by kind.
    if (_currentSpecialMember->_kind == SpecialMember::Kind::Unknown)
    {
      LOG(warning) << "Visitor did not populate the result type? "
                      "Skip saving node.";
      return b;
    }

    _specialMembers[_currentSpecialMember->_visibility].emplace_back(
      std::move(*_currentSpecialMember));
    return b;
  }

  template <typename Fn>
  void implicitBodyTraverseWrapper(Fn impl_)
  {
    if (!_currentSpecialMember ||
        !_currentSpecialMember->_needsImplicitBodyResolution)
      return;

    _workStream.str("");

    impl_();

    // Mark the body resolution as done, and save the contents.
    _currentSpecialMember->_needsImplicitBodyResolution = false;
    _currentSpecialMember->_components.back()._text = _workStream.str();
  }
};

template <typename T>
inline void concatTemporaryToVector(
  std::vector<T> from_,
  std::vector<T>& to_)
{
  to_.insert(to_.end(),
    std::make_move_iterator(from_.begin()),
    std::make_move_iterator(from_.end()));
}

SourceTextFragment createFromString(const std::string& str_)
{
  SourceTextFragment apiText;
  apiText.mapped = false;
  apiText.text = str_;
  return apiText;
}

/**
 * Converts the visitor-created SpecialMember's body text to the API's
 * representation.
 */
std::vector<SourceTextFragment> convertSpecialMember(
  const SpecialMemberCollector::SpecialMember& member_,
  const std::map<std::string, std::string> fileIds_)
{
  using Mapping = SpecialMemberCollector::SpecialMember::TextComponent::Mapping;

  std::vector<SourceTextFragment> ret;

  for (const auto& component : member_._components)
  {
    SourceTextFragment apiText;
    apiText.text = component._text;
    apiText.mapped = component._mapping == Mapping::OnFile;
    if (apiText.mapped)
    {
      apiText.file = fileIds_.at(component._file);
      apiText.topLeftOffset.line = component._range.start.line;
      apiText.topLeftOffset.column = component._range.start.column;
    }

    ret.emplace_back(std::move(apiText));
  }

  return ret;
}

} // namespace (anonymous)


namespace cc
{
namespace service
{
namespace language
{

TypeSpecialMemberPrinter::TypeSpecialMemberPrinter(
    odb::database& db_,
    DefinitionSearchFunction defSearch_)
  : _db(db_),
    _transaction(db_),
    _definitionSearch(defSearch_)
{
}

std::vector<SourceTextFragment>
TypeSpecialMemberPrinter::resolveMembersFor(
  const model::CppAstNodePtr astNode_,
  std::shared_ptr<ASTUnit> AST_)
{
  using Mapping = SpecialMemberCollector::SpecialMember::TextComponent::Mapping;

  SpecialMemberCollector smc(AST_->getASTContext(), astNode_->astValue);
  if (!smc.found())
  {
    LOG(warning) << "Clang could not match the found record node in the AST.";
    return {};
  }

  auto fetchSourceText = [&](
      SpecialMemberCollector::SpecialMember::TextComponent& component) {
    switch (component._mapping)
    {
      case Mapping::Unmapped:
        if (component._text.empty())
          LOG(warning) <<
                       "Unmapped code component without body retrieved.";
        break;
      case Mapping::OnFile:
        component._text = getFileSubstring(component._file, component._range);
        break;
      case Mapping::ThroughDeclaration:
        model::FileLoc definition =
          _definitionSearch(component._file, component._range);
        if (!definition.file || !definition.file->id)
        {
          component._mapping = Mapping::Unmapped;
          component._text = "Could not match declaration to definition for "
                            "this forward declaration though \"jump to "
                            "definition\" feature.";
        }
        else
        {
          // If the definition is found, update the mapping to the
          // resolved location and fetch it.
          component._mapping = Mapping::OnFile;
          component._file = definition.file->path;
          component._range = definition.range;

          component._text = getFileSubstring(component._file, component._range);
        }
        break;
    }
  };

  std::vector<SourceTextFragment> ret;

  SpecialMemberCollector::SpecialMember recordHead = smc.getRecordHead();
  fetchSourceText(recordHead._components.front());

  // In case the class' opening bracket is on the same line as the class' name,
  // it will be included in the text fetched.
  auto& headText = recordHead._components.front()._text;
  std::size_t classNameEndPos = headText.find_first_of("\n{");
  if (classNameEndPos != std::string::npos)
    headText = headText.substr(0, classNameEndPos);

  concatTemporaryToVector(convertSpecialMember(recordHead, _fileIds), ret);
  ret.emplace_back(createFromString("\n{\n\n"));

  SpecialMemberCollector::SpecialMemberMap& members = smc.getSpecialMembers();
  for (auto& visibilityGroup : members)
  {
    std::string visText;
    switch (visibilityGroup.first)
    {
      case AS_public:
        visText = "public:\n\n";
        break;
      case AS_protected:
        visText = "protected:\n\n";
        break;
      case AS_private:
        visText = "private:\n\n";
        break;
      default:
        visText = "unknown_visibility:\n\n";
        break;
    }
    ret.emplace_back(createFromString(visText));

    for (auto& member : visibilityGroup.second)
    {
      for (auto& component : member._components)
      {
        // TODO: Strip leading whitespaces for the entire text based on first
        // line's.

        fetchSourceText(component);
        std::string& code = component._text;

        if (code.empty())
        {
          LOG(debug) << "Got empty text for the component after mapping. "
                        "File not found?";
          continue;
        }

        if (component._stripSignature &&
            component._mapping != Mapping::Unmapped)
        {
          auto startOffset = getFunctionBodyOffset(code);

          // The code begins at line 1 in the "code" variable, but not in
          // the source file, this is why "line" has to be added, not set.
          component._range.start.line += startOffset.line;
          component._range.start.column = startOffset.column;
          fetchSourceText(component);
        }

        if (component._stripExistingLineEndingSemicolon ||
            component._needsExtraLineEndingSemicolon ||
            component._shouldBeCommentedOut)
        {
          bool wasLineEnd = code.back() == '\n';
          if (wasLineEnd)
            code.pop_back();

          if (component._stripExistingLineEndingSemicolon)
          {
            auto lastSemicolon = code.find_last_of(';');
            if (lastSemicolon != std::string::npos)
              code.at(lastSemicolon) = ' ';

            // Trim the potential trailing whitespace.
            code.erase(code.find_last_not_of(" \t") + 1);
          }
          else if (component._needsExtraLineEndingSemicolon)
            code.push_back(';');

          if (component._shouldBeCommentedOut)
          {
            std::size_t letter_pos = code.find_first_not_of(" \t");
            if (letter_pos >= 3)
            {
              // Find the first non-whitespace character in the text, and if
              // there is sufficient space, insert the comment marker there,
              // not at the very beginning, i.e.
              //     MyType(const MyType& rhs) = default;
              // becomes
              //  /* MyType(const MyType& rhs) = default; */
              code.replace(letter_pos - 3, 3, "/* ");
              code.append(" */");
            }
            else
            {
              // Otherwise just make it a comment anyways, even if it might
              // look a bit uglier.
              code = std::string("/* ").append(code).append(" */");
            }
          }

          if (wasLineEnd)
            code.push_back('\n');
        }
      }

      concatTemporaryToVector(convertSpecialMember(member, _fileIds), ret);
      ret.emplace_back(createFromString("\n"));
    }
  }

  ret.emplace_back(createFromString("};\n"));
  return ret;
}

std::string TypeSpecialMemberPrinter::getFileSubstring(
  const std::string& filePath_,
  const model::Range& range_)
{
  auto& fileLines = _fileCache[filePath_];

  if (fileLines.empty())
  {
    _transaction([&]() {
      model::FilePtr file =
        _db.query_one<model::File>(FileQuery::path == filePath_);
      if (!file)
      {
        // FIXME: This happens when the target file is accessed at runtime via
        // a symlink, such as /usr/include/c++/6.0.0/... but actually saved
        // in the database as /usr/include/c++/6. In this case the file is not
        // found at this point. For now, let's just bail out.
        LOG(warning) << "Attempted to load file '" << filePath_
          << "' but it was not found in the database.";

        _fileIds[filePath_] = "invalid";
        return;
      }

      file->content.load();
      _fileIds[filePath_] = std::to_string(file->id);
      boost::algorithm::split_regex(fileLines,
        file->content->content,
        boost::regex("(\r|\n|\r\n)"));
    });

    if (fileLines.empty())
      return "";
  }

  assert(fileLines.size() >= range_.end.line &&
         "Input source file is smaller than text fragment range requested.");

  // Fetch the lines from the split source file.
  std::ostringstream content;
  auto sourceCodeIt = fileLines.cbegin();
  std::advance(sourceCodeIt, range_.start.line - 1);
  auto sourceCodeEnd = sourceCodeIt + range_.end.line - range_.start.line + 1;

  std::copy(sourceCodeIt, sourceCodeEnd,
    std::ostream_iterator<std::string>(content, "\n"));
  const std::string& linesRanged = content.str();

  // Make sure the text begins and ends at the requested columns.
  std::size_t lastNewLinePos = linesRanged.find_last_of('\n');
  if (lastNewLinePos == std::string::npos)
    lastNewLinePos = 0;

  return linesRanged.substr(
    (range_.start.column ? range_.start.column - 1 : 0),
    lastNewLinePos +
      (linesRanged.length() - lastNewLinePos + range_.end.column));
}

} // namespace language
} // namespace service
} // namespace cc
