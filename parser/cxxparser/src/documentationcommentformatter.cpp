#include "documentationcommentformatter.h"

#include "clang/AST/CommentVisitor.h"
#include "clang/Basic/SourceManager.h"
#include <sstream>

#include <boost/algorithm/string/trim.hpp>

namespace cc {
namespace parser {
  
namespace {

using namespace clang::comments;

class DeclSearcher
{
public:
  DeclSearcher(clang::ASTContext& ctx, const clang::DeclContext* dctx) :
    _ctx(ctx), _declctx(dctx) {}
  ~DeclSearcher() {}
  const clang::NamedDecl* getMethodOrClassDecl(const std::string& name);
  void tokenizer(const std::string content, std::vector<std::string>& tokens, const std::string delimiters = " ", int count = 0);

private:
  template<typename T>
  const T* nameLookup(const std::string& name, const clang::DeclContext* dctx);
  template<typename T>
  const T* nameLookupFor(const std::string& name, const clang::DeclContext* dctx, clang::Decl::Kind kind);
  template<typename T>
  const T* recursiveNameLookup(const std::string& name, const clang::DeclContext* dctx);
  template<typename T>
  const T* absoluteNameLookup(const std::string& name, const std::vector<std::string>& qualifiers);

  clang::ASTContext& _ctx;
  const clang::DeclContext* _declctx;
};

const clang::NamedDecl* DeclSearcher::getMethodOrClassDecl(const std::string &name)
{
  bool isFunction = false;
  size_t pos = name.find("(");
  std::string oname(name);
  if (pos!=std::string::npos)
  {
    oname = name.substr(0,pos);  //name without ()
    isFunction = true;
  }

  const clang::NamedDecl* result_decl;      //decl to name
  std::vector<std::string> qualifiers;

  tokenizer(oname, qualifiers, ":#");

  if (qualifiers.size()>1)  //not just one identifier
  {
    std::string objectname(qualifiers.back());    //the last is the name of object
    qualifiers.pop_back();

    if (isFunction)
      result_decl = absoluteNameLookup<clang::FunctionDecl>(objectname, qualifiers);
    else
      result_decl = absoluteNameLookup<clang::CXXRecordDecl>(objectname, qualifiers);
  }
  else      //just one identifier
  {
    if (isFunction)
      result_decl = recursiveNameLookup<clang::FunctionDecl>(oname, _declctx);
    else
      result_decl = recursiveNameLookup<clang::CXXRecordDecl>(oname, _declctx);
  }

  return result_decl;
}

/** @brief split string by delimeters with max count strict
 *  @param count max count of tokens in result, (<0 = all)
 */
void DeclSearcher::tokenizer(const std::string content, std::vector<std::string>& tokens, const std::string delimiters, int count)
{
  std::string::size_type lastPos = content.find_first_not_of(delimiters, 0);
  std::string::size_type pos = content.find_first_of(delimiters, lastPos);

  int d = 0;
  while ((std::string::npos != pos || std::string::npos != lastPos) && (count <= 0 || d < count))
  {
    tokens.push_back(content.substr(lastPos, pos - lastPos));
    lastPos = content.find_first_not_of(delimiters, pos);
    pos = content.find_first_of(delimiters, lastPos);
    ++d;
  }
  if (std::string::npos != lastPos) //remains
    tokens.push_back(content.substr(lastPos, std::string::npos - lastPos));
}

/// \brief Lookup for concrete kind, iterate thought all decl in current context.
template<typename T>
const T* DeclSearcher::nameLookupFor(const std::string& name, const clang::DeclContext* dctx, clang::Decl::Kind kind)
{
  for (clang::DeclContext::decl_iterator I = dctx->decls_begin(), E = dctx->decls_end(); I != E; ++I)
    if (I->getKind() == kind && llvm::isa<T>(*I))
    {
      const T* nd = llvm::cast<T>(*I);
      if (nd->getNameAsString() == name)
        return nd;
    }
  return NULL;
}

/// \brief Main lookup
template<typename T>
const T* DeclSearcher::nameLookup(const std::string& name, const clang::DeclContext* dctx)
{
  if (dctx != NULL)
  {
    if (name.find_first_of('~')==0) //is it destructor?
    {
      return nameLookupFor<T>(name, dctx, clang::Decl::CXXDestructor);
    }
    else
    {
      if (dctx->getDeclKind() == clang::Decl::LinkageSpec)
      {
        // "Should not perform lookups into linkage specs!"
        return 0;
      }

      //can't find constructor/destructor, todo
      clang::DeclContext::lookup_const_result lu_result = dctx->lookup(
        _ctx.DeclarationNames.getIdentifier(&_ctx.Idents.get(name)));
      for (const auto& it : lu_result)
      {
        if (llvm::isa<T>(it))
        {
          return llvm::cast<T>(it);
        }
      }

      //if reach this, maybe it is a constructor
      return nameLookupFor<T>(name, dctx, clang::Decl::CXXConstructor);
    }
  }
  return NULL;
}

template<typename T>
const T* DeclSearcher::recursiveNameLookup(const std::string& name, const clang::DeclContext* dctx)
{
  if (dctx != NULL)
  {
    const T* object = nameLookup<T>(name, dctx);

    if (object)
      return object;
    else
      return recursiveNameLookup<T>(name, dctx->getParent());
  }
  return NULL;
}

template<typename T>
const T* DeclSearcher::absoluteNameLookup(const std::string& name, const std::vector<std::string>& qualifiers)
{
  const clang::DeclContext* declctx = _ctx.getTranslationUnitDecl();
  const clang::DeclContext* tempdeclctx;

  for(std::string s : qualifiers)
  {
    tempdeclctx = nameLookup<clang::NamespaceDecl>(s, declctx);
    if (!tempdeclctx)
    {
      tempdeclctx = nameLookup<clang::CXXRecordDecl>(s, declctx);
      if (!tempdeclctx)
        return NULL;
    }
    declctx = tempdeclctx;
  }

  return nameLookup<T>(name, declctx);
}

//for sections
enum States { normal, param, sa, returns };
//for handle std::endl
typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
typedef CoutType& (*StandardEndLine)(CoutType&);

class StreamCollection
{
public:
  StreamCollection() : _state(normal) { }

  std::string getStringOfState(States st) { return getStreamOfState(st).str(); }
  void changeState(States st) { _state = st; }
  States getState() { return _state; }
  std::string str()
  {
    std::ostringstream content;
    content << "<div>" << std::endl;
    content << "<dl>" << std::endl;
    createSection(content, "Description", normal);
    createSection(content, "Parameters", param);
    createSection(content, "Returns", returns);
    createSection(content, "See also", sa);
    content << "</dl>" << std::endl;
    content << "</div>" << std::endl;

    return content.str();
  }

  template <typename T>
  friend StreamCollection& operator <<(StreamCollection& g, const T str);
  friend StreamCollection& operator <<(StreamCollection& g,StandardEndLine manip);

private:
  States _state;
  std::ostringstream _normal, _param, _sa, _returns;

  std::ostringstream& getStreamOfState(States st)
  {
    switch (st)
    {
      case param:   return _param;
      case sa:      return _sa;
      case returns: return _returns;
      case normal:
      default:      return _normal;
    }

    // Make eclipse happy
    return _normal;
  }

  void createSection(std::ostringstream& stream, std::string title, States st)
  {
    std::string content(getStringOfState(st));
    if (!content.empty())
    {
      stream << "<dt>" << std::endl << "<b>" << title << "</b>" << std::endl << "</dt>" << std::endl;
      stream << "<dd>" << std::endl << content << std::endl << "</dd>" << std::endl;
    }
  }
};

template <typename T>
StreamCollection& operator<<(StreamCollection& g, const T str)
{
  g.getStreamOfState(g._state) << str;
  return g;
}

// handle std::endl
StreamCollection& operator<<(StreamCollection& g, StandardEndLine manip)
{
  g.getStreamOfState(g._state) << manip;
  return g;
}

/*
 * based on clang::comments::CommentDumper
 */
class CommentFormatter: public clang::comments::ConstCommentVisitor<CommentFormatter>
{
  StreamCollection o;
  //std::ostringstream o;

  const clang::SourceManager &SM;

  /// The \c FullComment parent of the comment being dumped.
  const clang::comments::FullComment *FC;

  int IndentLevel = 0;

  std::vector<std::pair<std::string, States> > tagStack;
  std::map<std::string, unsigned long long> _params;
  std::map<const void*, model::CppAstNodePtr>& _clang2our;
  std::unordered_set<const void*>&             _newNodes;

  std::string strSafe(const std::string& s);
  void indent();
  void createDocCommentLink(const clang::NamedDecl* decl, std::string& name, std::string other);
  std::string getSignature(const clang::FunctionDecl* decl);
  std::string dumpNode(const clang::Decl* decl);

public:
  CommentFormatter(
      const clang::SourceManager &SM,
      const FullComment *FC,
      std::map<std::string, unsigned long long> &params,
      std::map<const void*, model::CppAstNodePtr>& clang2our,
      std::unordered_set<const void*>& newNodes_) :
    SM(SM), FC(FC), _params(params), _clang2our(clang2our), _newNodes(newNodes_)
  { }

  void formatSubTree(const Comment *C);

  // Inline content.
  void visitTextComment(const TextComment *C);
  void visitInlineCommandComment(const InlineCommandComment *C);
  void visitHTMLStartTagComment(const HTMLStartTagComment *C);
  void visitHTMLEndTagComment(const HTMLEndTagComment *C);

  // Block content.
  void visitParagraphComment(const ParagraphComment *C);
  void visitBlockCommandComment(const BlockCommandComment *C);
  void visitParamCommandComment(const ParamCommandComment *C);
  void visitTParamCommandComment(const TParamCommandComment *C);
  void visitVerbatimBlockComment(const VerbatimBlockComment *C);
  void visitVerbatimBlockLineComment(const VerbatimBlockLineComment *C);
  void visitVerbatimLineComment(const VerbatimLineComment *C);

  void visitFullComment(const FullComment *C);

  /*const char *getCommandName(unsigned CommandID) {
    if (Traits)
      return Traits->getCommandInfo(CommandID)->Name;
    const CommandInfo *Info = CommandTraits::getBuiltinCommandInfo(CommandID);
    if (Info)
      return Info->Name;
    return "<not a builtin command>";
  }*/

  std::string get()
  {
    return o.str();
  }
};

/**
 * Escapes string to be HTML-safe.
 *
 * @param s the string to be escaped
 * @return the escaped string
 */
std::string CommentFormatter::strSafe(const std::string& s)
{
  std::string rv;
  for (char ch : s)
  {
    switch (ch)
    {
    case '\n' : rv.push_back(' '); break;
    case '&'  : rv.push_back('&'); break; //TODO or &amp; ???
    case '<'  : rv += "&lt;"; break;
    case '>'  : rv += "&gt;"; break;
    // Clang converts the html characters to its actual values
    // so for example &#0; is converted to \0 so this sucks.
    case '\0' : rv += "\\0"; break;
    default   : rv.push_back(ch); break;
    }
  }
  return rv;
}

/**
 * Prints proper indentation.
 * The current indentation level is read from the \ref IndentLevel member.
 */
void CommentFormatter::indent() {
  for (int i = 0, e = IndentLevel; i < e; ++i)
    o << "  ";
}

/// \brief Create a link to astnode
void CommentFormatter::createDocCommentLink(
  const clang::NamedDecl *decl, std::string& name, std::string other)
{
  if (_newNodes.find(decl) == _newNodes.end())
    return;

  auto node = _clang2our.at(decl);
  o << "<span class=\"docCommentLink\" data-linkHash=\""
    << node->id
    << "\">"
    << strSafe(name)
    << "</span>"
    << strSafe(other)
    << std::endl;
}

/**
 * This function returns the signature of the given function as string. By
 * signature we mean the return type, name and parameter list with their types.
 * This implementation is copied from cxxparser (see nodeinfocollector.h).
 */
std::string CommentFormatter::getSignature(const clang::FunctionDecl* decl)
{
  std::string s = "";

  s += decl->getReturnType().getAsString() + " " + decl->getNameAsString() + "(";
  auto it = decl->param_begin();
  if( it != decl->param_end() )
  {
    s += (*it)->getType().getAsString();

    for( ++it; it != decl->param_end(); ++it)
    {
      s += ", " + (*it)->getType().getAsString();
    }
  }
  s += ")";

  return s;
}

/**
 * This function returns a string representation of the given declaration. In
 * case of function declaration it is its signature, in case of variable or type
 * it is its name. In other cases an empty string is returned.
 */
std::string CommentFormatter::dumpNode(const clang::Decl* decl)
{
  DeclSearcher dsr(
    FC->getDecl()->getASTContext(), FC->getDecl()->getDeclContext());

  std::string type;
  std::string name;

  if (llvm::isa<clang::FunctionDecl>(decl))
  {
    const clang::FunctionDecl* function = llvm::cast<clang::FunctionDecl>(decl);

    name = getSignature(function);
    type = "Function: ";
  }

  if (llvm::isa<clang::TypeDecl>(decl))
  {
    const clang::TypeDecl* typedecl = llvm::cast<clang::TypeDecl>(decl);

    name = typedecl->getNameAsString();
    type = "Type: ";
  }

  if (llvm::isa<clang::VarDecl>(decl))
  {
    const clang::VarDecl* variable = llvm::cast<clang::VarDecl>(decl);

    name = variable->getType().getAsString() + " " + variable->getNameAsString();
    type = "Variable: ";
  }

  if (llvm::isa<clang::FieldDecl>(decl))
  {
    const clang::FieldDecl* field = llvm::cast<clang::FieldDecl>(decl);

    name = field->getType().getAsString() + " " + field->getNameAsString();
    type = "FieldDecl: ";
  }
  return "<div class=\"node\">" + name + "</div>";
}

/**
 * Formats a comment subtree into HTML
 *
 * @param C the root of the comment subtree to be formatted.
 */
void CommentFormatter::formatSubTree(const Comment *C) {
  if (C) {
    size_t tagStackSizeAtBegin = tagStack.size();

    visit(C);

    ++IndentLevel;
    for (Comment::child_iterator I = C->child_begin(),
                                 E = C->child_end();
         I != E; ++I) {
      formatSubTree(*I);
    }

    --IndentLevel;
    //put end tags
    while (tagStackSizeAtBegin < tagStack.size()) {
      std::pair<std::string, States> tagToClose = tagStack.back();
      tagStack.pop_back();
      indent();
      o.changeState(tagToClose.second);
      o << "</" << tagToClose.first << ">" << std::endl;
    }

    o.changeState(normal);
  } else {
    indent();
    o << "(NULL)" << std::endl;
  }
}

// ------------------------------------------------------------
// -                      INLINE CONTENT                      -
// ------------------------------------------------------------

/**
 * This visitor runs when reaching a text body. This happens when visiting a new
 * paragraph or the text of a Doxygen command
 * (e.g. "Text part" in "\param variable Text part").
 */
void CommentFormatter::visitTextComment(const TextComment *C)
{
  indent();
  if (o.getState() == sa)
  {
    DeclSearcher dsr(
      FC->getDecl()->getASTContext(), FC->getDecl()->getDeclContext());

    std::string str(C->getText().str());
    boost::trim(str);

    createDocCommentLink(dsr.getMethodOrClassDecl(str), str, "");
  } else
    o << strSafe(C->getText().str()) << std::endl;
}

void CommentFormatter::visitInlineCommandComment(const InlineCommandComment *C)
{
  //o << "(command " << C->getCommandName().str() << ")" << std::endl;
  indent();
  o << "<!-- inlinecommandcomment -->" << std::endl;
}

/**
 * This visitor runs when a HTML tag is reached in the documentation. The
 * content of the tag is not visited here, just the tag itself. If the tag has
 * a closing counterpart then that will be visited in visitHTMLEndTagComment().
 */
void CommentFormatter::visitHTMLStartTagComment(const HTMLStartTagComment *C)
{
  indent();
  o << "<" << C->getTagName().str();
  if (C->isSelfClosing()) {
    o << " /";
  } else {
    ++IndentLevel;
  }
  o << ">";
}

/**
 * This visitor runs when a closing HTML tag comes.
 */
void CommentFormatter::visitHTMLEndTagComment(const HTMLEndTagComment *C)
{
  --IndentLevel;
  indent();
  o << "</" << C->getTagName().str() << ">";
}


// ------------------------------------------------------------
// -                      BLOCK CONTENT                       -
// ------------------------------------------------------------

/**
 * This visitor runs when a new paragraph begins. This happens at the beginning
 * of a text block or when a new block is separated by an empty line. The text
 * itself is not meant to be the part of the visited documentation, it just
 * signs that a new paragraph is about to begin.
 */
void CommentFormatter::visitParagraphComment(const ParagraphComment *C)
{
  indent();
  std::string tag = o.getState() == param ? "dd" : "p";
  o << "<" << tag << ">" << std::endl;
  tagStack.push_back(std::make_pair(tag, o.getState()));
}

/**
 * This visitor runs when reaching a Doxygen command (e.g. \returns, @throws,
 * etc.).
 * \param C Through this variable one can retrieve the command.
 */
void CommentFormatter::visitBlockCommandComment(const BlockCommandComment *C)
{
  indent();
  std::string name = C->getCommandName(
    FC->getDecl()->getASTContext().getCommentCommandTraits()).str();

  if (name == "return" || name == "returns")
    o.changeState(returns);
  else if (name == "see" || name == "sa")
    o.changeState(sa);
}

/**
 * This visitor runs then reaching the parameter of a Doxygen command
 * (e.g. in "\param variable TextBlock" this parameter is "variable").
 * \param C Through this variable one can retrieve the parameter.
 */
void CommentFormatter::visitParamCommandComment(const ParamCommandComment *C)
{
  // Workaround for artf395380: if there is a malfored param (only a param
  // without the parameter name and description) then the getParamNameAsWritten
  // method will fire an assert and a parser will crash.
  if (C->getNumArgs() == 0) { 
    return;
  }

  o.changeState(param);

  indent();

  ++IndentLevel; indent();

  std::string name = C->getParamNameAsWritten().data();
  auto item = _params.find(name);

  o << "<dl class=\"param\">" << std::endl;
  o << "<dt>" << std::endl;

  tagStack.push_back(std::make_pair("dl", param));

  if (C->isParamIndexValid() && item != _params.end())  //valid
    o << "<span class=\"paramname\">"
      << "<span class=\"docCommentLink\" data-linkHash=\""
      << item->second
      << "\">"
      << name
      << "</span></span>"
      << std::endl;
  else  //invalid
    o << "<span class=\"paramname\">"
      << name
      << "</span>"
      << std::endl;

  o << "</dt>" << std::endl;

  --IndentLevel;
}

void CommentFormatter::visitTParamCommandComment(const TParamCommandComment *C)
{
  o.changeState(param);
  indent();
  o << "<!-- tparamcomment -->" << std::endl;
}

void CommentFormatter::visitVerbatimBlockComment(const VerbatimBlockComment *C)
{
  indent();
  o << "<pre>" << std::endl;
  tagStack.push_back(std::make_pair("pre", o.getState()));
}

void CommentFormatter::visitVerbatimBlockLineComment(const VerbatimBlockLineComment *C)
{
  //no indent here!
  o << strSafe(C->getText().str()) << std::endl;
}

void CommentFormatter::visitVerbatimLineComment(const VerbatimLineComment *C)
{
  indent();
  o << "<pre>" << std::endl;

  std::string cname = C->getCommandName(FC->getDecl()->getASTContext().getCommentCommandTraits()).str();
  if (cname=="ref")
  {
    std::string name, text;
    std::istringstream iss(C->getText().str());
    iss >> name;      //first word of string
    std::ostringstream oss;
    oss << iss.rdbuf();   //other content of string
    text = oss.str();

    DeclSearcher dsr(FC->getDecl()->getASTContext(), FC->getDecl()->getDeclContext());
    createDocCommentLink(dsr.getMethodOrClassDecl(name), name, text);
  }
  else
    o << strSafe(C->getText().str()) << std::endl;
  o << "</pre>" << std::endl;
}


void CommentFormatter::visitFullComment(const FullComment *C)
{
  indent();
  o.changeState(normal);
  o << dumpNode(C->getDecl()) << std::endl;
}

}


std::string DocumentationCommentFormatter::format(
    const clang::SourceManager &SM,
    clang::comments::FullComment *fc,
    std::map<std::string, unsigned long long> &params,
    CxxParseSession& session)
{
  CommentFormatter F(SM, fc, params, session.clang2our, session.newNodes);
  F.formatSubTree(fc);
  return F.get();
}


} // parser  
} // cc
