#pragma once

#include "sl_CodeWriter.h"
#include "sl_Tree.h"

namespace sl
{
class HLSLTree;
struct HLSLFunction;
struct HLSLStruct;

/**
 * This class is used to generate HLSL which is compatible with the D3D9
 * compiler (i.e. no cbuffers).
 */
class MSLGenerator
{
public:
    enum Target
    {
        Target_VertexShader,
        Target_PixelShader,
    };

    explicit MSLGenerator(Allocator* allocator);

    bool Generate(HLSLTree* tree, Target target, const char* entryName, std::string* code);
    const char* GetResult() const;

private:
    static const char* GetAttributeName(HLSLAttributeType attributeType);
    static const char* GetTypeName(const HLSLType& type);

    void OutputExpressionList(HLSLExpression* expression);
    void OutputExpression(HLSLExpression* expression);
    void OutputArguments(HLSLArgument* argument);
    void OutputAttributes(int indent, HLSLAttribute* attribute);
    void OutputStatements(int indent, HLSLStatement* statement);
    void OutputDeclaration(HLSLDeclaration* declaration);
    void OutputDeclaration(const HLSLType& type, const char* name, const char* semantic = NULL, const char* registerName = NULL, HLSLExpression* defaultValue = NULL);
    void OutputDeclarationType(const HLSLType& type);
    void OutputDeclarationBody(const HLSLType& type, const char* name, const char* semantic = NULL, const char* registerName = NULL, HLSLExpression* assignment = NULL);

    /** Generates a name of the format "base+n" where n is an integer such that the name
     * isn't used in the syntax tree. */
    bool ChooseUniqueName(const char* base, char* dst, int dstLength) const;

private:
    CodeWriter m_writer;

    const HLSLTree* m_tree;
    const char* m_entryName;
    Target m_target;
    bool m_isInsideBuffer;

    struct
    tex_t
    {
        std::string name;
        HLSLBaseType type;
        unsigned unit;
    };
    std::vector<tex_t> _tex;
};

} // namespace sl
