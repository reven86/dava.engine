#include "GameCore.h"
    
    #include "Render/RHI/rhi_Public.h"
    #include "Render/RHI/Common/rhi_Private.h"
    #include "Render/RHI/Common/dbg_StatSet.h"
    #include "Render/RHI/rhi_ShaderCache.h"
    #include "Render/RHI/rhi_ShaderSource.h"

    #include "Render/RHI/Common/PreProcess.h"

#include "Render/RenderBase.h"

    #include "Render/RHI/dbg_Draw.h"

    #include "FileSystem/DynamicMemoryFile.h"

    #include "M4/Engine.h"
    #include "M4/GLESGenerator.h"
    #include "M4/HLSLGenerator.h"
    #include "M4/MSLGenerator.h"
    #include "M4/HLSLParser.h"
    #include "M4/HLSLTree.h"

    #define PROFILER_ENABLED 1
    #include "Debug/Profiler.h"

using namespace DAVA;

GameCore::GameCore()
{
}

GameCore::~GameCore()
{
}

M4::Allocator M4_Allocator;

static const char*
readFile(const char* filename, size_t* lengthptr)
{
    char* buffer = nullptr;
    File* file = File::CreateFromSystemPath(filename, File::OPEN | File::READ);

    if (file)
    {
        size_t length = file->GetSize();

        buffer = M4_Allocator.New<char>(length);
        file->Read(buffer, length);
        file->Release();
        *lengthptr = length;
    }

    return buffer;
}

static const char*
_IndentString(int indent)
{
    static char text[256];

    memset(text, ' ', indent * 2);
    text[indent * 2] = '\0';
    return text;
}

static void
_DumpExpression(M4::HLSLExpression* expr, int indent, bool dump_subexpr = true)
{
    using namespace M4;

    switch (expr->nodeType)
    {
    case HLSLNodeType_ConstructorExpression:
    {
        HLSLConstructorExpression* ctor = (HLSLConstructorExpression*)expr;

        Log_Error("%sctor\n", _IndentString(indent));
        _DumpExpression(ctor->argument, indent + 1);
    }
    break;

    case HLSLNodeType_BinaryExpression:
    {
        HLSLBinaryExpression* bin = (HLSLBinaryExpression*)expr;
        const char* op = "";

        switch (bin->binaryOp)
        {
        case HLSLBinaryOp_And:
            op = "and";
            break;
        case HLSLBinaryOp_Or:
            op = "or";
            break;
        case HLSLBinaryOp_Add:
            op = "add";
            break;
        case HLSLBinaryOp_Sub:
            op = "sub";
            break;
        case HLSLBinaryOp_Mul:
            op = "mul";
            break;
        case HLSLBinaryOp_Div:
            op = "div";
            break;
        case HLSLBinaryOp_Less:
            op = "less";
            break;
        case HLSLBinaryOp_Greater:
            op = "greater";
            break;
        case HLSLBinaryOp_LessEqual:
            op = "lesseq";
            break;
        case HLSLBinaryOp_GreaterEqual:
            op = "gequal";
            break;
        case HLSLBinaryOp_Equal:
            op = "equal";
            break;
        case HLSLBinaryOp_NotEqual:
            op = "nequal";
            break;
        case HLSLBinaryOp_BitAnd:
            op = "bit-and";
            break;
        case HLSLBinaryOp_BitOr:
            op = "bit-or";
            break;
        case HLSLBinaryOp_BitXor:
            op = "bit-xor";
            break;
        case HLSLBinaryOp_Assign:
            op = "assign";
            break;
        case HLSLBinaryOp_AddAssign:
            op = "add-assign";
            break;
        case HLSLBinaryOp_SubAssign:
            op = "sub-assign";
            break;
        case HLSLBinaryOp_MulAssign:
            op = "mul-assign";
            break;
        case HLSLBinaryOp_DivAssign:
            op = "div-assign";
            break;
        }

        Log_Error("%sbin-expr %s\n", _IndentString(indent), op);
        _DumpExpression(bin->expression1, indent + 1);
        _DumpExpression(bin->expression2, indent + 1);
    }
    break;

    case HLSLNodeType_IdentifierExpression:
    {
        HLSLIdentifierExpression* var = (HLSLIdentifierExpression*)(expr);

        Log_Error("%svar \"%s\"\n", _IndentString(indent), var->name);
    }
    break;

    case HLSLNodeType_MemberAccess:
    {
        HLSLMemberAccess* member = (HLSLMemberAccess*)(expr);

        Log_Error("%svar.member \"%s\"\n", _IndentString(indent), member->field);
        _DumpExpression(member->object, indent + 1, false);
    }
    break;

    case HLSLNodeType_FunctionCall:
    {
        HLSLFunctionCall* call = (HLSLFunctionCall*)(expr);

        Log_Error("%scall \"%s\"\n", _IndentString(indent), call->function->name);
        int a = 0;
        for (HLSLExpression *e = call->argument; e; e = e->nextExpression, ++a)
        {
            Log_Error("%sarg%i\n", _IndentString(indent + 1), a);
            _DumpExpression(e, indent + 2, false);
        }
    }
    break;
    }

    if (dump_subexpr)
    {
        for (HLSLExpression* e = expr->nextExpression; e; e = e->nextExpression)
        {
            _DumpExpression(e, indent + 1);
        }
    }
}

static void
_DumpStatement(M4::HLSLStatement* s, int indent)
{
    using namespace M4;

    switch (s->nodeType)
    {
    case HLSLNodeType_Declaration:
    {
        HLSLDeclaration* decl = (HLSLDeclaration*)s;

        Log_Error("%sdecl  name= %s\n", _IndentString(indent), decl->name);
        for (HLSLExpression* e = decl->assignment; e; e = e->nextExpression)
        {
            _DumpExpression(e, indent + 1);
        }
    }
    break;

    case HLSLNodeType_Expression:
    {
        Log_Error("  expr\n");
    }
    break;

    case HLSLNodeType_ExpressionStatement:
    {
        HLSLExpressionStatement* st = (HLSLExpressionStatement*)s;

        Log_Error("%sexpr-statement\n", _IndentString(indent));
        _DumpExpression(st->expression, indent + 1);
    }
    break;

    case HLSLNodeType_ReturnStatement:
    {
        HLSLReturnStatement* ret = (HLSLReturnStatement*)s;

        Log_Error("%sret\n", _IndentString(indent));

        for (HLSLExpression* e = ret->expression; e; e = e->nextExpression)
        {
            _DumpExpression(e, indent + 1);
        }
    }
    break;

    case HLSLNodeType_BlockStatement:
    {
        HLSLBlockStatement* block = (HLSLBlockStatement*)s;

        Log_Error("%sblock\n", _IndentString(indent));
        for (HLSLStatement* b = block->statement; b; b = b->nextStatement)
        {
            _DumpStatement(b, indent + 1);
        }
        /*
            Log_Error( "%sblock\n", _IndentString(indent) );
            for( HLSLStatement* b=block->statement; b; b=b->nextStatement )
            {
                for( HLSLAttribute* a=block->attributes; a; a=a->nextAttribute )
                {
                    if( a->argument )
                        _DumpExpression( a->argument, indent+1, false );
                }
            }
*/
    }
    break;
    }
}

class
MyVisitor
: public M4::HLSLTreeVisitor
{
public:
    int indent;

    MyVisitor()
        : indent(0)
    {
    }
    //    virtual void VisitType(HLSLType & type);

    //-    virtual void VisitRoot(HLSLRoot * node);
    virtual void VisitTopLevelStatement(M4::HLSLStatement* node)
    {
        M4::HLSLStatement* s = (M4::HLSLStatement*)node;

        if (s->nodeType == M4::HLSLNodeType_Buffer)
        {
            M4::HLSLBuffer* buf = (M4::HLSLBuffer*)s;
            M4::HLSLDeclaration* decl = buf->field;

            M4::Log_Error("buf  \"%s\"\n", buf->name);
            M4::Log_Error("  decl  \"%s\"\n", decl->name);
        }

        HLSLTreeVisitor::VisitTopLevelStatement(node);
    }
    //    virtual void VisitStatements(HLSLStatement * statement);
    //    virtual void VisitStatement(HLSLStatement * node);
    virtual void VisitDeclaration(M4::HLSLDeclaration* node)
    {
        M4::HLSLDeclaration* decl = (M4::HLSLDeclaration*)node;

        M4::Log_Error("%sdecl  \"%s\"\n", _IndentString(indent), node->name);
        if (decl->assignment)
        {
            if (decl->assignment->nodeType == M4::HLSLNodeType_ArrayAccess)
            {
                M4::HLSLArrayAccess* aa = (M4::HLSLArrayAccess*)decl->assignment;

                if (aa->array->nodeType == M4::HLSLNodeType_IdentifierExpression)
                {
                    M4::HLSLIdentifierExpression* ie = (M4::HLSLIdentifierExpression*)(aa->array);

                    M4::Log_Error("    aa  \"%s\"\n", ie->name);
                }

                M4::Log_Error("    aa  \"%s\"\n", aa->fileName);
            }
            if (decl->assignment->nodeType == M4::HLSLNodeType_MemberAccess)
            {
                M4::HLSLMemberAccess* ma = (M4::HLSLMemberAccess*)decl->assignment;

                M4::Log_Error("    ma  \"%s\"\n", ma->field);
            }
            if (decl->assignment->nodeType == M4::HLSLNodeType_ConstructorExpression)
            {
                M4::HLSLConstructorExpression* ctor = (M4::HLSLConstructorExpression*)decl->assignment;

                M4::Log_Error("    ma  \"%s\"\n", ctor->fileName);
            }
        }
        HLSLTreeVisitor::VisitDeclaration(node);
    }
    virtual void VisitStruct(M4::HLSLStruct* node)
    {
        M4::Log_Error("%sstruct  \"%s\"\n", _IndentString(indent), node->name);
        HLSLTreeVisitor::VisitStruct(node);
    }
    //    virtual void VisitStructField(HLSLStructField * node);
    //    virtual void VisitBuffer(HLSLBuffer * node);
    virtual void VisitFunction(M4::HLSLFunction* func)
    {
        using namespace M4;

        Log_Error("%sfunction  \"%s\"\n", _IndentString(indent), func->name);

        for (HLSLStatement* s = func->statement; s; s = s->nextStatement)
        {
            _DumpStatement(s, indent + 1);
        }

        //        HLSLTreeVisitor::VisitFunction( func );
    }
    virtual void VisitArgument(M4::HLSLArgument* node)
    {
        //        M4::Log_Error( "  arg  name= %s\n", node->name );
        M4::HLSLTreeVisitor::VisitArgument(node);
    }
    virtual void VisitExpressionStatement(M4::HLSLExpressionStatement* node)
    {
        M4::Log_Error("%sexpr-statement\n", _IndentString(indent));
        M4::HLSLTreeVisitor::VisitExpressionStatement(node);
    }
    virtual void VisitExpression(M4::HLSLExpression* node)
    {
        //        M4::Log_Error( "  expr\n" );
        M4::HLSLTreeVisitor::VisitExpression(node);
    }
    virtual void VisitReturnStatement(M4::HLSLReturnStatement* node)
    {
        //        M4::Log_Error( "  ret\n" );
        M4::HLSLTreeVisitor::VisitReturnStatement(node);
    }
    //    virtual void VisitDiscardStatement(HLSLDiscardStatement * node);
    //    virtual void VisitBreakStatement(HLSLBreakStatement * node);
    //    virtual void VisitContinueStatement(HLSLContinueStatement * node);
    //    virtual void VisitIfStatement(HLSLIfStatement * node);
    //    virtual void VisitForStatement(HLSLForStatement * node);
    //    virtual void VisitBlockStatement(HLSLBlockStatement * node);
    //    virtual void VisitUnaryExpression(HLSLUnaryExpression * node);
    //    virtual void VisitBinaryExpression(HLSLBinaryExpression * node);
    //    virtual void VisitConditionalExpression(HLSLConditionalExpression * node);
    //    virtual void VisitCastingExpression(HLSLCastingExpression * node);
    //    virtual void VisitLiteralExpression(HLSLLiteralExpression * node);
    //    virtual void VisitIdentifierExpression(HLSLIdentifierExpression * node);
    //    virtual void VisitConstructorExpression(HLSLConstructorExpression * node);
    //    virtual void VisitMemberAccess(HLSLMemberAccess * node);
    //    virtual void VisitArrayAccess(HLSLArrayAccess * node);
    //    virtual void VisitFunctionCall(HLSLFunctionCall * node);
    //    virtual void VisitStateAssignment(HLSLStateAssignment * node);
    virtual void VisitSamplerState(M4::HLSLSamplerState* node)
    {
        M4::HLSLTreeVisitor::VisitSamplerState(node);
    }
    //    virtual void VisitPass(HLSLPass * node);
    //    virtual void VisitTechnique(HLSLTechnique * node);

    //    virtual void VisitFunctions(HLSLRoot * root);
    //    virtual void VisitParameters(HLSLRoot * root);
};

static void
_ProcessProperties(M4::HLSLTree* ast)
{
    struct
    buf_t
    {
        //        ShaderProp::Scope   scope;
        rhi::ShaderProp::Storage storage;
        FastName tag;
        uint32 regCount;
        std::vector<int> avlRegIndex;
    };

    struct
    prop_t
    {
        M4::HLSLDeclaration* decl;
        M4::HLSLStatement* prev_statement;
    };

    std::vector<rhi::ShaderProp> property;
    std::vector<prop_t> prop_decl;
    std::vector<buf_t> buf;
    std::vector<rhi::ShaderSampler> sampler;
    rhi::VertexLayout layout;

    // find properties/samplers
    {
        M4::Log_Error("properties :\n");
        M4::HLSLStatement* statement = ast->GetRoot()->statement;
        M4::HLSLStatement* pstatement = NULL;

        while (statement)
        {
            if (statement->nodeType == M4::HLSLNodeType_Declaration)
            {
                M4::HLSLDeclaration* decl = (M4::HLSLDeclaration*)statement;

                if (decl->type.flags & M4::HLSLTypeFlag_Property)
                {
                    property.resize(property.size() + 1);
                    rhi::ShaderProp& prop = property.back();

                    prop.uid = DAVA::FastName(decl->name);
                    prop.precision = rhi::ShaderProp::PRECISION_NORMAL;
                    prop.arraySize = 1;
                    prop.isBigArray = false;

                    switch (decl->type.baseType)
                    {
                    case M4::HLSLBaseType_Float:
                        prop.type = rhi::ShaderProp::TYPE_FLOAT1;
                        break;
                    case M4::HLSLBaseType_Float2:
                        prop.type = rhi::ShaderProp::TYPE_FLOAT2;
                        break;
                    case M4::HLSLBaseType_Float3:
                        prop.type = rhi::ShaderProp::TYPE_FLOAT3;
                        break;
                    case M4::HLSLBaseType_Float4:
                        prop.type = rhi::ShaderProp::TYPE_FLOAT4;
                        break;
                    case M4::HLSLBaseType_Float4x4:
                        prop.type = rhi::ShaderProp::TYPE_FLOAT4X4;
                        break;
                    }

                    for (M4::HLSLAttribute* a = decl->attributes; a; a = a->nextAttribute)
                    {
                        if (!stricmp(a->attrText, "static"))
                            prop.storage = rhi::ShaderProp::STORAGE_STATIC;
                        else if (!stricmp(a->attrText, "dynamic"))
                            prop.storage = rhi::ShaderProp::STORAGE_DYNAMIC;
                        else
                            prop.tag = FastName(a->attrText);
                    }

                    // TODO: handle assignment/default-value

                    buf_t* cbuf = nullptr;

                    for (std::vector<buf_t>::iterator b = buf.begin(), b_end = buf.end(); b != b_end; ++b)
                    {
                        if (b->storage == prop.storage && b->tag == prop.tag)
                        {
                            cbuf = &(buf[b - buf.begin()]);
                            break;
                        }
                    }

                    if (!cbuf)
                    {
                        buf.resize(buf.size() + 1);

                        cbuf = &(buf.back());

                        cbuf->storage = prop.storage;
                        cbuf->tag = prop.tag;
                        cbuf->regCount = 0;
                    }

                    prop.bufferindex = static_cast<uint32>(cbuf - &(buf[0]));

                    if (prop.type == rhi::ShaderProp::TYPE_FLOAT1 || prop.type == rhi::ShaderProp::TYPE_FLOAT2 || prop.type == rhi::ShaderProp::TYPE_FLOAT3)
                    {
                        bool do_add = true;
                        uint32 sz = 0;

                        switch (prop.type)
                        {
                        case rhi::ShaderProp::TYPE_FLOAT1:
                            sz = 1;
                            break;
                        case rhi::ShaderProp::TYPE_FLOAT2:
                            sz = 2;
                            break;
                        case rhi::ShaderProp::TYPE_FLOAT3:
                            sz = 3;
                            break;
                        default:
                            break;
                        }

                        for (unsigned r = 0; r != cbuf->avlRegIndex.size(); ++r)
                        {
                            if (cbuf->avlRegIndex[r] + sz <= 4)
                            {
                                prop.bufferReg = r;
                                prop.bufferRegCount = cbuf->avlRegIndex[r];

                                cbuf->avlRegIndex[r] += sz;

                                do_add = false;
                                break;
                            }
                        }

                        if (do_add)
                        {
                            prop.bufferReg = cbuf->regCount;
                            prop.bufferRegCount = 0;

                            ++cbuf->regCount;

                            cbuf->avlRegIndex.push_back(sz);
                        }
                    }
                    else if (prop.type == rhi::ShaderProp::TYPE_FLOAT4 || prop.type == rhi::ShaderProp::TYPE_FLOAT4X4)
                    {
                        prop.bufferReg = cbuf->regCount;
                        prop.bufferRegCount = prop.arraySize * ((prop.type == rhi::ShaderProp::TYPE_FLOAT4) ? 1 : 4);

                        cbuf->regCount += prop.bufferRegCount;

                        for (int i = 0; i != prop.bufferRegCount; ++i)
                            cbuf->avlRegIndex.push_back(4);
                    }

                    prop_t pp;

                    pp.decl = decl;
                    pp.prev_statement = pstatement;

                    prop_decl.push_back(pp);

                    M4::Log_Error("  property \"%s\"  {", decl->name);
                    for (M4::HLSLAttribute* a = decl->attributes; a; a = a->nextAttribute)
                    {
                        M4::Log_Error(" \"%s\"%s", a->attrText, (a->nextAttribute) ? "," : "");
                    }
                    M4::Log_Error(" }\n");
                }

                if (decl->type.baseType == M4::HLSLBaseType_Sampler2D
                    || decl->type.baseType == M4::HLSLBaseType_SamplerCube
                    )
                {
                    sampler.resize(sampler.size() + 1);
                    rhi::ShaderSampler& s = sampler.back();

                    switch (decl->type.baseType)
                    {
                    case M4::HLSLBaseType_Sampler2D:
                        s.type = rhi::TEXTURE_TYPE_2D;
                        break;
                    case M4::HLSLBaseType_SamplerCube:
                        s.type = rhi::TEXTURE_TYPE_CUBE;
                        break;
                    }
                    s.uid = FastName(decl->name);
                }
            }

            pstatement = statement;
            statement = statement->nextStatement;
        }
    }

    // get vertex-layout
    {
        M4::HLSLStruct* input = ast->FindGlobalStruct("VP_Input");

        if (input)
        {
            struct
            {
                rhi::VertexSemantics usage;
                const char* semantic;
            }
            semantic[] =
            {
              { rhi::VS_POSITION, "SV_POSITION" },
              { rhi::VS_NORMAL, "SV_NORMAL" },
              { rhi::VS_COLOR, "SV_COLOR" },
              { rhi::VS_TEXCOORD, "SV_TEXCOORD" },
              { rhi::VS_TANGENT, "SV_TANGENT" },
              { rhi::VS_BINORMAL, "SV_BINORMAL" },
              { rhi::VS_BLENDWEIGHT, "SV_BLENDWEIGHT" },
              { rhi::VS_BLENDINDEX, "SV_BLENDINDICES" }
            };

            layout.Clear();

            rhi::VertexDataFrequency cur_freq = rhi::VDF_PER_VERTEX;

            for (M4::HLSLStructField* field = input->field; field; field = field->nextField)
            {
                rhi::VertexSemantics usage;
                unsigned usage_i = 0;
                rhi::VertexDataType data_type = rhi::VDT_FLOAT;
                unsigned data_count = 0;
                rhi::VertexDataFrequency freq = rhi::VDF_PER_VERTEX;

                switch (field->type.baseType)
                {
                case M4::HLSLBaseType_Float4:
                    data_type = rhi::VDT_FLOAT;
                    data_count = 4;
                    break;
                case M4::HLSLBaseType_Float3:
                    data_type = rhi::VDT_FLOAT;
                    data_count = 3;
                    break;
                case M4::HLSLBaseType_Float2:
                    data_type = rhi::VDT_FLOAT;
                    data_count = 2;
                    break;
                case M4::HLSLBaseType_Float:
                    data_type = rhi::VDT_FLOAT;
                    data_count = 1;
                    break;
                case M4::HLSLBaseType_Uint4:
                    data_type = rhi::VDT_UINT8;
                    data_count = 4;
                    break;
                }

                for (unsigned i = 0; i != countof(semantic); ++i)
                {
                    const char* t = strstr(field->semantic, semantic[i].semantic);

                    if (t)
                    {
                        const char* tu = field->semantic + strlen(semantic[i].semantic);

                        usage = semantic[i].usage;
                        usage_i = atoi(tu);
                        break;
                    }
                }

                if (field->attribute)
                {
                    if (stricmp(field->attribute->attrText, "vertex") == 0)
                        freq = rhi::VDF_PER_VERTEX;
                    else if (stricmp(field->attribute->attrText, "instance") == 0)
                        freq = rhi::VDF_PER_INSTANCE;
                }

                if (freq != cur_freq)
                    layout.AddStream(freq);
                cur_freq = freq;

                layout.AddElement(usage, usage_i, data_type, data_count);
            }

            Logger::Info("vertex-layout:");
            layout.Dump();
        }
    }

#if 1
    if (prop_decl.size())
    {
        std::vector<M4::HLSLBuffer*> cbuf_decl;

        cbuf_decl.resize(buf.size());
        for (unsigned i = 0; i != buf.size(); ++i)
        {
            M4::HLSLBuffer* cbuf = ast->AddNode<M4::HLSLBuffer>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
            M4::HLSLDeclaration* decl = ast->AddNode<M4::HLSLDeclaration>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
            M4::HLSLLiteralExpression* sz = ast->AddNode<M4::HLSLLiteralExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
            ;
            char buf_name[128];
            char buf_type_name[128];
            char buf_reg_name[128];

            Snprintf(buf_name, sizeof(buf_name), "VP_Buffer%u", i);
            Snprintf(buf_type_name, sizeof(buf_name), "VP_Buffer%u_t", i);
            Snprintf(buf_reg_name, sizeof(buf_name), "b%u", i);

            decl->name = ast->AddString(buf_name);
            decl->type.baseType = M4::HLSLBaseType_Float4;
            decl->type.array = true;
            decl->type.arraySize = sz;

            sz->type = M4::HLSLBaseType_Int;
            sz->iValue = buf[i].regCount;

            cbuf->field = decl;
            cbuf->name = ast->AddString(buf_type_name);
            cbuf->registerName = ast->AddString(buf_reg_name);

            cbuf_decl[i] = cbuf;
        }

        for (unsigned i = 0; i != cbuf_decl.size() - 1; ++i)
            cbuf_decl[i]->nextStatement = cbuf_decl[i + 1];

        prop_decl[0].prev_statement->nextStatement = cbuf_decl[0];
        cbuf_decl[cbuf_decl.size() - 1]->nextStatement = prop_decl[0].decl;

        #define DO_FLOAT4_CAST 1

        DVASSERT(property.size() == prop_decl.size());
        for (unsigned i = 0; i != property.size(); ++i)
        {
            switch (property[i].type)
            {
            case rhi::ShaderProp::TYPE_FLOAT4:
            {
                M4::HLSLArrayAccess* arr_access = ast->AddNode<M4::HLSLArrayAccess>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                M4::HLSLLiteralExpression* idx = ast->AddNode<M4::HLSLLiteralExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                M4::HLSLIdentifierExpression* arr = ast->AddNode<M4::HLSLIdentifierExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                char buf_name[128];

                Snprintf(buf_name, sizeof(buf_name), "VP_Buffer%u", property[i].bufferindex);
                arr->name = ast->AddString(buf_name);
                arr->global = true;

                idx->type = M4::HLSLBaseType_Int;
                idx->iValue = property[i].bufferReg;

                arr_access->array = arr;
                arr_access->index = idx;
                    
                    #if DO_FLOAT4_CAST
                M4::HLSLCastingExpression* cast_expr = ast->AddNode<M4::HLSLCastingExpression>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                cast_expr->expression = arr_access;
                cast_expr->type.baseType = M4::HLSLBaseType_Float4;

                prop_decl[i].decl->assignment = cast_expr;
                prop_decl[i].decl->type.flags |= M4::HLSLTypeFlag_Static | M4::HLSLTypeFlag_Property;
                    #else
                prop_decl[i].decl->assignment = arr_access;
                prop_decl[i].decl->type.flags |= M4::HLSLTypeFlag_Static | M4::HLSLTypeFlag_Property;
                    #endif
            }
            break;

            case rhi::ShaderProp::TYPE_FLOAT3:
            case rhi::ShaderProp::TYPE_FLOAT2:
            case rhi::ShaderProp::TYPE_FLOAT1:
            {
                M4::HLSLMemberAccess* member_access = ast->AddNode<M4::HLSLMemberAccess>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                char xyzw[] = { 'x', 'y', 'z', 'w', '\0' };
                unsigned elem_cnt = 0;
                M4::HLSLArrayAccess* arr_access = ast->AddNode<M4::HLSLArrayAccess>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                M4::HLSLLiteralExpression* idx = ast->AddNode<M4::HLSLLiteralExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                M4::HLSLIdentifierExpression* arr = ast->AddNode<M4::HLSLIdentifierExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                char buf_name[128];

                switch (property[i].type)
                {
                case rhi::ShaderProp::TYPE_FLOAT1:
                    elem_cnt = 1;
                    break;
                case rhi::ShaderProp::TYPE_FLOAT2:
                    elem_cnt = 2;
                    break;
                case rhi::ShaderProp::TYPE_FLOAT3:
                    elem_cnt = 3;
                    break;
                }

                member_access->object = arr_access;
                xyzw[property[i].bufferRegCount + elem_cnt] = 0;
                member_access->field = ast->AddString(xyzw + property[i].bufferRegCount);

                Snprintf(buf_name, sizeof(buf_name), "VP_Buffer%u", property[i].bufferindex);
                arr->name = ast->AddString(buf_name);
                arr->global = true;

                idx->type = M4::HLSLBaseType_Int;
                idx->iValue = property[i].bufferReg;

                arr_access->array = arr;
                arr_access->index = idx;

                    #if DO_FLOAT4_CAST
                M4::HLSLCastingExpression* cast_expr = ast->AddNode<M4::HLSLCastingExpression>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                cast_expr->expression = arr_access;
                cast_expr->type.baseType = M4::HLSLBaseType_Float4;
                member_access->object = cast_expr;

                prop_decl[i].decl->assignment = member_access;
                prop_decl[i].decl->type.flags |= M4::HLSLTypeFlag_Static | M4::HLSLTypeFlag_Property;
                    #else
                prop_decl[i].decl->assignment = member_access;
                prop_decl[i].decl->type.flags |= M4::HLSLTypeFlag_Static | M4::HLSLTypeFlag_Property;
                    #endif
            }
            break;

            case rhi::ShaderProp::TYPE_FLOAT4X4:
            {
                M4::HLSLConstructorExpression* ctor = ast->AddNode<M4::HLSLConstructorExpression>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                M4::HLSLArrayAccess* arr_access[4];
                M4::HLSLCastingExpression* cast_expr[4];

                ctor->type.baseType = M4::HLSLBaseType_Float4x4;

                for (unsigned k = 0; k != 4; ++k)
                {
                    arr_access[k] = ast->AddNode<M4::HLSLArrayAccess>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);

                    M4::HLSLLiteralExpression* idx = ast->AddNode<M4::HLSLLiteralExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                    M4::HLSLIdentifierExpression* arr = ast->AddNode<M4::HLSLIdentifierExpression>(prop_decl[0].decl->fileName, prop_decl[0].decl->line);
                    char buf_name[128];

                    Snprintf(buf_name, sizeof(buf_name), "VP_Buffer%u", property[i].bufferindex);
                    arr->name = ast->AddString(buf_name);

                    idx->type = M4::HLSLBaseType_Int;
                    idx->iValue = property[i].bufferReg + k;

                    arr_access[k]->array = arr;
                    arr_access[k]->index = idx;
                }

                    #if DO_FLOAT4_CAST
                for (unsigned k = 0; k != 4; ++k)
                {
                    cast_expr[k] = ast->AddNode<M4::HLSLCastingExpression>(prop_decl[i].decl->fileName, prop_decl[i].decl->line);
                    cast_expr[k]->expression = arr_access[k];
                    cast_expr[k]->type.baseType = M4::HLSLBaseType_Float4;
                }

                ctor->argument = cast_expr[0];
                for (unsigned k = 0; k != 4 - 1; ++k)
                    cast_expr[k]->nextExpression = cast_expr[k + 1];
                    #else
                ctor->argument = arr_access[0];
                for (unsigned k = 0; k != 4 - 1; ++k)
                    arr_access[k]->nextExpression = arr_access[k + 1];
                    #endif

                prop_decl[i].decl->assignment = ctor;
                prop_decl[i].decl->type.flags |= M4::HLSLTypeFlag_Static | M4::HLSLTypeFlag_Property;
            }
            break;
            }
        }
    }
#endif

#if 1
    Logger::Info("properties (%u) :", property.size());
    for (std::vector<rhi::ShaderProp>::const_iterator p = property.begin(), p_end = property.end(); p != p_end; ++p)
    {
        if (p->type == rhi::ShaderProp::TYPE_FLOAT4 || p->type == rhi::ShaderProp::TYPE_FLOAT4X4)
        {
            if (p->arraySize == 1)
            {
                Logger::Info("  %-16s    buf#%u  -  %u, %u x float4", p->uid.c_str(), p->bufferindex, p->bufferReg, p->bufferRegCount);
            }
            else
            {
                char name[128];

                Snprintf(name, sizeof(name) - 1, "%s[%u]", p->uid.c_str(), p->arraySize);
                Logger::Info("  %-16s    buf#%u  -  %u, %u x float4", name, p->bufferindex, p->bufferReg, p->bufferRegCount);
            }
        }
        else
        {
            const char* xyzw = "xyzw";

            switch (p->type)
            {
            case rhi::ShaderProp::TYPE_FLOAT1:
                Logger::Info("  %-16s    buf#%u  -  %u, %c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount]);
                break;

            case rhi::ShaderProp::TYPE_FLOAT2:
                Logger::Info("  %-16s    buf#%u  -  %u, %c%c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount + 0], xyzw[p->bufferRegCount + 1]);
                break;

            case rhi::ShaderProp::TYPE_FLOAT3:
                Logger::Info("  %-16s    buf#%u  -  %u, %c%c%c", p->uid.c_str(), p->bufferindex, p->bufferReg, xyzw[p->bufferRegCount + 0], xyzw[p->bufferRegCount + 1], xyzw[p->bufferRegCount + 2]);
                break;

            default:
                break;
            }
        }
    }

    Logger::Info("\n--- ShaderSource");
    Logger::Info("buffers (%u) :", buf.size());
    for (unsigned i = 0; i != buf.size(); ++i)
    {
        Logger::Info("  buf#%u  reg.count = %u", i, buf[i].regCount);
    }

    Logger::Info("samplers (%u) :", sampler.size());
    for (unsigned s = 0; s != sampler.size(); ++s)
    {
        Logger::Info("  sampler#%u  \"%s\"", s, sampler[s].uid.c_str());
    }
    Logger::Info("\n\n");
#endif
};

void
GameCore::_test_parser()
{
    #define SHADER_VERTEX 1
    #define SHADER_PIXEL 2
//    #define SHADER_TYPE  SHADER_VERTEX
    #define SHADER_TYPE SHADER_PIXEL

// Read input file.
//    const char* filename = "test.fx";
    #if SHADER_TYPE == SHADER_VERTEX
    const char* filename = "vp-test.fx";
    #else
    const char* filename = "fp-test.fx";
    #endif

    size_t length;
    const char* buffer = readFile(filename, &length);

    M4::HLSLTree tree(&M4_Allocator);
    M4::HLSLParser parser(&M4_Allocator, filename, buffer, length);

    if (!parser.Parse(&tree))
    {
        M4::Log_Error("Parse error.\n");
    }

    _ProcessProperties(&tree);

    MyVisitor visitor;

    visitor.VisitRoot(tree.GetRoot());

    M4::Array<M4::HLSLFunctionCall*> fcall(&M4_Allocator);
    const char* func_name = "p_offset";

    tree.FindFunctionCall(func_name, &fcall);
    for (unsigned f = 0; f != fcall.GetSize(); ++f)
    {
        Log_Error("\"%s\" call = %p\n", func_name, fcall[f]);
    }

    class
    FindStatementExpression
    : public M4::HLSLTreeVisitor
    {
    public:
        FindStatementExpression(M4::HLSLFunctionCall* fc)
            : _fcall(fc)
            , _expr(&M4_Allocator)
            , _cur_expr(NULL)
            , _cur_statement(NULL)
            , _cur_statement_parent(NULL)
            , expr(NULL)
            , statement(NULL)
        {
        }

        M4::HLSLExpression* expr;
        M4::HLSLStatement* statement;

        virtual void VisitStatements(M4::HLSLStatement* statement)
        {
            _cur_statement = statement;
            HLSLTreeVisitor::VisitStatements(statement);
        }
        virtual void VisitExpressionStatement(M4::HLSLExpressionStatement* node)
        {
            _cur_statement = node;
            M4::HLSLTreeVisitor::VisitExpressionStatement(node);
        }
        virtual void VisitBlockStatement(M4::HLSLBlockStatement* node)
        {
            _cur_statement = node->statement;
            _cur_statement_parent = node;
            M4::HLSLTreeVisitor::VisitBlockStatement(node);
        }
        virtual void VisitExpression(M4::HLSLExpression* node)
        {
            _expr.PushBack(node);
            M4::HLSLTreeVisitor::VisitExpression(node);
            if (_expr.GetSize())
                _expr.Resize(_expr.GetSize() - 1);
        }
        virtual void VisitFunctionCall(M4::HLSLFunctionCall* node)
        {
            if (node == _fcall)
            {
                if (_expr.GetSize() > 1)
                    expr = _expr[_expr.GetSize() - 2];
                statement = _cur_statement;
            }
            HLSLTreeVisitor::VisitFunctionCall(node);
        }

    private:
        M4::Array<M4::HLSLExpression*> _expr;

        M4::HLSLFunctionCall* _fcall;
        M4::HLSLExpression* _cur_expr;
        M4::HLSLStatement* _cur_statement;
        M4::HLSLStatement* _cur_statement_parent;
    };

    for (unsigned f = 0; f != fcall.GetSize(); ++f)
    {
        FindStatementExpression find_statement_expression(fcall[f]);

        find_statement_expression.VisitRoot(tree.GetRoot());

        if (find_statement_expression.statement)
        {
            class
            FindParentPrev
            : public M4::HLSLTreeVisitor
            {
            public:
                M4::HLSLStatement* target;

                M4::HLSLStatement* parent;
                M4::HLSLStatement* prev;

                FindParentPrev()
                    : target(nullptr)
                    , parent(nullptr)
                    , prev(nullptr)
                {
                }

                virtual void VisitFunction(M4::HLSLFunction* func)
                {
                    M4::HLSLTreeVisitor::VisitFunction(func);
                }
                virtual void VisitStatements(M4::HLSLStatement* statement)
                {
                    for (M4::HLSLStatement* s = statement; s; s = s->nextStatement)
                    {
                        if (s->nextStatement == target)
                        {
                            prev = s;
                            break;
                        }
                    }

                    switch (statement->nodeType)
                    {
                    case M4::HLSLNodeType_BlockStatement:
                    {
                        M4::HLSLBlockStatement* block = (M4::HLSLBlockStatement*)statement;

                        if (block->statement == target)
                        {
                            parent = statement;
                        }
                        else
                        {
                            for (M4::HLSLStatement* bs = block->statement; bs; bs = bs->nextStatement)
                            {
                                if (bs->nextStatement == target)
                                {
                                    parent = block;
                                    prev = bs;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                    }

                    M4::HLSLTreeVisitor::VisitStatements(statement);
                }
                virtual void VisitBlockStatement(M4::HLSLBlockStatement* node)
                {
                    M4::HLSLBlockStatement* block = (M4::HLSLBlockStatement*)node;

                    if (block->statement == target)
                    {
                        parent = block;
                    }
                    else
                    {
                        for (M4::HLSLStatement* bs = block->statement; bs; bs = bs->nextStatement)
                        {
                            if (bs->nextStatement == target)
                            {
                                parent = block;
                                prev = bs;
                                break;
                            }
                        }
                    }
                }
            };

            FindParentPrev find_parent_prev;
            find_parent_prev.target = find_statement_expression.statement;
            find_parent_prev.VisitRoot(tree.GetRoot());

            M4::HLSLStatement* statement = find_statement_expression.statement;
            M4::HLSLDeclaration* rv_decl = tree.AddNode<M4::HLSLDeclaration>(statement->fileName, statement->line);
            M4::HLSLBlockStatement* func_block = tree.AddNode<M4::HLSLBlockStatement>(statement->fileName, statement->line);

            rv_decl->type.baseType = M4::HLSLBaseType_Float3;
            rv_decl->name = "__temp";

            switch (find_statement_expression.expr->nodeType)
            {
            case M4::HLSLNodeType_BinaryExpression:
            {
                M4::HLSLBinaryExpression* bin = (M4::HLSLBinaryExpression*)(find_statement_expression.expr);
                M4::HLSLExpression** ce = NULL;

                if (bin->expression1 == fcall[f])
                    ce = &(bin->expression1);
                else if (bin->expression2 == fcall[f])
                    ce = &(bin->expression2);

                DVASSERT(ce);

                M4::HLSLIdentifierExpression* val = tree.AddNode<M4::HLSLIdentifierExpression>(statement->fileName, statement->line);

                val->name = "__retval";
                (*ce) = val;
            }
            break;
            }

            M4::HLSLStatement* func_parent = NULL;
            M4::HLSLFunction* func = tree.FindFunction(func_name, &func_parent);
            M4::HLSLStatement* func_ret_parent = NULL;
            M4::HLSLExpression* func_ret = NULL;
            DVASSERT(func);
            DVASSERT(func_parent);

            M4::HLSLStatement* sp = NULL;
            for (M4::HLSLStatement* fs = func->statement; fs; fs = fs->nextStatement)
            {
                if (fs->nodeType == M4::HLSLNodeType_ReturnStatement)
                {
                    func_ret = ((M4::HLSLReturnStatement*)(fs))->expression;
                    func_ret_parent = sp;
                    break;
                }
                sp = fs;
            }
            DVASSERT(func_ret);
            DVASSERT(func_ret_parent);

            M4::Array<M4::HLSLDeclaration*> arg(&M4_Allocator);

            {
                int a = 0;
                for (M4::HLSLExpression *e = fcall[f]->argument; e; e = e->nextExpression, ++a)
                {
                    M4::HLSLDeclaration* arg_decl = tree.AddNode<M4::HLSLDeclaration>(statement->fileName, statement->line);

                    int aa = 0;
                    for (M4::HLSLArgument *fa = func->argument; fa; fa = fa->nextArgument, ++aa)
                    {
                        if (aa == a)
                        {
                            arg_decl->name = fa->name;
                            arg_decl->type = fa->type;
                            break;
                        }
                    }

                    arg_decl->assignment = e;
                    arg.PushBack(arg_decl);
                }
            }

            M4::HLSLBinaryExpression* ret_ass = tree.AddNode<M4::HLSLBinaryExpression>(statement->fileName, statement->line);
            M4::HLSLIdentifierExpression* ret_var = tree.AddNode<M4::HLSLIdentifierExpression>(statement->fileName, statement->line);
            M4::HLSLExpressionStatement* ret_statement = tree.AddNode<M4::HLSLExpressionStatement>(statement->fileName, statement->line);

            ret_statement->expression = ret_ass;
            ret_var->name = "__retval";
            ret_ass->binaryOp = M4::HLSLBinaryOp_Assign;
            ret_ass->expression1 = ret_var;
            ret_ass->expression2 = func_ret;

            func_ret_parent->nextStatement = ret_statement;
            func_parent->nextStatement = func->nextStatement; // remove func definition

            if (find_parent_prev.parent && !find_parent_prev.prev)
            {
                DVASSERT(find_parent_prev.parent->nodeType == M4::HLSLNodeType_BlockStatement);
                M4::HLSLBlockStatement* block = (M4::HLSLBlockStatement*)(find_parent_prev.parent);
                M4::HLSLStatement* next = block->statement;

                block->statement = rv_decl;
                rv_decl->nextStatement = func_block;
                func_block->nextStatement = next;
            }
            else
            {
                find_parent_prev.prev->nextStatement = rv_decl;
                rv_decl->nextStatement = func_block;
                func_block->nextStatement = statement;
            }

            DVASSERT(arg.GetSize());
            func_block->statement = arg[0];
            for (int i = 1; i < arg.GetSize(); ++i)
            {
                arg[i - 1]->nextStatement = arg[i];
            }
            arg[arg.GetSize() - 1]->nextStatement = func->statement;
        }
    }

    M4::HLSLGenerator hlsl_gen(&M4_Allocator);
    M4::GLESGenerator gles_gen(&M4_Allocator);
    M4::MSLGenerator msl_gen(&M4_Allocator);

    #if SHADER_TYPE == SHADER_VERTEX
    M4::HLSLGenerator::Target hlsl_target = M4::HLSLGenerator::Target_VertexShader;
    M4::GLESGenerator::Target gles_target = M4::GLESGenerator::Target_VertexShader;
    M4::MSLGenerator::Target msl_target = M4::MSLGenerator::Target_VertexShader;
    #else
    M4::HLSLGenerator::Target hlsl_target = M4::HLSLGenerator::Target_PixelShader;
    M4::GLESGenerator::Target gles_target = M4::GLESGenerator::Target_FragmentShader;
    M4::MSLGenerator::Target msl_target = M4::MSLGenerator::Target_PixelShader;
    #endif

    if (hlsl_gen.Generate(&tree, hlsl_target, "main", false))
    {
        puts(hlsl_gen.GetResult());
        ::OutputDebugStringA("\n\n---- HLSL\n");
        ::OutputDebugStringA(hlsl_gen.GetResult());
    }
    else
    {
        M4::Log_Error("HLSL-gen error\n");
    }

    /*
    if (gles_gen.Generate(&tree, gles_target, "main"))
    {
        puts(gles_gen.GetResult());
        ::OutputDebugStringA("\n\n---- GLSL(ES)\n");
        ::OutputDebugStringA(gles_gen.GetResult());
    }
    else
    {
        M4::Log_Error("GLSL-gen error\n");
    }
*/

    if (msl_gen.Generate(&tree, msl_target, "main"))
    {
        puts(msl_gen.GetResult());
        ::OutputDebugStringA("\n\n---- Metal\n");
        ::OutputDebugStringA(msl_gen.GetResult());
    }
    else
    {
        M4::Log_Error("MSL-gen error\n");
    }
}

void GameCore::OnAppStarted()
{
    _test_parser();
    exit(0);

    DbgDraw::EnsureInited();
    
    #if defined(__DAVAENGINE_WIN32__)
    {
        KeyedArchive* opt = new KeyedArchive();
        char title[128] = "ParserTest  -  ";

        switch (rhi::HostApi())
        {
        case rhi::RHI_DX9:
            strcat(title, "DX9");
            break;
        case rhi::RHI_DX11:
            strcat(title, "DX11");
            break;
        case rhi::RHI_GLES2:
            strcat(title, "GL");
            break;
        }

        opt->SetInt32("fullscreen", 0);
        opt->SetInt32("bpp", 32);
        opt->SetString(String("title"), String(title));

        DAVA::Core::Instance()->SetOptions(opt);
    }
    #endif
}

void GameCore::OnAppFinished()
{
    DbgDraw::Uninitialize();
    //-    rhi::Uninitialize();
}

void GameCore::OnSuspend()
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    ApplicationCore::OnSuspend();
#endif
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}


#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
void GameCore::OnDeviceLocked()
{
    Core::Instance()->Quit();
}

void GameCore::OnBackground()
{
}

void GameCore::OnForeground()
{
    ApplicationCore::OnForeground();
}

#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

void GameCore::Update(float32 timeElapsed)
{
}

void GameCore::BeginFrame()
{
}

void GameCore::Draw()
{
}

void GameCore::EndFrame()
{
    rhi::Present();
}
