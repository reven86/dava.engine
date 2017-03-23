#include <iomanip>
#include <algorithm>
#include <cstring>

#include "Reflection/Reflection.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Reflection/Private/Wrappers/ValueWrapperDirect.h"
#include "Reflection/Private/Wrappers/StructureWrapperDefault.h"

namespace DAVA
{
namespace ReflectedTypeDBDetail
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

            if (::strlen(typeName) > static_cast<size_t>(w))
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

    static void PrintHierarhy(std::ostringstream& out, size_t level, int colWidth)
    {
        if (level > 0)
        {
            for (size_t i = 0; i < level; ++i)
            {
                out << std::setw(colWidth);
                out << std::left << ' ';
            }
        }

        out << std::setfill(' ');
    }

    static void Dump(std::ostream& out, const Reflection::Field& field, size_t level, size_t maxlevel)
    {
        if (level <= maxlevel || 0 == maxlevel)
        {
            const size_t hierarchyColWidth = 2;
            const size_t nameColWidth = 40;
            const size_t valueColWidth = 25;
            const size_t typeColWidth = 20;

            std::ostringstream line;

            bool hasChildren = field.ref.IsValid() && field.ref.HasFields();

            // print hierarchy
            PrintHierarhy(line, level, hierarchyColWidth);

            // print key setup
            line << std::setw(nameColWidth - level * hierarchyColWidth) << std::left;

            // print key
            {
                std::ostringstream name;

                if (field.inheritFrom != nullptr)
                {
                    name << std::setw(6) << field.inheritFrom->GetType()->GetName() << "::";
                }

                DumpAny(name, field.key);

                if (0 == maxlevel || !hasChildren)
                {
                    line << name.str();
                }
                else
                {
                    if ((level + 1) <= maxlevel)
                    {
                        line << name.str() + "[-]";
                    }
                    else
                    {
                        line << name.str() + "[+]";
                    }
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
            out << line.str() << "\n";

            // children
            if (hasChildren)
            {
                Vector<Reflection::Field> children = field.ref.GetFields();
                for (size_t i = 0; i < children.size(); ++i)
                {
                    Dump(out, children[i], level + 1, maxlevel);
                }
            }

            // methods
            Vector<Reflection::Method> methods = field.ref.GetMethods();
            for (auto& method : methods)
            {
                std::ostringstream methodline;
                const AnyFn::Params& params = method.fn.GetInvokeParams();

                // print hierarchy
                PrintHierarhy(methodline, level + 1, hierarchyColWidth);
                methodline << "{} ";
                methodline << method.key.c_str() << "(";

                for (size_t i = 0; i < params.argsType.size(); ++i)
                {
                    methodline << params.argsType[i]->GetName();
                    if (i < (params.argsType.size() - 1))
                    {
                        methodline << ", ";
                    }
                }
                methodline << ") -> ";
                methodline << params.retType->GetName();

                out << methodline.str() << "\n";
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

Reflection::Reflection(const ReflectedObject& object_, const ValueWrapper* vw, const StructureWrapper* sw, const ReflectedMeta* meta_)
    : object(object_)
    , valueWrapper(vw)
    , structureWrapper(sw)
    , meta(meta_)
{
    // try to get structureWrapper from object reflected type
    const ReflectedType* reflectedType = valueWrapper->GetValueObject(object).GetReflectedType();
    if (nullptr != reflectedType)
    {
        if (nullptr == structureWrapper)
        {
            structureWrapper = reflectedType->GetStrucutreWrapper();
        }

        if (nullptr == meta && nullptr != reflectedType->GetStructure())
        {
            meta = reflectedType->GetStructure()->meta.get();
        }
    }

    /*
    if (nullptr != objectMeta)
    {
        if (objectMeta->HasMeta<StructureWrapper>())
        {
            structureWrapper = objectMeta->GetMeta<StructureWrapper>();
        }
    }
    */

    // in still no structureWrapper use empty one
    if (nullptr == structureWrapper)
    {
        static StructureWrapperDefault emptyStructureWrapper;
        structureWrapper = &emptyStructureWrapper;
    }
}

bool Reflection::HasFields() const
{
    return structureWrapper->HasFields(object, valueWrapper);
}

Reflection Reflection::GetField(const Any& key) const
{
    return structureWrapper->GetField(object, valueWrapper, key);
}

Vector<Reflection::Field> Reflection::GetFields() const
{
    return structureWrapper->GetFields(object, valueWrapper);
}

const Reflection::FieldCaps& Reflection::GetFieldsCaps() const
{
    return structureWrapper->GetFieldsCaps(object, valueWrapper);
}

AnyFn Reflection::GetFieldCreator() const
{
    return structureWrapper->GetFieldCreator(object, valueWrapper);
}

bool Reflection::AddField(const Any& key, const Any& value) const
{
    return structureWrapper->AddField(object, valueWrapper, key, value);
}

bool Reflection::InsertField(const Any& beforeKey, const Any& key, const Any& value) const
{
    return structureWrapper->InsertField(object, valueWrapper, beforeKey, key, value);
}

bool Reflection::RemoveField(const Any& key) const
{
    return structureWrapper->RemoveField(object, valueWrapper, key);
}

bool Reflection::HasMethods() const
{
    return structureWrapper->HasMethods(object, valueWrapper);
}

AnyFn Reflection::GetMethod(const String& key) const
{
    return structureWrapper->GetMethod(object, valueWrapper, key);
}

Vector<Reflection::Method> Reflection::GetMethods() const
{
    return structureWrapper->GetMethods(object, valueWrapper);
}

void Reflection::Dump(std::ostream& out, size_t maxlevel) const
{
    ReflectedTypeDBDetail::Dumper::Dump(out, Reflection::Field(Any("this"), Reflection(*this), nullptr), 0, maxlevel);
}

Reflection Reflection::Create(const ReflectedObject& object, const ReflectedMeta* objectMeta)
{
    if (object.IsValid())
    {
        static ValueWrapperObject objectValueWrapper;
        return Reflection(object, &objectValueWrapper, nullptr, objectMeta);
    }

    return Reflection();
}

Reflection Reflection::Create(const Any& any, const ReflectedMeta* objectMeta)
{
    static ValueWrapperDirect vw;

    if (!any.IsEmpty())
    {
        if (any.GetType()->IsPointer())
        {
            const ReflectedType* objectType = ReflectedTypeDB::GetByType(any.GetType()->Deref());

            if (nullptr != objectType)
            {
                ReflectedObject object(any.Get<void*>(), objectType);
                return Reflection(object, &vw, nullptr, objectMeta);
            }
        }
        else
        {
            const ReflectedType* objectType = ReflectedTypeDB::GetByType(any.GetType());

            if (nullptr != objectType)
            {
                ReflectedObject object(const_cast<void*>(any.GetData()), objectType);
                return Reflection(object, &vw, nullptr, objectMeta);
            }
        }
    }

    return Reflection();
}

Reflection Reflection::Create(const Reflection& etalon, const Reflection& metaProvider)
{
    return Reflection(etalon.object, etalon.valueWrapper, etalon.structureWrapper, metaProvider.meta);
}

Reflection::Field::Field(Any&& key_, Reflection&& ref_, const ReflectedType* inheritFrom_)
    : key(key_)
    , ref(ref_)
    , inheritFrom(inheritFrom_)
{
}

Reflection::Method::Method(FastName key_, AnyFn&& fn_)
    : key(key_)
    , fn(fn_)
{
}

} // namespace DAVA
