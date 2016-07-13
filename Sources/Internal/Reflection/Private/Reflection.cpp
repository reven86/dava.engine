#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Logger/Logger.h"

namespace ReflectionPrinterDetails
{
struct TypePrinter
{
    const DAVA::Type* type;
    void (*printFn)(char* buf, size_t sz, const DAVA::Any& any);
};

TypePrinter* GetTypePrinter(const DAVA::Type* type)
{
    static TypePrinter default_pointer_printer = { DAVA::Type::Instance<void*>(), [](char* buf, size_t sz, const DAVA::Any& any) { Snprintf(buf, sz, "0x%08p", any.Get<void*>()); } };

    static DAVA::Vector<TypePrinter> pointer_printes = {
        { DAVA::Type::Instance<char*>(), [](char* buf, size_t sz, const DAVA::Any& any) { Snprintf(buf, sz, "%s", any.Get<char*>()); } }
    };

    static DAVA::Vector<TypePrinter> printers = {
        { DAVA::Type::Instance<DAVA::int32>(), [](char* buf, size_t sz, const DAVA::Any& any) { Snprintf(buf, sz, "%d", any.Get<DAVA::int32>()); } },
        { DAVA::Type::Instance<DAVA::uint32>(), [](char* buf, size_t sz, const DAVA::Any& any) { Snprintf(buf, sz, "%u", any.Get<DAVA::uint32>()); } },
        { DAVA::Type::Instance<DAVA::float32>(), [](char* buf, size_t sz, const DAVA::Any& any) { Snprintf(buf, sz, "%g", any.Get<DAVA::float32>()); } },
        { DAVA::Type::Instance<DAVA::float64>(), [](char* buf, size_t sz, const DAVA::Any& any) { Snprintf(buf, sz, "%g", any.Get<DAVA::float64>()); } },
        { DAVA::Type::Instance<size_t>(), [](char* buf, size_t sz, const DAVA::Any& any) { Snprintf(buf, sz, "%llu", static_cast<DAVA::uint64>(any.Get<size_t>())); } },
        { DAVA::Type::Instance<DAVA::String>(), [](char* buf, size_t sz, const DAVA::Any& any) { Snprintf(buf, sz, "%s", any.Get<DAVA::String>().c_str()); } }
    };

    TypePrinter* ret = nullptr;

    if (nullptr != type)
    {
        if (type->IsPointer())
        {
            for (size_t i = 0; i < pointer_printes.size(); ++i)
            {
                if (pointer_printes[i].type == type || pointer_printes[i].type == type->Decay())
                {
                    ret = &pointer_printes[i];
                    break;
                }
            }

            if (nullptr == ret)
            {
                ret = &default_pointer_printer; // get void* printer
            }
        }
        else
        {
            for (size_t i = 0; i < printers.size(); ++i)
            {
                if (printers[i].type == type || printers[i].type == type->Decay())
                {
                    ret = &printers[i];
                    break;
                }
            }
        }
    }

    return ret;
}

const char* AnyToStr(const DAVA::Any& any)
{
    static char buf[1024];
    buf[0] = 0;

    const DAVA::Type* type = any.GetType();
    TypePrinter* printer = GetTypePrinter(type);

    if (nullptr != printer)
    {
        printer->printFn(buf, sizeof(buf), any);
    }
    else
    {
        Snprintf(buf, sizeof(buf), "?");
    }

    return buf;
}

const char* ReflectionToStr(const DAVA::Reflection& ref)
{
    static char buf[1024];
    buf[0] = 0;

    const DAVA::Type* type = ref.GetValueType();
    TypePrinter* printer = GetTypePrinter(type);

    if (nullptr != printer && ref.IsValid())
    {
        printer->printFn(buf, sizeof(buf), ref.GetValue());
    }
    else
    {
        Snprintf(buf, sizeof(buf), "?");
    }

    return buf;
}

void PrintNameAndValue(std::ostream& out, const char* name, const char* value, const DAVA::Type* type, int level)
{
    static char buf[1024];

    int n = 0;
    for (int i = 0; i < level; ++i)
    {
        n += Snprintf(buf + n, sizeof(buf) - n, "  ");
    }

    if (nullptr != name)
    {
        n += Snprintf(buf + n, sizeof(buf) - n, "%s", name);
    }

    for (int i = n; i < 40; ++i)
    {
        n += Snprintf(buf + n, sizeof(buf) - n, " ");
    }

    n += Snprintf(buf + n, sizeof(buf) - n, " : ");

    if (nullptr != value)
    {
        n += Snprintf(buf + n, sizeof(buf) - n, "%s", value);
    }

    for (int i = n; i < 80; ++i)
    {
        n += Snprintf(buf + n, sizeof(buf) - n, " ");
    }

    if (nullptr != type)
    {
        n += Snprintf(buf + n, sizeof(buf) - n, "| [%.25s]", type->GetName());
    }

    out << buf << std::endl;
    DAVA::Logger::Info(buf);
}

void Dump(std::ostream& out, const char* name, const DAVA::Reflection& ref, int level)
{
    PrintNameAndValue(out, name, ReflectionToStr(ref), ref.GetValueType(), level);

    DAVA::ReflectedObject vobject = ref.GetValueObject();
    const DAVA::StructureWrapper* structWrapper = ref.GetStructure();
    if (structWrapper != nullptr)
    {
        DAVA::Ref::FieldsList fields = structWrapper->GetFields(vobject);

        for (size_t i = 0; i < fields.size(); ++i)
        {
            Dump(out, AnyToStr(fields[i].key), fields[i].valueRef, level + 1);
        }
    }
}
}

namespace DAVA
{
void Reflection::Dump(std::ostream& out)
{
    ReflectionPrinterDetails::Dump(out, "Reflection", *this, 0);
}

} // namespace DAVA
