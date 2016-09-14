#include "sl_Common.h"

#include "sl_GeneratorHLSL.h"
#include "sl_Parser.h"
#include "sl_Tree.h"

namespace sl
{
const char*
HLSLGenerator::GetTypeName(const HLSLType& type)
{
    switch (type.baseType)
    {
    case HLSLBaseType_Void:
        return "void";
    case HLSLBaseType_Float:
        return "float";
    case HLSLBaseType_Float2:
        return "float2";
    case HLSLBaseType_Float3:
        return "float3";
    case HLSLBaseType_Float4:
        return "float4";
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

int HLSLGenerator::GetFunctionArguments(HLSLFunctionCall* functionCall, HLSLExpression* expression[], int maxArguments)
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

HLSLGenerator::HLSLGenerator(Allocator* allocator)
    : m_writer()
{
    m_tree = NULL;
    m_entryName = NULL;
    m_target = Target_VertexShader;
    m_isInsideBuffer = false;
    /*
    m_textureSampler2DStruct[0] = 0;
    m_textureSampler2DCtor[0] = 0;
    m_textureSampler3DStruct[0] = 0;
    m_textureSampler3DCtor[0] = 0;
    m_textureSamplerCubeStruct[0] = 0;
    m_textureSamplerCubeCtor[0] = 0;
    m_tex2DFunction[0] = 0;
    m_tex2DProjFunction[0] = 0;
    m_tex2DLodFunction[0] = 0;
    m_tex2DBiasFunction[0] = 0;
    m_tex2DGradFunction[0] = 0;
    m_tex2DGatherFunction[0] = 0;
    m_tex2DSizeFunction[0] = 0;
    m_tex2DCmpFunction[0] = 0;
    m_tex2DMSFetchFunction[0] = 0;
    m_tex3DFunction[0] = 0;
    m_tex3DLodFunction[0] = 0;
    m_tex3DBiasFunction[0] = 0;
    m_texCubeFunction[0] = 0;
    m_texCubeLodFunction[0] = 0;
    m_texCubeBiasFunction[0] = 0;
*/
}

// @@ We need a better way of doing semantic replacement:
// - Look at the function being generated.
// - Return semantic, semantics associated to fields of the return structure, or output arguments, or fields of structures associated to output arguments -> output semantic replacement.
// - Semantics associated input arguments or fields of the input arguments -> input semantic replacement.
const char*
HLSLGenerator::TranslateSemantic(const char* semantic, bool output, HLSLGenerator::Target target) const
{
    const char* sem = nullptr;

    if (target == HLSLGenerator::Target_VertexShader)
    {
        if (!stricmp(semantic, "SV_Position"))
            sem = (output && m_mode == MODE_DX11) ? "SV_POSITION" : "POSITION";
        else if (!stricmp(semantic, "SV_Normal"))
            sem = "NORMAL";
    }
    else if (target == HLSLGenerator::Target_PixelShader)
    {
        if (!stricmp(semantic, "SV_Target"))
            sem = (m_mode == MODE_DX11) ? "SV_TARGET0" : "COLOR0";
        else if (!stricmp(semantic, "SV_Target0"))
            sem = (m_mode == MODE_DX11) ? "SV_TARGET0" : "COLOR0";
        else if (!stricmp(semantic, "SV_Target1"))
            sem = (m_mode == MODE_DX11) ? "SV_TARGET1" : "COLOR1";
        else if (!stricmp(semantic, "SV_Target2"))
            sem = (m_mode == MODE_DX11) ? "SV_TARGET2" : "COLOR2";
        else if (!stricmp(semantic, "SV_Target3"))
            sem = (m_mode == MODE_DX11) ? "SV_TARGET3" : "COLOR3";
    }

    return sem;
    /*
    if (target == HLSLGenerator::Target_VertexShader)
    {
        if (output)
        {
            if (String_Equal("POSITION", semantic))
                return "SV_Position";
        }
    }
    else if (target == HLSLGenerator::Target_PixelShader)
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
*/
}

bool HLSLGenerator::Generate(HLSLTree* tree, Mode mode, Target target, const char* entryName, std::string* code)
{
    m_tree = tree;
    m_mode = mode;
    m_entryName = entryName;
    m_target = target;
    m_isInsideBuffer = false;

    m_writer.Reset(code);
    //    m_writer.EnableLineNumbers(true);
    if (m_mode == MODE_DX11)
        m_writer.WriteLine(0, "#define FP_A8(t) (t).r");
    else
        m_writer.WriteLine(0, "#define FP_A8(t) (t).a");

    // @@ Should we generate an entirely new copy of the tree so that we can modify it in place?

    HLSLFunction* function = tree->FindFunction(entryName);

    // Handle return value semantics
    if (function->semantic != NULL)
    {
        function->sv_semantic = TranslateSemantic(function->semantic, /*output=*/true, target);
    }
    if (function->returnType.baseType == HLSLBaseType_UserDefined)
    {
        HLSLStruct* s = tree->FindGlobalStruct(function->returnType.typeName);

        HLSLStructField* sv_fields = NULL;

        HLSLStructField* lastField = NULL;
        HLSLStructField* field = s->field;
        while (field)
        {
            HLSLStructField* nextField = field->nextField;

            if (field->semantic)
            {
                field->hidden = false;
                field->sv_semantic = TranslateSemantic(field->semantic, /*output=*/true, target);

                // Fields with SV semantics are stored at the end to avoid linkage problems.

                if (field->sv_semantic != NULL)
                {
                    // Unlink from last.
                    if (lastField != NULL)
                        lastField->nextField = nextField;
                    else
                        s->field = nextField;

                    // Add to sv_fields.
                    field->nextField = sv_fields;
                    sv_fields = field;
                }
            }

            if (field != sv_fields)
                lastField = field;
            field = nextField;
        }

        // Append SV fields at the end.
        if (sv_fields != NULL)
        {
            if (lastField == NULL)
            {
                s->field = sv_fields;
            }
            else
            {
                DVASSERT(lastField->nextField == NULL);
                lastField->nextField = sv_fields;
            }
        }
    }

    // Handle argument semantics.
    HLSLArgument* argument = function->argument;
    while (argument)
    {
        bool output = argument->modifier == HLSLArgumentModifier_Out;
        if (argument->semantic)
        {
            argument->sv_semantic = TranslateSemantic(argument->semantic, output, target);
        }

        if (argument->type.baseType == HLSLBaseType_UserDefined)
        {
            HLSLStruct* s = tree->FindGlobalStruct(argument->type.typeName);

            HLSLStructField* field = s->field;
            while (field)
            {
                if (field->semantic)
                {
                    field->hidden = false;
                    if (target == Target_PixelShader && !output && String_EqualNoCase(field->semantic, "POSITION"))
                    {
                        DVASSERT(String_EqualNoCase(field->sv_semantic, "SV_Position"));
                        field->hidden = true;
                    }

                    field->sv_semantic = TranslateSemantic(field->semantic, output, target);
                }

                field = field->nextField;
            }
        }

        argument = argument->nextArgument;
    }

    HLSLRoot* root = m_tree->GetRoot();
    OutputStatements(0, root->statement);

    m_tree = NULL;
    return true;
}

const char* HLSLGenerator::GetResult() const
{
    return m_writer.GetResult();
}

void HLSLGenerator::OutputExpressionList(HLSLExpression* expression)
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

void HLSLGenerator::OutputExpression(HLSLExpression* expression)
{
    if (expression->nodeType == HLSLNodeType_IdentifierExpression)
    {
        HLSLIdentifierExpression* identifierExpression = static_cast<HLSLIdentifierExpression*>(expression);
        const char* name = identifierExpression->name;

        if (IsSamplerType(identifierExpression->expressionType) && identifierExpression->global)
        {
            if (m_mode == MODE_DX11)
            {
                if (identifierExpression->expressionType.baseType == HLSLBaseType_Sampler2D || identifierExpression->expressionType.baseType == HLSLBaseType_SamplerCube)
                {
                    m_writer.Write("%s_texture.Sample( %s_sampler ", name, name);
                }
            }
            else if (m_mode == MODE_DX9)
            {
                if (identifierExpression->expressionType.baseType == HLSLBaseType_Sampler2D)
                {
                    m_writer.Write("tex2D( %s", name);
                }
                else if (identifierExpression->expressionType.baseType == HLSLBaseType_SamplerCube)
                {
                    m_writer.Write("texCUBE( %s", name);
                }
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
        m_writer.Write("(");
        OutputDeclaration(castingExpression->type, "");
        m_writer.Write(")(");
        OutputExpression(castingExpression->expression);
        m_writer.Write(")");
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
        m_writer.Write("(");
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
        m_writer.Write(")");
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
        m_writer.Write("(");
        OutputExpression(memberAccess->object);
        m_writer.Write(").%s", memberAccess->field);
        //        m_writer.Write(".%s", memberAccess->field);
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
        bool sampler_call = false;
        bool sampler_lod = String_Equal(name, "tex2Dlod");

        if (String_Equal(name, "tex2D") || String_Equal(name, "tex2Dlod") || String_Equal(name, "texCUBE"))
        {
            sampler_call = true;
        }

        if (sampler_call)
        {
            if (sampler_lod)
            {
                DVASSERT(functionCall->argument->nodeType == HLSLNodeType_IdentifierExpression);
                HLSLIdentifierExpression* identifier = static_cast<HLSLIdentifierExpression*>(functionCall->argument);
                DVASSERT(IsSamplerType(identifier->expressionType) && identifier->global);

                if (m_mode == MODE_DX11)
                {
                    if (identifier->expressionType.baseType == HLSLBaseType_Sampler2D)
                    {
                        m_writer.Write("%s_texture.SampleLevel( %s_sampler ", identifier->name, identifier->name);

                        for (HLSLExpression* expr = identifier->nextExpression; expr; expr = expr->nextExpression)
                        {
                            m_writer.Write(", ");
                            OutputExpression(expr);
                        }
                        m_writer.Write(")");
                    }
                }
                else if (m_mode == MODE_DX9)
                {
                    DVASSERT(!"notimpl");
                    if (identifier->expressionType.baseType == HLSLBaseType_Sampler2D)
                    {
                        m_writer.Write("tex2D( %s", name);
                    }
                    else if (identifier->expressionType.baseType == HLSLBaseType_SamplerCube)
                    {
                        m_writer.Write("texCUBE( %s", name);
                    }
                }
            }
            else
            {
            OutputExpressionList(functionCall->argument);
            m_writer.Write(")");
            }
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

void HLSLGenerator::OutputArguments(HLSLArgument* argument)
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

static const char* GetAttributeName(HLSLAttributeType attributeType)
{
    if (attributeType == HLSLAttributeType_Unroll)
        return "unroll";
    if (attributeType == HLSLAttributeType_Branch)
        return "branch";
    if (attributeType == HLSLAttributeType_Flatten)
        return "flatten";
    return NULL;
}

void HLSLGenerator::OutputAttributes(int indent, HLSLAttribute* attribute)
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

void HLSLGenerator::OutputStatements(int indent, HLSLStatement* statement)
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
            m_writer.BeginLine(indent, declaration->fileName, declaration->line);
            OutputDeclaration(declaration);
            m_writer.EndLine(";");
        }
        else if (statement->nodeType == HLSLNodeType_Struct)
        {
            HLSLStruct* structure = static_cast<HLSLStruct*>(statement);
            m_writer.WriteLine(indent, structure->fileName, structure->line, "struct %s {", structure->name);
            HLSLStructField* field = structure->field;
            while (field != NULL)
            {
                if (!field->hidden)
                {
                    m_writer.BeginLine(indent + 1, field->fileName, field->line);
                    const char* semantic = field->sv_semantic ? field->sv_semantic : field->semantic;
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
            HLSLDeclaration* field = buffer->field;

            if (m_mode == MODE_DX11)
            {
                m_writer.BeginLine(indent, buffer->fileName, buffer->line);
                m_writer.Write("cbuffer %s", buffer->name);
                if (buffer->registerName != NULL)
                {
                    m_writer.Write(" : register(%s)", buffer->registerName);
                }
                m_writer.EndLine(" {");

                m_isInsideBuffer = true;

                while (field != NULL)
                {
                    if (!field->hidden)
                    {
                        m_writer.BeginLine(indent + 1, field->fileName, field->line);
                        OutputDeclaration(field->type, field->name, /*semantic=*/NULL, /*registerName*/ field->registerName, field->assignment);
                        m_writer.Write(";");
                        m_writer.EndLine();
                    }
                    field = (HLSLDeclaration*)field->nextStatement;
                }

                m_isInsideBuffer = false;

                m_writer.WriteLine(indent, "};");
            }
            else if (m_mode == MODE_DX9)
            {
                m_writer.WriteLine(indent, "uniform float4 %s[%u];", field->name, buffer->registerCount);
            }
        }
        else if (statement->nodeType == HLSLNodeType_Function)
        {
            HLSLFunction* function = static_cast<HLSLFunction*>(statement);

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

void HLSLGenerator::OutputDeclaration(HLSLDeclaration* declaration)
{
    bool isSamplerType = IsSamplerType(declaration->type);

    if (isSamplerType)
    {
        int reg = -1;
        if (declaration->registerName != NULL)
        {
            sscanf(declaration->registerName, "s%d", &reg);
        }

        if (m_mode == MODE_DX11)
        {
            const char* textureType = NULL;
            const char* samplerType = "SamplerState";
            // @@ Handle generic sampler type.

            if (declaration->type.baseType == HLSLBaseType_Sampler2D)
            {
                textureType = "Texture2D";
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
                if (reg != -1)
                {
                    m_writer.Write("%s %s_texture : register(t%d); %s %s_sampler : register(s%d)", textureType, declaration->name, reg, samplerType, declaration->name, reg);
                }
                else
                {
                    m_writer.Write("%s %s_texture; %s %s_sampler", textureType, declaration->name, samplerType, declaration->name);
                }
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
        }
        else if (m_mode == MODE_DX9)
        {
            const char* ttype = NULL;

            if (declaration->type.baseType == HLSLBaseType_Sampler2D)
            {
                ttype = "sampler2D";
            }
            else if (declaration->type.baseType == HLSLBaseType_SamplerCube)
            {
                ttype = "samplerCUBE";
            }

            DVASSERT(ttype);
            m_writer.Write("%s %s", ttype, declaration->name);
            if (reg != -1)
                m_writer.Write(" : TEXUNIT%i", reg);
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

void HLSLGenerator::OutputDeclarationType(const HLSLType& type)
{
    const char* typeName = GetTypeName(type);

    if (type.flags & HLSLTypeFlag_Const)
    {
        m_writer.Write("const ");
    }
    if (type.flags & HLSLTypeFlag_Static)
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

void HLSLGenerator::OutputDeclarationBody(const HLSLType& type, const char* name, const char* semantic /*=NULL*/, const char* registerName /*=NULL*/, HLSLExpression* assignment /*=NULL*/)
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
        m_writer.Write(" : %s", semantic);
    }

    if (registerName != NULL)
    {
        /*
        if (m_isInsideBuffer)
        {
            m_writer.Write(" : packoffset(%s)", registerName);
        }
        else
        {
            m_writer.Write(" : register(%s)", registerName);
        }
*/
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

void HLSLGenerator::OutputDeclaration(const HLSLType& type, const char* name, const char* semantic /*=NULL*/, const char* registerName /*=NULL*/, HLSLExpression* assignment /*=NULL*/)
{
    OutputDeclarationType(type);
    OutputDeclarationBody(type, name, semantic, registerName, assignment);
}

bool HLSLGenerator::ChooseUniqueName(const char* base, char* dst, int dstLength) const
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
