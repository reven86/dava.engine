#ifndef __DAVA_Reflection_Definition__
#define __DAVA_Reflection_Definition__

#include <sstream>
#include "Base/Any.h"
#include "Base/AnyFn.h"
#include "Base/Type.h"

#include "Reflection/ReflectedBase.h"
#include "Reflection/ReflectedObject.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
class ValueWrapper;
class StructureWrapper;

/// \brief  Reflection allows to inspect and modify objects at runtime. It is some kind of runtime object reflection
///         with predefined methods, that are giving generic access to the real runtime object value or his fields.
///         Complex types (user type with number of fields) of the object that are going to be reflected should
///         registered with ReflectionRegistrator class.
/// \sa ReflectionRegistrator
/// \sa ReflectedBase
/// \sa ReflectedObject
/// \sa ReflectedType
/// \sa ReflectedMeta
class Reflection final
{
public:
    struct Field;
    struct Method;

    /// \brief Default constructor.
    Reflection() = default;

    /// \brief Constructor.
    /// \param  object  ReflectedObject, that is wrapping pointer on runtime object.
    /// \param  vw      ValueWrapper, that gives direct access to the runtime object value.
    /// \param  rtype   ReflectedType, that gives access to the registered  runtime object structure.
    /// \param  meta    Additional meta info.
    Reflection(const ReflectedObject& object, const ValueWrapper* vw, const ReflectedType* rtype, const ReflectedMeta* meta);

    /// \brief Query if reflection is valid.
    /// \return true if valid, false if not.
    bool IsValid() const;

    /// \brief Query if reflection is readonly.
    /// \return true if readonly, false if not.
    bool IsReadonly() const;

    /// \brief Gets reflection value type.
    /// \return null if it fails, else the value type.
    const Type* GetValueType() const;

    /// \brief Gets reflection object.
    /// \return The reflection object.
    ReflectedObject GetValueObject() const;
    const ReflectedType* GetReflectedType() const;

    /// \brief Gets reflection value.
    /// \return Reflection value.
    Any GetValue() const;

    /// \brief Sets reflection value.
    /// \param  value   Value to set.
    /// \return true if it succeeds, false if it fails (readonly).
    bool SetValue(const Any& value) const;

    /// \brief Query if reflection has fields.
    /// \return true if has, false if not.
    bool HasFields() const;

    /// \brief Gets a reflection field.
    /// \param  name    The name of the field to get.
    /// \return The field. If field with specified name isn't found empty field will be returned.
    Field GetField(const Any& name) const;

    /// \brief Gets all reflection fields.
    /// \return All fields.
    Vector<Field> GetFields() const;

    /// \brief Determine if fields can be added to reflection.
    /// \return true if fields can be added, false if not.
    bool CanAddFields() const;

    /// \brief Determine if fields can be inserted info reflection.
    /// \return true if fields can be inserted, false if not.
    bool CanInsertFields() const;

    /// \brief Determine if fields can be removed from reflection.
    /// \return true if fields can be removed, false if not.
    bool CanRemoveFields() const;

    /// \brief  Determine if there is ability to create detached field value.
    /// \return true if we fields can be added, false if not.
    /// \sa Reflection::CreateFieldValue
    bool CanCreateFieldValue() const;

    /// \brief  Creates detached value, that can be added or inserted as a new reflection field. One should do this with
    ///         Reflection::AddField or Reflection::InsertField calls.
    /// \return The new field value.
    /// \sa Reflection::AddField.
    /// \sa Reflection::InsertField.
    Any CreateFieldValue() const;

    /// \brief Adds a new field to reflection.
    /// \param  key     Field key. In some cases it can be ignored, e.g. when reflection is from std::list<T>
    /// \param  value   Field value.
    /// \return true if it succeeds, false if it fails.
    bool AddField(const Any& key, const Any& value) const;

    /// \brief Inserts a new field into reflection.
    /// \param  beforeKey   Key of the field, before which insertion will be performed.
    /// \param  key         Field key. In some cases it can be ignored, e.g. when reflection is from std::list<T>
    /// \param  value       Field value.
    /// \return true if it succeeds, false if it fails.
    bool InsertField(const Any& beforeKey, const Any& key, const Any& value) const;

    /// \brief Removes field with specified key from reflection.
    /// \param  key Field key, that should be removed.
    /// \return true if it succeeds, false if it fails.
    bool RemoveField(const Any& key) const;

    /// \brief Query if reflection has callable methods.
    /// \return true if has methods, false if not.
    bool HasMethods() const;

    /// \brief  Gets a reflection method. Returned Method is binded to the current reflection, so it will be called over
    ///         runtime object that is carried by current Reflection instance.
    /// \param  key Name of the method to get.
    /// \return Method that is binded to current reflection and can be called.
    /// \sa AnyFn
    Method GetMethod(const String& key) const;

    /// \brief Gets all reflection methods.
    /// \return All methods.
    Vector<Method> GetMethods() const;

    /// \brief Query if this reflection has meta with specified type Meta.
    /// \return true if there is meta, false if not.
    template <typename Meta>
    bool HasMeta() const;

    /// \brief Gets the meta with specified type Meta.
    /// \return null if it fails, else the meta.
    template <typename Meta>
    const Meta* GetMeta() const;

    /// \brief Dumps reflection fields and its values, including field of the fields and so on.
    /// \param [out]    out     Output stream.
    /// \param          deep    (Optional) The max deep. If 0 deep is unlimited.
    void Dump(std::ostream& out, size_t deep = 0) const;

    /// \brief Dumps reflection methods and its invoke params.
    /// \param [in,out] out The out.
    void DumpMethods(std::ostream& out) const;

    /// \brief Creates a new Reflection from given runtime object.
    /// \param [in] ptr     Pointer on runtime object to reflect.
    /// \param      meta    (Optional) The meta.
    /// \return A Reflection.
    template <typename T>
    static Reflection Create(T* ptr, const ReflectedMeta* meta = nullptr);

private:
    const ValueWrapper* vw = nullptr;
    const StructureWrapper* sw = nullptr;
    const ReflectedMeta* meta = nullptr;
    const ReflectedType* objectType = nullptr;

    ReflectedObject object;
};

/// \brief A reflection field.
struct Reflection::Field
{
    Any key; ///< field key (usually name or index)
    Reflection ref; ///< field reflection

    template <typename T>
    static Reflection::Field Create(const Any& key, T* ptr, const ReflectedMeta* meta = nullptr);
};

/// \brief A reflection method.
struct Reflection::Method
{
    String key; ///< method key (usually its name)
    AnyFn fn; ///< method itself with binded runtime object it belongs to
};

} // namespace DAVA

#endif // __DAVA_Reflection_Definition__

#ifndef __DAVA_Reflection_Definition_Only__
#define __DAVA_Reflection__
#include "Reflection/Wrappers.h"
#include "Reflection/ReflectedType.h"
#include "Reflection/Private/Reflection_impl.h"
#include "Reflection/Private/StructureWrapperClass.h"
#endif
