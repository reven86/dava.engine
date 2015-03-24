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
    T GetData(const QByteArray &role);

    template<typename T>
    void SetData(const T value, const QByteArray &role);
signals:
    void DataChanged(const QByteArray &role);
private:
    template<typename T>
    typename std::enable_if_t<std::is_pointer<T>::value>
        Clear(T value);

    template<typename T>
    typename std::enable_if_t<!std::is_pointer<T>::value>
        Clear(T value);

    template<typename T>
    typename std::enable_if_t<std::is_pointer<T>::value, T>
        GetDataInternal(const QByteArray &role);

    template<typename T>
    typename std::enable_if_t<!std::is_pointer<T>::value, T>
        GetDataInternal(const QByteArray &role);
public:
    using ValueMap = QMap < QByteArray, QVariant >;
    ValueMap values;

};

template<typename T>
T WidgetContext::GetData(const QByteArray &role)
{
    return GetDataInternal<T>(role);
}

template<typename T>
inline void WidgetContext::SetData(const T value, const QByteArray &role)
{
    ValueMap::ConstIterator it = values.constFind(role);
    if (it != values.constEnd())
    {
        std::conditional_t<std::is_pointer<T>::value, void*, T> prevValue = it.value().value<std::conditional_t<std::is_pointer<T>::value, void*, T> >();
        if (prevValue == value)
        {
            return;
        }
        values[role] = QVariant::fromValue<std::conditional_t<std::is_pointer<T>::value, void*, T> >(value);
        emit DataChanged(role);
        Clear(prevValue);
    }
    else
    {
        values[role] = QVariant::fromValue<std::conditional_t<std::is_pointer<T>::value, void*, T> >(value);
        emit DataChanged(role);
    }
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
WidgetContext::GetDataInternal(const QByteArray &role)
{
    ValueMap::ConstIterator it = values.constFind(role);
    if (it == values.constEnd())
    {
        Q_ASSERT("requested property are invalid or empty");
        return nullptr;
    }
    if (it.value().canConvert<void*>())
    {
        return reinterpret_cast<T>(it.value().value<void*>());
    }
    else
    {
        Q_ASSERT("can not convert data to pointer");
        return nullptr;
    }
}

template<typename T>
typename std::enable_if_t<!std::is_pointer<T>::value, T>
WidgetContext::GetDataInternal(const QByteArray &role)
{
    ValueMap::ConstIterator it = values.constFind(role);
    if (it == values.constEnd())
    {
        Q_ASSERT("requested property are invalid or empty");
        return T();
    }
    if (it.value().canConvert<T>())
    {
        return it.value().value<T>();
    }
    else
    {
        Q_ASSERT("requested property are invalid or empty");
        return T();
    }
}

Q_DECLARE_METATYPE(WidgetContext*);

#endif // __QUICKED_WIDGET_CONTEXT_H__
