#include "sl_Common.h"

#include "sl_GeneratorMSL.h"
#include "sl_Parser.h"
#include "sl_Tree.h"

#include "Render/RHI/rhi_Type.h"

namespace sl
{
const char* MSLGenerator::GetTypeName(const HLSLType& type)
{
    switch (type.baseType)
    {
    case HLSLBaseType_Void:
        return "void";
    case HLSLBaseType_Float:
        return "float";
    case HLSLBaseType_Float2:
        return "vector_float2";
    case HLSLBaseType_Float3:
        return "vector_float3";
    case HLSLBaseType_Float4:
        return "vector_float4";
    case HLSLBaseType_Float3x3:
        return "float3x3";
    case HLSLBaseType_Float4x4:
        return "float4x4";
    case HLSLBaseType_Half:
        return "half";
    case HLSLBaseType_Half2:
        return "half2";
    case HLSLBaseType_Half3:
        return "half3";
    case HLSLBaseType_Half4:
        return "half4";
    case HLSLBaseType_Half3x3:
        return "half3x3";
    case HLSLBaseType_Half4x4:
        return "half4x4";
    case HLSLBaseType_Bool:
        return "bool";
    case HLSLBaseType_Int:
        return "int";
    case HLSLBaseType_Int2:
        return "int2";
    case HLSLBaseType_Int3:
        return "int3";
    case HLSLBaseType_Int4:
        return "int4";
    case HLSLBaseType_Uint:
        return "uint";
    case HLSLBaseType_Uint2:
        return "uint2";
    case HLSLBaseType_Uint3:
        return "uint3";
    case HLSLBaseType_Uint4:
        return "uint4";
    case HLSLBaseType_Texture:
        return "texture";
    case HLSLBaseType_Sampler:
        return "sampler";
    case HLSLBaseType_Sampler2D:
        return "sampler2D";
    case HLSLBaseType_Sampler3D:
        return "sampler3D";
    case HLSLBaseType_SamplerCube:
        return "samplerCUBE";
    case HLSLBaseType_Sampler2DShadow:
        return "sampler2DShadow";
    case HLSLBaseType_Sampler2DMS:
        return "sampler2DMS";
    case HLSLBaseType_UserDefined:
        return type.typeName;
    }
    return "?";
}

static int GetFunctionArguments(HLSLFunctionCall* functionCall, HLSLExpression* expression[], int maxArguments)
{
    HLSLExpression* argument = functionCall->argument;
    int numArguments = 0;
    while (argument != NULL)
    {
        if (numArguments < maxArguments)
        {
            expression[numArguments] = argument;
        }
        argument = argument->nextExpression;
        ++numArguments;
    }
    return numArguments;
}

MSLGenerator::MSLGenerator(Allocator* allocator)
{
    m_tree = NULL;
    m_entryName = NULL;
    m_target = Target_VertexShader;
    m_isInsideBuffer = false;
}

// @@ We need a better way of doing semantic replacement:
// - Look at the function being generated.
// - Return semantic, semantics associated to fields of the return structure, or output arguments, or fields of structures associated to output arguments -> output semantic replacement.
// - Semantics associated input arguments or fields of the input arguments -> input semantic replacement.
static const char* TranslateSemantic(const char* semantic, bool output, MSLGenerator::Target target)
{
    if (target == MSLGenerator::Target_VertexShader)
    {
        if (output)
        {
            if (String_Equal("POSITION", semantic))
                return "SV_Position";
        }
    }
    else if (target == MSLGenerator::Target_PixelShader)
    {
        if (output)
        {
            if (String_Equal("DEPTH", semantic))
                return "SV_Depth";
            if (String_Equal("COLOR", semantic))
                return "SV_Target";
            if (String_Equal("COLOR0", semantic))
                return "SV_Target0";
            if (String_Equal("COLOR1", semantic))
                return "SV_Target1";
            if (String_Equal("COLOR2", semantic))
                return "SV_Target2";
            if (String_Equal("COLOR3", semantic))
                return "SV_Target3";
        }
        else
        {
            if (String_Equal("VPOS", semantic))
                return "SV_Position";
            if (String_Equal("VFACE", semantic))
                return "SV_IsFrontFace"; // bool   @@ Should we do type replacement too?
        }
    }
    return NULL;
}

bool MSLGenerator::Generate(HLSLTree* tree, Target target, const char* entryName, std::string* code)
{
    m_tree = tree;
    m_entryName = entryName;
    m_target = target;
    m_isInsideBuffer = false;

    m_writer.Reset(code);
    _tex.clear();

    const char* mtl_define[] =
    {

      "#include <metal_stdlib>",
      "#include <metal_graphics>",
      "#include <metal_matrix>",
      "#include <metal_geometric>",
      "#include <metal_math>",
      "#include <metal_texture>",
      "using namespace metal;",

      "inline vector_float4 mul( vector_float4 v, float4x4 m ) { return m*v; }",
      "inline vector_float4 mul( float4x4 m, vector_float4 v ) { return v*m; }",
      "inline vector_float3 mul( vector_float3 v, float3x3 m ) { return m*v; }",

      "inline float  lerp( float a, float b, float t ) { return mix( a, b, t ); }",
      "inline vector_float2 lerp( vector_float2 a, vector_float2 b, float t ) { return mix( a, b, t ); }",
      "inline vector_float3 lerp( vector_float3 a, vector_float3 b, float t ) { return mix( a, b, t ); }",
      "inline vector_float4 lerp( vector_float4 a, vector_float4 b, float t ) { return mix( a, b, t ); }",

      ""
    };

    for (unsigned i = 0; i != countof(mtl_define); ++i)
        m_writer.WriteLine(0, mtl_define[i]);

    HLSLRoot* root = m_tree->GetRoot();
    OutputStatements(0, root->statement);

    m_tree = NULL;
    return true;
}

const char* MSLGenerator::GetResult() const
{
    return m_writer.GetResult();
}

void MSLGenerator::OutputExpressionList(HLSLExpression* expression)
{
    int numExpressions = 0;
    while (expression != NULL)
    {
        if (numExpressions > 0)
        {
            m_writer.Write(", ");
        }
        OutputExpression(expression);
        expression = expression->nextExpression;
        ++numExpressions;
    }
}

void MSLGenerator::OutputExpression(HLSLExpression* expression)
{
    if (expression->nodeType == HLSLNodeType_IdentifierExpression)
    {
        HLSLIdentifierExpression* identifierExpression = static_cast<HLSLIdentifierExpression*>(expression);
        const char* name = identifierExpression->name;

        if (IsSamplerType(identifierExpression->expressionType) && identifierExpression->global)
        {
            if (identifierExpression->expressionType.baseType == HLSLBaseType_Sampler2D || identifierExpression->expressionType.baseType == HLSLBaseType_SamplerCube)
            {
                m_writer.Write("%s_texture.sample( %s_sampler ", name, name);
            }
        }
        else
        {
            m_writer.Write("%s", name);
        }
    }
    else if (expression->nodeType == HLSLNodeType_CastingExpression)
    {
        HLSLCastingExpression* castingExpression = static_cast<HLSLCastingExpression*>(expression);
        const char* tname = GetTypeName(castingExpression->type);

        if (strstr(tname, "vector_float"))
        {
            OutputDeclaration(castingExpression->type, "");
            m_writer.Write("(");
            OutputExpression(castingExpression->expression);
            m_writer.Write(")");
        }
        else
        {
            m_writer.Write("(");
            OutputDeclaration(castingExpression->type, "");
            m_writer.Write(")(");
            OutputExpression(castingExpression->expression);
            m_writer.Write(")");
        }
    }
    else if (expression->nodeType == HLSLNodeType_ConstructorExpression)
    {
        HLSLConstructorExpression* constructorExpression = static_cast<HLSLConstructorExpression*>(expression);
        m_writer.Write("%s(", GetTypeName(constructorExpression->type));
        OutputExpressionList(constructorExpression->argument);
        m_writer.Write(")");
    }
    else if (expression->nodeType == HLSLNodeType_LiteralExpression)
    {
        HLSLLiteralExpression* literalExpression = static_cast<HLSLLiteralExpression*>(expression);
        switch (literalExpression->type)
        {
        case HLSLBaseType_Half:
        case HLSLBaseType_Float:
        {
            // Don't use printf directly so that we don't use the system locale.
            char buffer[64];
            String_FormatFloat(buffer, sizeof(buffer), literalExpression->fValue);
            m_writer.Write("%s", buffer);
        }
        break;
        case HLSLBaseType_Int:
            m_writer.Write("%d", literalExpression->iValue);
            break;
        case HLSLBaseType_Bool:
            m_writer.Write("%s", literalExpression->bValue ? "true" : "false");
            break;
        default:
            DVASSERT(0);
        }
    }
    else if (expression->nodeType == HLSLNodeType_UnaryExpression)
    {
        HLSLUnaryExpression* unaryExpression = static_cast<HLSLUnaryExpression*>(expression);
        const char* op = "?";
        bool pre = true;
        switch (unaryExpression->unaryOp)
        {
        case HLSLUnaryOp_Negative:
            op = "-";
            break;
        case HLSLUnaryOp_Positive:
            op = "+";
            break;
        case HLSLUnaryOp_Not:
            op = "!";
            break;
        case HLSLUnaryOp_PreIncrement:
            op = "++";
            break;
        case HLSLUnaryOp_PreDecrement:
            op = "--";
            break;
        case HLSLUnaryOp_PostIncrement:
            op = "++";
            pre = false;
            break;
        case HLSLUnaryOp_PostDecrement:
            op = "--";
            pre = false;
            break;
        }
        m_writer.Write("(");
        if (pre)
        {
            m_writer.Write("%s", op);
            OutputExpression(unaryExpression->expression);
        }
        else
        {
            OutputExpression(unaryExpression->expression);
            m_writer.Write("%s", op);
        }
        m_writer.Write(")");
    }
    else if (expression->nodeType == HLSLNodeType_BinaryExpression)
    {
        HLSLBinaryExpression* binaryExpression = static_cast<HLSLBinaryExpression*>(expression);
        //        m_writer.Write("(");
        OutputExpression(binaryExpression->expression1);
        const char* op = "?";
        switch (binaryExpression->binaryOp)
        {
        case HLSLBinaryOp_Add:
            op = " + ";
            break;
        case HLSLBinaryOp_Sub:
            op = " - ";
            break;
        case HLSLBinaryOp_Mul:
            op = " * ";
            break;
        case HLSLBinaryOp_Div:
            op = " / ";
            break;
        case HLSLBinaryOp_Less:
            op = " < ";
            break;
        case HLSLBinaryOp_Greater:
            op = " > ";
            break;
        case HLSLBinaryOp_LessEqual:
            op = " <= ";
            break;
        case HLSLBinaryOp_GreaterEqual:
            op = " >= ";
            break;
        case HLSLBinaryOp_Equal:
            op = " == ";
            break;
        case HLSLBinaryOp_NotEqual:
            op = " != ";
            break;
        case HLSLBinaryOp_Assign:
            op = " = ";
            break;
        case HLSLBinaryOp_AddAssign:
            op = " += ";
            break;
        case HLSLBinaryOp_SubAssign:
            op = " -= ";
            break;
        case HLSLBinaryOp_MulAssign:
            op = " *= ";
            break;
        case HLSLBinaryOp_DivAssign:
            op = " /= ";
            break;
        case HLSLBinaryOp_And:
            op = " && ";
            break;
        case HLSLBinaryOp_Or:
            op = " || ";
            break;
        default:
            DVASSERT(0);
        }
        m_writer.Write("%s", op);
        OutputExpression(binaryExpression->expression2);
        //        m_writer.Write(")");
    }
    else if (expression->nodeType == HLSLNodeType_ConditionalExpression)
    {
        HLSLConditionalExpression* conditionalExpression = static_cast<HLSLConditionalExpression*>(expression);
        m_writer.Write("((");
        OutputExpression(conditionalExpression->condition);
        m_writer.Write(")?(");
        OutputExpression(conditionalExpression->trueExpression);
        m_writer.Write("):(");
        OutputExpression(conditionalExpression->falseExpression);
        m_writer.Write("))");
    }
    else if (expression->nodeType == HLSLNodeType_MemberAccess)
    {
        HLSLMemberAccess* memberAccess = static_cast<HLSLMemberAccess*>(expression);
#if 0        
        m_writer.Write("(");
        OutputExpression(memberAccess->object);
        m_writer.Write(").%s", memberAccess->field);
#else
        OutputExpression(memberAccess->object);
        m_writer.Write(".%s", memberAccess->field);
#endif
    }
    else if (expression->nodeType == HLSLNodeType_ArrayAccess)
    {
        HLSLArrayAccess* arrayAccess = static_cast<HLSLArrayAccess*>(expression);
        OutputExpression(arrayAccess->array);
        m_writer.Write("[");
        OutputExpression(arrayAccess->index);
        m_writer.Write("]");
    }
    else if (expression->nodeType == HLSLNodeType_FunctionCall)
    {
        HLSLFunctionCall* functionCall = static_cast<HLSLFunctionCall*>(expression);
        const char* name = functionCall->function->name;

        if (String_Equal(name, "tex2D") || String_Equal(name, "texCUBE"))
        {
            OutputExpressionList(functionCall->argument);
            m_writer.Write(")");
        }
        else
        {
            m_writer.Write("%s(", name);
            OutputExpressionList(functionCall->argument);
            m_writer.Write(")");
        }
    }
    else
    {
        m_writer.Write("<unknown expression>");
    }
}

void MSLGenerator::OutputArguments(HLSLArgument* argument)
{
    int numArgs = 0;
    while (argument != NULL)
    {
        if (numArgs > 0)
        {
            m_writer.Write(", ");
        }

        switch (argument->modifier)
        {
        case HLSLArgumentModifier_In:
            m_writer.Write("in ");
            break;
        case HLSLArgumentModifier_Out:
            m_writer.Write("out ");
            break;
        case HLSLArgumentModifier_Inout:
            m_writer.Write("inout ");
            break;
        case HLSLArgumentModifier_Uniform:
            m_writer.Write("uniform ");
            break;
        }

        const char* semantic = argument->sv_semantic ? argument->sv_semantic : argument->semantic;

        OutputDeclaration(argument->type, argument->name, semantic, /*registerName=*/NULL, argument->defaultValue);
        argument = argument->nextArgument;
        ++numArgs;
    }
}

const char* MSLGenerator::GetAttributeName(HLSLAttributeType attributeType)
{
    if (attributeType == HLSLAttributeType_Unroll)
        return "unroll";
    if (attributeType == HLSLAttributeType_Branch)
        return "branch";
    if (attributeType == HLSLAttributeType_Flatten)
        return "flatten";
    return NULL;
}

void MSLGenerator::OutputAttributes(int indent, HLSLAttribute* attribute)
{
    while (attribute != NULL)
    {
        const char* attributeName = GetAttributeName(attribute->attributeType);

        if (attributeName != NULL)
        {
            m_writer.WriteLine(indent, attribute->fileName, attribute->line, "[%s]", attributeName);
        }

        attribute = attribute->nextAttribute;
    }
}

void MSLGenerator::OutputStatements(int indent, HLSLStatement* statement)
{
    while (statement != NULL)
    {
        if (statement->hidden)
        {
            statement = statement->nextStatement;
            continue;
        }

        OutputAttributes(indent, statement->attributes);

        if (statement->nodeType == HLSLNodeType_Declaration)
        {
            HLSLDeclaration* declaration = static_cast<HLSLDeclaration*>(statement);
            if (!(declaration->type.flags & HLSLTypeFlag_Property))
            {
                m_writer.BeginLine(indent, declaration->fileName, declaration->line);
                OutputDeclaration(declaration);
                m_writer.EndLine(";");
            }
        }
        else if (statement->nodeType == HLSLNodeType_Struct)
        {
            HLSLStruct* structure = static_cast<HLSLStruct*>(statement);
            m_writer.WriteLine(indent, structure->fileName, structure->line, "struct %s", structure->name);
            m_writer.WriteLine(indent, structure->fileName, structure->line, "{");
            HLSLStructField* field = structure->field;
            while (field != NULL)
            {
                if (!field->hidden)
                {
                    m_writer.BeginLine(indent + 1, field->fileName, field->line);
                    const char* semantic = field->sv_semantic ? field->sv_semantic : field->semantic;
                    char attr_name[64];

                    if (structure->usage == struct_VertexIn || structure->usage == struct_VertexOut || structure->usage == struct_FragmentIn || structure->usage == struct_FragmentOut)
                    {
                        struct
                        {
                            const char* sv_semantic;
                            unsigned vattr;
                            const char* mtl_semantic;
                            const char* mtl_fp_semantic;
                        } attr[] =
                        {
                          { "POSITION", rhi::VATTR_POSITION, "position", "" },
                          { "SV_POSITION", rhi::VATTR_POSITION, "position", "" },
                          { "NORMAL", rhi::VATTR_NORMAL, "normal", "" },
                          { "NORMAL", rhi::VATTR_NORMAL, "normal", "" },
                          { "TEXCOORD", rhi::VATTR_TEXCOORD_0, "texcoord0", "" },
                          { "TEXCOORD0", rhi::VATTR_TEXCOORD_0, "texcoord0", "" },
                          { "TEXCOORD1", rhi::VATTR_TEXCOORD_1, "texcoord1", "" },
                          { "TEXCOORD2", rhi::VATTR_TEXCOORD_2, "texcoord2", "" },
                          { "TEXCOORD3", rhi::VATTR_TEXCOORD_3, "texcoord3", "" },
                          { "TEXCOORD4", rhi::VATTR_TEXCOORD_4, "texcoord4", "" },
                          { "TEXCOORD5", rhi::VATTR_TEXCOORD_5, "texcoord5", "" },
                          { "TEXCOORD6", rhi::VATTR_TEXCOORD_6, "texcoord6", "" },
                          { "TEXCOORD7", rhi::VATTR_TEXCOORD_7, "texcoord7", "" },
                          { "COLOR", rhi::VATTR_COLOR_0, "color0", "color(0)" },
                          { "COLOR0", rhi::VATTR_COLOR_0, "color0", "color(0)" },
                          { "COLOR1", rhi::VATTR_COLOR_1, "color1", "color(1)" },
                          { "SV_TARGET", rhi::VATTR_COLOR_0, "color0", "color(0)" },
                          { "SV_TARGET0", rhi::VATTR_COLOR_0, "color0", "color(0)" },
                          { "SV_TARGET1", rhi::VATTR_COLOR_1, "color1", "color(1)" },
                          { "TANGENT", rhi::VATTR_TANGENT, "", "" },
                          { "BINORMAL", rhi::VATTR_BINORMAL, "", "" },
                          { "BLENDWEIGHT", rhi::VATTR_BLENDWEIGHT, "", "" },
                          { "BLENDINDICES", rhi::VATTR_BLENDINDEX, "", "" }
                        };

                        for (unsigned i = 0; i != countof(attr); ++i)
                        {
                            if (stricmp(attr[i].sv_semantic, semantic) == 0)
                            {
                                if (structure->usage == struct_VertexIn)
                                    Snprintf(attr_name, countof(attr_name), "[[ attribute(%u) ]]", attr[i].vattr);
                                else if (structure->usage == struct_VertexOut && stricmp(semantic, "SV_POSITION") == 0)
                                    Snprintf(attr_name, countof(attr_name), "[[ position ]]");
                                else if (structure->usage == struct_VertexOut || structure->usage == struct_FragmentIn)
                                    Snprintf(attr_name, countof(attr_name), "[[ user(%s) ]]", attr[i].mtl_semantic);
                                else if (structure->usage == struct_FragmentOut)
                                    Snprintf(attr_name, countof(attr_name), "[[ %s ]]", attr[i].mtl_fp_semantic);

                                semantic = attr_name;
                                break;
                            }
                        }
                    }

                    OutputDeclaration(field->type, field->name, semantic);
                    m_writer.Write(";");
                    m_writer.EndLine();
                }
                field = field->nextField;
            }
            m_writer.WriteLine(indent, "};");
        }
        else if (statement->nodeType == HLSLNodeType_Buffer)
        {
            HLSLBuffer* buffer = static_cast<HLSLBuffer*>(statement);
            HLSLLiteralExpression* arr_sz = static_cast<HLSLLiteralExpression*>(buffer->field->type.arraySize);

            m_writer.WriteLine
            (
            indent, buffer->fileName, buffer->line,
            "struct __%s { packed_float4 data[%i]; };",
            buffer->name,
            arr_sz->iValue
            );
        }
        else if (statement->nodeType == HLSLNodeType_Function)
        {
            HLSLFunction* function = static_cast<HLSLFunction*>(statement);

            if (strcmp(function->name, m_entryName) == 0)
            {
                const char* returnTypeName = GetTypeName(function->returnType);
                char btype = (m_target == Target_VertexShader) ? 'V' : 'F';

                m_writer.BeginLine(indent, function->fileName, function->line);
                m_writer.WriteLine(indent, "%s %s %s", (m_target == Target_VertexShader) ? "vertex" : "fragment", returnTypeName, function->name);
                m_writer.WriteLine(indent, "(");

                //                m_writer.WriteLine(indent + 1, "%s %s [[ stage_in ]]", GetTypeName(function->argument->type), function->argument->name);
                m_writer.WriteLine(indent + 1, "%s in [[ stage_in ]]", GetTypeName(function->argument->type));
                for (unsigned i = 0; i != 32; ++i)
                {
                    char bname[64];

                    Snprintf(bname, countof(bname), "%cP_Buffer%u_t", btype, i);
                    if (m_tree->FindBuffer(bname))
                    {
                        m_writer.WriteLine(indent + 1, ", constant __%cP_Buffer%u_t* buf%u [[ buffer(%u) ]]", btype, i, i, (m_target == Target_VertexShader) ? rhi::MAX_VERTEX_STREAM_COUNT + i : i);
                    }
                }
                for (unsigned t = 0; t != _tex.size(); ++t)
                {
                    const char* ttype = "";

                    if (_tex[t].type == HLSLBaseType_Sampler2D)
                    {
                        ttype = "texture2d<float>";
                    }

                    m_writer.WriteLine(indent + 1, ", %s %s_texture [[ texture(%u) ]]", ttype, _tex[t].name.c_str(), _tex[t].unit);
                }
                for (unsigned t = 0; t != _tex.size(); ++t)
                {
                    m_writer.WriteLine(indent + 1, ", sampler %s_sampler [[ sampler(%u) ]]", _tex[t].name.c_str(), _tex[t].unit);
                }
                m_writer.WriteLine(indent, ")");
                m_writer.WriteLine(indent, "{");
                for (unsigned i = 0; i != 32; ++i)
                {
                    char bname[64];

                    Snprintf(bname, countof(bname), "%cP_Buffer%u_t", btype, i);
                    if (m_tree->FindBuffer(bname))
                    {
                        m_writer.WriteLine(indent + 1, "constant packed_float4* %cP_Buffer%u = buf%u->data;", btype, i, i);
                    }
                }
                m_writer.WriteLine(indent + 1, "%s %s = in;", GetTypeName(function->argument->type), function->argument->name);

                for (HLSLStatement* s = m_tree->GetRoot()->statement; s; s = s->nextStatement)
                {
                    if (s->nodeType == HLSLNodeType_Declaration)
                    {
                        HLSLDeclaration* decl = (HLSLDeclaration*)s;

                        if (decl->type.flags & HLSLTypeFlag_Property)
                        {
                            m_writer.BeginLine(indent + 1);
                            OutputDeclarationType(decl->type);
                            OutputDeclarationBody(decl->type, decl->name, decl->semantic, decl->registerName, decl->assignment);
                            m_writer.Write(";");
                            m_writer.EndLine();
                        }
                    }
                }

                OutputStatements(indent + 1, function->statement);
                m_writer.WriteLine(indent, "};");
            }
            else
            {
                // Use an alternate name for the function which is supposed to be entry point
                // so that we can supply our own function which will be the actual entry point.
                const char* functionName = function->name;
                const char* returnTypeName = GetTypeName(function->returnType);

                m_writer.BeginLine(indent, function->fileName, function->line);
                m_writer.Write("%s %s(", returnTypeName, functionName);

                OutputArguments(function->argument);

                const char* semantic = function->sv_semantic ? function->sv_semantic : function->semantic;
                if (semantic != NULL)
                {
                    m_writer.Write(") : %s {", semantic);
                }
                else
                {
                    m_writer.Write(") {");
                }

                m_writer.EndLine();

                OutputStatements(indent + 1, function->statement);
                m_writer.WriteLine(indent, "};");
            }
        }
        else if (statement->nodeType == HLSLNodeType_ExpressionStatement)
        {
            HLSLExpressionStatement* expressionStatement = static_cast<HLSLExpressionStatement*>(statement);
            m_writer.BeginLine(indent, statement->fileName, statement->line);
            OutputExpression(expressionStatement->expression);
            m_writer.EndLine(";");
        }
        else if (statement->nodeType == HLSLNodeType_ReturnStatement)
        {
            HLSLReturnStatement* returnStatement = static_cast<HLSLReturnStatement*>(statement);
            if (returnStatement->expression != NULL)
            {
                m_writer.BeginLine(indent, returnStatement->fileName, returnStatement->line);
                m_writer.Write("return ");
                OutputExpression(returnStatement->expression);
                m_writer.EndLine(";");
            }
            else
            {
                m_writer.WriteLine(indent, returnStatement->fileName, returnStatement->line, "return;");
            }
        }
        else if (statement->nodeType == HLSLNodeType_DiscardStatement)
        {
            HLSLDiscardStatement* discardStatement = static_cast<HLSLDiscardStatement*>(statement);
            m_writer.WriteLine(indent, discardStatement->fileName, discardStatement->line, "discard;");
        }
        else if (statement->nodeType == HLSLNodeType_BreakStatement)
        {
            HLSLBreakStatement* breakStatement = static_cast<HLSLBreakStatement*>(statement);
            m_writer.WriteLine(indent, breakStatement->fileName, breakStatement->line, "break;");
        }
        else if (statement->nodeType == HLSLNodeType_ContinueStatement)
        {
            HLSLContinueStatement* continueStatement = static_cast<HLSLContinueStatement*>(statement);
            m_writer.WriteLine(indent, continueStatement->fileName, continueStatement->line, "continue;");
        }
        else if (statement->nodeType == HLSLNodeType_IfStatement)
        {
            HLSLIfStatement* ifStatement = static_cast<HLSLIfStatement*>(statement);
            m_writer.BeginLine(indent, ifStatement->fileName, ifStatement->line);
            m_writer.Write("if (");
            OutputExpression(ifStatement->condition);
            m_writer.Write(") {");
            m_writer.EndLine();
            OutputStatements(indent + 1, ifStatement->statement);
            m_writer.WriteLine(indent, "}");
            if (ifStatement->elseStatement != NULL)
            {
                m_writer.WriteLine(indent, "else {");
                OutputStatements(indent + 1, ifStatement->elseStatement);
                m_writer.WriteLine(indent, "}");
            }
        }
        else if (statement->nodeType == HLSLNodeType_ForStatement)
        {
            HLSLForStatement* forStatement = static_cast<HLSLForStatement*>(statement);
            m_writer.BeginLine(indent, forStatement->fileName, forStatement->line);
            m_writer.Write("for (");
            OutputDeclaration(forStatement->initialization);
            m_writer.Write("; ");
            OutputExpression(forStatement->condition);
            m_writer.Write("; ");
            OutputExpression(forStatement->increment);
            m_writer.Write(") {");
            m_writer.EndLine();
            OutputStatements(indent + 1, forStatement->statement);
            m_writer.WriteLine(indent, "}");
        }
        else if (statement->nodeType == HLSLNodeType_BlockStatement)
        {
            HLSLBlockStatement* blockStatement = static_cast<HLSLBlockStatement*>(statement);
            m_writer.WriteLine(indent, blockStatement->fileName, blockStatement->line, "{");
            OutputStatements(indent + 1, blockStatement->statement);
            m_writer.WriteLine(indent, "}");
        }
        else if (statement->nodeType == HLSLNodeType_Technique)
        {
            // Techniques are ignored.
        }
        else if (statement->nodeType == HLSLNodeType_Pipeline)
        {
            // Pipelines are ignored.
        }
        else
        {
            // Unhanded statement type.
            DVASSERT(0);
        }

        statement = statement->nextStatement;
    }
}

void MSLGenerator::OutputDeclaration(HLSLDeclaration* declaration)
{
    bool isSamplerType = IsSamplerType(declaration->type);

    if (isSamplerType)
    {
        int reg = -1;
        if (declaration->registerName != NULL)
        {
            sscanf(declaration->registerName, "s%d", &reg);
        }

        const char* textureType = NULL;
        const char* samplerType = "SamplerState";
        // @@ Handle generic sampler type.

        if (declaration->type.baseType == HLSLBaseType_Sampler2D)
        {
            textureType = "texture2d<float>";
            samplerType = "sampler";
        }
        else if (declaration->type.baseType == HLSLBaseType_Sampler3D)
        {
            textureType = "Texture3D";
        }
        else if (declaration->type.baseType == HLSLBaseType_SamplerCube)
        {
            textureType = "TextureCube";
        }
        else if (declaration->type.baseType == HLSLBaseType_Sampler2DShadow)
        {
            textureType = "Texture2D";
            samplerType = "SamplerComparisonState";
        }
        else if (declaration->type.baseType == HLSLBaseType_Sampler2DMS)
        {
            textureType = "Texture2DMS<float4>"; // @@ Is template argument required?
            samplerType = NULL;
        }

        if (samplerType != NULL)
        {
            tex_t tex;

            tex.name = declaration->name;
            tex.type = declaration->type.baseType;
            tex.unit = _tex.size();
            _tex.push_back(tex);
            /*            
            if (reg != -1)
            {
                m_writer.Write("%s %s_texture [[ texture(%d) ]]; %s %s_sampler [[ sampler(%d) ]];", textureType, declaration->name, reg, samplerType, declaration->name, reg);
            }
            else
            {
                static unsigned t = 0;
                //                m_writer.Write("%s %s_texture; %s %s_sampler", textureType, declaration->name, samplerType, declaration->name);
                m_writer.Write
                (
                "%s %s_texture [[ texture(%u) ]]; %s %s_sampler [[ sampler(%u) ]]",
                textureType, declaration->name, t,
                samplerType, declaration->name, t
                );
                ++t;
            }
*/
        }
        else
        {
            if (reg != -1)
            {
                m_writer.Write("%s %s : register(t%d)", textureType, declaration->name, reg);
            }
            else
            {
                m_writer.Write("%s %s", textureType, declaration->name);
            }
        }
        return;
    }

    OutputDeclarationType(declaration->type);
    OutputDeclarationBody(declaration->type, declaration->name, declaration->semantic, declaration->registerName, declaration->assignment);
    declaration = declaration->nextDeclaration;

    while (declaration != NULL)
    {
        m_writer.Write(", ");
        OutputDeclarationBody(declaration->type, declaration->name, declaration->semantic, declaration->registerName, declaration->assignment);
        declaration = declaration->nextDeclaration;
    };
}

void MSLGenerator::OutputDeclarationType(const HLSLType& type)
{
    const char* typeName = GetTypeName(type);

    if (type.flags & HLSLTypeFlag_Const)
    {
        m_writer.Write("const ");
    }
    if (type.flags & HLSLTypeFlag_Static && !(type.flags & HLSLTypeFlag_Property))
    {
        m_writer.Write("static ");
    }

    // Interpolation modifiers.
    if (type.flags & HLSLTypeFlag_Centroid)
    {
        m_writer.Write("centroid ");
    }
    if (type.flags & HLSLTypeFlag_Linear)
    {
        m_writer.Write("linear ");
    }
    if (type.flags & HLSLTypeFlag_NoInterpolation)
    {
        m_writer.Write("nointerpolation ");
    }
    if (type.flags & HLSLTypeFlag_NoPerspective)
    {
        m_writer.Write("noperspective ");
    }
    if (type.flags & HLSLTypeFlag_Sample) // @@ Only in shader model >= 4.1
    {
        m_writer.Write("sample ");
    }

    m_writer.Write("%s ", typeName);
}

void MSLGenerator::OutputDeclarationBody(const HLSLType& type, const char* name, const char* semantic /*=NULL*/, const char* registerName /*=NULL*/, HLSLExpression* assignment /*=NULL*/)
{
    m_writer.Write("%s", name);

    if (type.array)
    {
        DVASSERT(semantic == NULL);
        m_writer.Write("[");
        if (type.arraySize != NULL)
        {
            OutputExpression(type.arraySize);
        }
        m_writer.Write("]");
    }

    if (semantic != NULL)
    {
        m_writer.Write(" %s", semantic);
    }

    if (registerName != NULL)
    {
        if (m_isInsideBuffer)
        {
            m_writer.Write(" : packoffset(%s)", registerName);
        }
        else
        {
            m_writer.Write(" : register(%s)", registerName);
        }
    }

    if (assignment != NULL && !IsSamplerType(type))
    {
        m_writer.Write(" = ");
        if (type.array)
        {
            m_writer.Write("{ ");
            OutputExpressionList(assignment);
            m_writer.Write(" }");
        }
        else
        {
            OutputExpression(assignment);
        }
    }
}

void MSLGenerator::OutputDeclaration(const HLSLType& type, const char* name, const char* semantic /*=NULL*/, const char* registerName /*=NULL*/, HLSLExpression* assignment /*=NULL*/)
{
    OutputDeclarationType(type);
    OutputDeclarationBody(type, name, semantic, registerName, assignment);
}

bool MSLGenerator::ChooseUniqueName(const char* base, char* dst, int dstLength) const
{
    // IC: Try without suffix first.
    String_Printf(dst, dstLength, "%s", base);
    if (!m_tree->GetContainsString(base))
    {
        return true;
    }

    for (int i = 1; i < 1024; ++i)
    {
        String_Printf(dst, dstLength, "%s%d", base, i);
        if (!m_tree->GetContainsString(dst))
        {
            return true;
        }
    }
    return false;
}

} // namespace sl