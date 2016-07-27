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
    #include "M4/GLSLGenerator.h"
    #include "M4/HLSLGenerator.h"
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
    //    virtual void VisitTopLevelStatement(HLSLStatement * node);
    //    virtual void VisitStatements(HLSLStatement * statement);
    //    virtual void VisitStatement(HLSLStatement * node);
    virtual void VisitDeclaration(M4::HLSLDeclaration* node)
    {
        M4::HLSLDeclaration* decl = (M4::HLSLDeclaration*)node;

        M4::Log_Error("%sdecl  \"%s\"\n", _IndentString(indent), node->name);
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
    //    virtual void VisitSamplerState(HLSLSamplerState * node);
    //    virtual void VisitPass(HLSLPass * node);
    //    virtual void VisitTechnique(HLSLTechnique * node);

    //    virtual void VisitFunctions(HLSLRoot * root);
    //    virtual void VisitParameters(HLSLRoot * root);
};

void
GameCore::_test_parser()
{
    // Read input file.
    const char* filename = "test.fx";
    size_t length;
    const char* buffer = readFile(filename, &length);

    M4::HLSLTree tree(&M4_Allocator);
    M4::HLSLParser parser(&M4_Allocator, filename, buffer, length);

    if (!parser.Parse(&tree))
    {
        M4::Log_Error("Parse error.\n");
    }

    MyVisitor visitor;

    visitor.VisitRoot(tree.GetRoot());

    // find properties
    {
        M4::Log_Error("properties :\n");
        M4::HLSLStatement* statement = tree.GetRoot()->statement;
        M4::HLSLStatement* pstatement = NULL;

        while (statement)
        {
            if (statement->nodeType == M4::HLSLNodeType_Declaration)
            {
                M4::HLSLDeclaration* decl = (M4::HLSLDeclaration*)statement;

                if (decl->type.flags & M4::HLSLTypeFlag_Property)
                {
                    M4::Log_Error("  property \"%s\"  {", decl->name);
                    for (M4::HLSLAttribute* a = decl->attributes; a; a = a->nextAttribute)
                    {
                        M4::Log_Error(" \"%s\"%s", a->attrText, (a->nextAttribute) ? "," : "");
                    }
                    M4::Log_Error(" }\n");
                }
            }

            pstatement = statement;
            statement = statement->nextStatement;
        }
    }

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
            ;

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

    if (hlsl_gen.Generate(&tree, M4::HLSLGenerator::Target_VertexShader, "vp_main", true))
    {
        puts(hlsl_gen.GetResult());
        ::OutputDebugStringA("\n\n---- HLSL\n");
        ::OutputDebugStringA(hlsl_gen.GetResult());
    }
    else
    {
        M4::Log_Error("Generation error.\n");
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
