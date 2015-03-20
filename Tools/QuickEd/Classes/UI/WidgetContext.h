#ifndef __QUICKED_WIDGET_CONTEXT_H__
#define __QUICKED_WIDGET_CONTEXT_H__

#include <type_traits>
#include <typeinfo>
#include <QtCore>

class WidgetContext : public QObject
{
    Q_OBJECT
public:
    WidgetContext(QObject *parent = nullptr);
    template<typename T>
    T GetData(const QString &role);

    template<typename T>
    void SetData(const T value, const QString &role);
signals:
    void DataChanged(const QString &role);
private:
    template<typename T>
    typename std::enable_if_t<std::is_pointer<T>::value>
        Clear(T value);

    template<typename T>
    typename std::enable_if_t<!std::is_pointer<T>::value>
        Clear(T value);

    template<typename T>
    typename std::enable_if_t<std::is_pointer<T>::value, T>
        GetDataInternal(const QString &role);

    template<typename T>
    typename std::enable_if_t<!std::is_pointer<T>::value, T>
        GetDataInternal(const QString &role);

};

template<typename T>
T WidgetContext::GetData(const QString &role)
{
    return GetDataInternal<T>(role);
}

template<typename T>
inline void WidgetContext::SetData(const T value, const QString &role)
{
    const char *roleStr = role.toUtf8().data();
    std::conditional_t<std::is_pointer<T>::value, void*, T> prevValue = property(roleStr).value<std::conditional_t<std::is_pointer<T>::value, void*, T> >();
    if (prevValue == value)
    {
        return;
    }
    Clear(prevValue);

    setProperty(roleStr, QVariant::fromValue<std::conditional_t<std::is_pointer<T>::value, void*, T> >(value));

    emit DataChanged(role);
}

template<typename T>
typename std::enable_if_t<std::is_pointer<T>::value>
WidgetContext::Clear(T value)
{
    delete value;
}

template<typename T>
typename std::enable_if_t<!std::is_pointer<T>::value>
WidgetContext::Clear(T value)
{
}

template<typename T>
typename std::enable_if_t<std::is_pointer<T>::value, T>
WidgetContext::GetDataInternal(const QString &role)
{
    QVariant var = property(role.toUtf8().data());
    if (var.canConvert<void*>())
    {
        return reinterpret_cast<T>(var.value<void*>());
    }
    else
    {
        Q_ASSERT("requested property are invalid or empty");
        return nullptr;
    }
}

template<typename T>
typename std::enable_if_t<!std::is_pointer<T>::value, T>
WidgetContext::GetDataInternal(const QString &role)
{
    QVariant var = property(role.toUtf8().data());
    if (var.canConvert<T>())
    {
        return var.value<T>();
    }
    else
    {
        Q_ASSERT("requested property are invalid or empty");
        return T();
    }
}

Q_DECLARE_METATYPE(WidgetContext*);

#endif // __QUICKED_WIDGET_CONTEXT_H__
