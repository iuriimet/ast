#ifndef __AST_H__
#define __AST_H__

#include <iostream>
#include <string>
#include <vector>
#include <set>

#include <clang-c/Index.h>


namespace ns_ast {


class ASTNode
{
public:
    ASTNode() = default;
    ASTNode(CXCursor cursor);
    virtual ~ASTNode() = default;

    friend std::ostream& operator<<(std::ostream& os, const ASTNode& node);

    friend CXChildVisitResult astVisitor(CXCursor cursor, CXCursor parent, CXClientData user_data);

private:
    std::string __str(std::string prefix = "|") const;

    static std::string cursorKind(CXCursor cursor);
    static std::string cursorLocation(CXCursor cursor);
    static std::string cursorExtent(CXCursor cursor);
    static std::string cursorSpelling(CXCursor cursor);
    static std::string cursorUSR(CXCursor cursor);
    static std::string cursorDisplayName(CXCursor cursor);
    static std::string cursorMangledName(CXCursor cursor);

    static std::string cxstring2string(const CXString& str);

    std::string m_kind;
    std::string m_location;
    std::string m_extent;
    std::string m_spelling;
    std::string m_USR; // Unified Symbol Resolution
    std::string m_display_name;
    std::vector<ASTNode> m_leaves;
};


class ASTTU
{
public:
    ASTTU(const std::string& file_pathname, std::vector<std::string> cli_args);
    virtual ~ASTTU() = default;

    friend std::ostream& operator<<(std::ostream& os, const ASTTU& tu);

private:
    std::string m_file_pathname;
    ASTNode m_root;
};


class AST
{
public:
    AST(const std::string& project_pathname, std::vector<std::string> cli_args);
    virtual ~AST() = default;

    friend std::ostream& operator<<(std::ostream& os, const AST& ast);

private:
    std::vector<std::string> sources(std::string path);
    std::set<std::string> includes(std::string path);

    static std::vector<std::string> findFilesByExtension(std::string path, std::string extension_pattern);

    std::string m_project_pathname;
    std::vector<ASTTU> m_tu;
};


} // namespace ns_ast

#endif // __AST_H__
