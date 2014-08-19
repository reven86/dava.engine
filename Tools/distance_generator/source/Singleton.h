#ifndef __DISTANCE_GENERATOR__SINGLETON__
#define __DISTANCE_GENERATOR__SINGLETON__

template <typename T>
class Singleton
{
public:
    Singleton()
    {
        if (instance == 0)
        {
            instance = (T*)this;
        }
    }

    virtual ~Singleton()
    {
        instance = 0;
    }

    static T* Instance() { return instance; }

    void Release()
    {
        if (this)
        {
            delete this;
            instance = 0;
        }
        else
        {
        }

    return;
    }

private:
    static T* instance;
};

template <typename T>
T* Singleton<T>::instance = 0;

#endif /* defined(__DISTANCE_GENERATOR__SINGLETON__) */
