#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <experimental/filesystem>
#include <regex>
#include <type_traits>

#include "ast.h"
#include "logger.h"

using namespace std;
namespace fs = std::experimental::filesystem;

namespace ns_ast {


inline char separator() {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}


ostream& operator<<(ostream& os, const ASTNode& node)
{
    os << node.__str();
    return os;
}

CXChildVisitResult astVisitor(CXCursor cursor, CXCursor parent, CXClientData user_data)
{
    ASTNode* parent_node = (reinterpret_cast<ASTNode*>(user_data));
    assert(parent_node);
    parent_node->m_leaves.emplace_back(ASTNode(cursor));
    clang_visitChildren(cursor, astVisitor, &parent_node->m_leaves[parent_node->m_leaves.size() - 1]);
    return CXChildVisit_Continue;
}


ASTNode::ASTNode(CXCursor cursor)
{
    m_kind = cursorKind(cursor);
    m_location = cursorLocation(cursor);
    m_extent = cursorExtent(cursor);
    m_spelling = cursorSpelling(cursor);
    m_USR = cursorUSR(cursor);
    m_display_name = cursorDisplayName(cursor);
}

string ASTNode::__str(string prefix) const
{
    stringstream ss;
    ss << prefix << " ASTNode(kind: " << m_kind <<
          ", spelling: " << m_spelling <<
          ", USR: " << m_USR <<
          ", display_name: " << m_display_name <<
          ", location: " << m_location <<
          ", extent: " << m_extent << ")\n";
    for (const ASTNode& leaf : m_leaves) ss << leaf.__str(prefix + '-');
    return ss.str();
}

string ASTNode::cursorKind(CXCursor cursor)
{
    return cxstring2string(clang_getCursorKindSpelling(clang_getCursorKind(cursor)));
}
string ASTNode::cursorLocation(CXCursor cursor)
{
    CXFile file;
    unsigned int lin;
    unsigned int col;
    unsigned int off;
    clang_getSpellingLocation(clang_getCursorLocation(cursor), &file, &lin, &col, &off);
    stringstream ss;
    ss << cxstring2string(clang_getFileName(file)) << "," << lin << "," << col << "," << off;
    return ss.str();
}
string ASTNode::cursorExtent(CXCursor cursor)
{
    CXSourceRange range = clang_getCursorExtent(cursor);
    stringstream ss;
    ss << range.begin_int_data << "," << range.end_int_data;
    return ss.str();
}
string ASTNode::cursorSpelling(CXCursor cursor)
{
    return cxstring2string(clang_getCursorSpelling(cursor));
}
string ASTNode::cursorUSR(CXCursor cursor)
{
    return cxstring2string(clang_getCursorUSR(cursor));
}
string ASTNode::cursorDisplayName(CXCursor cursor)
{
    return cxstring2string(clang_getCursorDisplayName(cursor));
}
string ASTNode::cursorMangledName(CXCursor cursor)
{
    return cxstring2string(clang_Cursor_getMangling(cursor));
}

string ASTNode::cxstring2string(const CXString& str)
{
    string res;
    const char* str_p = clang_getCString(str);
    if (str_p) res = string(str_p);
    clang_disposeString(str);
    return res;
}


ostream& operator<<(ostream& os, const ASTTU& tu)
{
    os << "ASTTU(file: " << tu.m_file_pathname << ")\n";
    os << tu.m_root << "\n";
    return os;
}


ASTTU::ASTTU(const string& file_pathname, vector<string> cli_args) :
    m_file_pathname(file_pathname)
{
    CXIndex idx = clang_createIndex(0, 1);


    // todo: remove new/delete operators
    int cli_args_num = cli_args.size();
    const char** cli_args_arr = new const char*[cli_args_num];
    transform( cli_args.begin(), cli_args.end(), cli_args_arr, [](const string& arg){return arg.c_str();});

    CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(idx, file_pathname.c_str(), cli_args_num, cli_args_arr, 0, nullptr);
    delete [] cli_args_arr;

    if (!tu)
        throw runtime_error("Can't create TRANSLATION UNIT");

    CXCursor root  = clang_getTranslationUnitCursor(tu);
    if (clang_getCursorKind(root) != CXCursor_TranslationUnit)
        throw runtime_error("Invalid cursor kind");



    // todo: clang_visitChildren -> ASTNode
    m_root = ASTNode(root);
    clang_visitChildren(root, astVisitor, &m_root);


    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(idx);
}


ostream& operator<<(ostream& os, const AST& ast)
{
    os << "AST(project: " << ast.m_project_pathname << ")\n\n";
    for (const ASTTU& tu : ast.m_tu) os << tu;
    return os;
}


AST::AST(const string& project_pathname, vector<string> cli_args) :
    m_project_pathname(fs::canonical(fs::path(project_pathname)).string())
{
    set<string> incs = includes(m_project_pathname);
    for (const string& inc : incs) {
        cli_args.emplace_back(string{"-I" + inc});
    }

    vector<string> src_files = sources(m_project_pathname);
    for (const auto& f : src_files) {
        m_tu.emplace_back(ASTTU(f, cli_args));
    }
}

vector<string> AST::sources(string path)
{
    return findFilesByExtension(path, ".*\\.(c|cc|cpp|cxx|c\\+\\+)$");
}
set<string> AST::includes(string path)
{
    set<string> res{m_project_pathname};

    regex sep_re("(/|\\|\\\\)");
    vector<string> files = findFilesByExtension(path, ".*\\.(h|hh|hpp|hxx|h\\+\\+)$");
    for (const string& file : files) {
        string path{m_project_pathname};
        string rpath = file.substr(m_project_pathname.length());
        vector<string> parts(sregex_token_iterator(rpath.begin(), rpath.end(), sep_re, -1), sregex_token_iterator());
        for (size_t i = 0; i < parts.size() - 1; i++) {
            if (parts[i].length() > 0) {
                path+={separator() + parts[i]};
                res.insert(fs::canonical(fs::path(path)).string());
            }
        }
    }

    return res;
}

vector<string> AST::findFilesByExtension(string path, string extension_pattern)
{
    vector<string> res;

    using iterator = conditional<true, fs::recursive_directory_iterator, fs::directory_iterator>::type;

    const iterator end;
    regex re{extension_pattern};
    for (iterator it{path}; it!=end; ++it) {
        if (fs::is_regular_file(*it) && regex_match(it->path().extension().string(), re)) {
            res.push_back(it->path().string());
        }
    }

    return res;
}


} // namespace ns_ast
