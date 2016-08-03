#include <iomanip>
#include <algorithm>
#include <cstring>

#include "Reflection/Reflection.h"

namespace DAVA
{
namespace ReflectionDetail
{
struct Dumper
{
    using PrinterFn = void (*)(std::ostringstream&, const Any&);
    using PrintersTable = Map<const Type*, PrinterFn>;

    static const PrintersTable pointerPrinters;
    static const PrintersTable valuePrinters;

    static std::pair<PrinterFn, PrinterFn> GetPrinterFns(const Type* type)
    {
        std::pair<PrinterFn, PrinterFn> ret = { nullptr, nullptr };

        if (nullptr != type)
        {
            const PrintersTable* pt = &valuePrinters;
            const Type* keyType = type;
            if (type->IsPointer())
            {
                pt = &pointerPrinters;
                type = type->Deref();
            }

            if (nullptr != type->Decay())
                type = type->Decay();

            auto it = pt->find(type);
            if (it != pt->end())
            {
                ret.first = it->second;
            }

            ret.second = pt->at(Type::Instance<void>());
        }
        else
        {
            ret.second = [](std::ostringstream& out, const Any&)
            {
                out << "__null__";
            };
        }

        return ret;
    }

    static void DumpAny(std::ostringstream& out, const Any& any)
    {
        std::ostringstream line;
        std::pair<PrinterFn, PrinterFn> fns = GetPrinterFns(any.GetType());

        if (fns.first != nullptr)
        {
            (*fns.first)(line, any);
        }
        else
        {
            (*fns.second)(line, any);
        }

        out << line.str();
    }

    static void DumpRef(std::ostringstream& out, const Reflection& ref)
    {
        if (ref.IsValid())
        {
            const Type* valueType = ref.GetValueType();

            std::ostringstream line;
            std::pair<PrinterFn, PrinterFn> fns = GetPrinterFns(valueType);

            if (nullptr != fns.first)
            {
                (*fns.first)(line, ref.GetValue());
            }
            else
            {
                if (valueType->IsPointer())
                {
                    (*fns.second)(line, ref.GetValue());
                }
                else
                {
                    (*fns.second)(line, Any());
                }
            }

            out << line.str();
        }
        else
        {
            out << "__invalid__";
        }
    }

    static void DumpType(std::ostringstream& out, const Reflection& ref)
    {
        std::ostringstream line;

        if (ref.IsValid())
        {
            const Type* valueType = ref.GetValueType();
            const char* typeName = valueType->GetName();

            std::streamsize w = 40;
            if (0 != out.width())
            {
                w = std::min(out.width(), w);
            }

            line << "(";

            if (::strlen(typeName) > w)
            {
                line.write(typeName, w);
            }
            else
            {
                line << typeName;
            }

            line << ")";

            out << line.str();
        }
    }

    static void PrintHierarhy(std::ostringstream& out, const char* symbols, size_t level, int colWidth, bool isLastRow)
    {
        static const char* defaultSymbols = "    ";

        if (nullptr == symbols)
        {
            symbols = defaultSymbols;
        }

        if (level > 0)
        {
            for (size_t i = 0; i < level - 1; ++i)
            {
                out << std::setw(colWidth);
                out << std::left << symbols[1];
            }

            if (!isLastRow)
            {
                out << std::setw(colWidth);
                out << std::setfill(symbols[0]);
                out << std::left << symbols[2];
            }
            else
            {
                out << std::setw(colWidth);
                out << std::setfill(symbols[0]);
                out << std::left << symbols[3];
            }
        }

        out << std::setfill(' ');
    }

    static void Dump(std::ostream& out, const Reflection::Field& field, size_t level, size_t maxlevel, bool isLastRow = false)
    {
        if (level <= maxlevel || 0 == maxlevel)
        {
            const size_t hierarchyColWidth = 4;
            const size_t nameColWidth = 30;
            const size_t valueColWidth = 25;
            const size_t typeColWidth = 20;

            std::ostringstream line;

            bool hasChildren = field.ref.IsValid() && field.ref.HasFields();

            // print hierarchy
            PrintHierarhy(line, nullptr, level, hierarchyColWidth, isLastRow);

            // print key
            line << std::setw(nameColWidth - level * hierarchyColWidth) << std::left;
            if (0 == maxlevel || !hasChildren)
            {
                DumpAny(line, field.key);
            }
            else
            {
                std::ostringstream name;
                DumpAny(name, field.key);

                if ((level + 1) <= maxlevel)
                {
                    line << name.str() + "[-]";
                }
                else
                {
                    line << name.str() + "[+]";
                }
            }

            // delimiter
            line << " = ";

            // print value
            line << std::setw(valueColWidth) << std::left;
            DumpRef(line, field.ref);

            // print type
            line << std::setw(typeColWidth);
            DumpType(line, field.ref);

            // endl
            out << line.str() << std::endl;

            // children
            if (hasChildren)
            {
                Vector<Reflection::Field> children = field.ref.GetFields();
                for (size_t i = 0; i < children.size(); ++i)
                {
                    bool isLast = (i == (children.size() - 1));
                    Dump(out, children[i], level + 1, maxlevel, isLast);
                }
            }
        }
    }
};

const Dumper::PrintersTable Dumper::valuePrinters = {
    { Type::Instance<int32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<int32>(); } },
    { Type::Instance<uint32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<uint32>(); } },
    { Type::Instance<int64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<int64>(); } },
    { Type::Instance<uint64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<uint64>(); } },
    { Type::Instance<float32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<float32>(); } },
    { Type::Instance<float64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<float64>(); } },
    { Type::Instance<String>(), [](std::ostringstream& out, const Any& any) { out << any.Get<String>().c_str(); } },
    { Type::Instance<size_t>(), [](std::ostringstream& out, const Any& any) { out << any.Get<size_t>(); } },
    { Type::Instance<void>(), [](std::ostringstream& out, const Any& any) { out << "???"; } }
};

const Dumper::PrintersTable Dumper::pointerPrinters = {
    { Type::Instance<char>(), [](std::ostringstream& out, const Any& any) { out << any.Get<const char*>(); } },
    { Type::Instance<void>(), [](std::ostringstream& out, const Any& any) { out << "0x" << std::setw(8) << std::setfill('0') << std::hex << any.Get<void*>(); } }
};

} // ReflectionDetail

void Reflection::Dump(std::ostream& out, size_t maxlevel) const
{
    ReflectionDetail::Dumper::Dump(out, { "this", *this }, 0, maxlevel);
}

void Reflection::DumpMethods(std::ostream& out) const
{
    Vector<Method> methods = GetMethods();
    for (auto& method : methods)
    {
        const AnyFn::InvokeParams& params = method.fn.GetInvokeParams();

        out << params.retType->GetName() << " ";
        out << method.key << "(";

        for (size_t i = 0; i < params.argsType.size(); ++i)
        {
            out << params.argsType[i]->GetName();

            if (i < (params.argsType.size() - 1))
            {
                out << ", ";
            }
        }

        out << ");" << std::endl;
    }
}

} // namespace DAVA
