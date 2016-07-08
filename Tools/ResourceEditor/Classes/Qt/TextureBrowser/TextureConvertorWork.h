#ifndef __TEXTURE_CONVERTOR_WORK_H__
#define __TEXTURE_CONVERTOR_WORK_H__

#include "DAVAEngine.h"
#include "Render/TextureDescriptor.h"
#include "TextureConvertMode.h"

struct JobItem
{
    int id;
    int type;
    eTextureConvertMode convertMode;
    const DAVA::TextureDescriptor* descriptor;

    JobItem()
        : id(0)
        , type(0)
        , convertMode(CONVERT_NOT_EXISTENT)
        , descriptor(NULL)
    {
    }
};

class JobStack
{
public:
    JobStack();
    ~JobStack();

    bool push(const JobItem& item);
    JobItem* pop();
    int size();

private:
    struct JobItemWrapper : public JobItem
    {
        JobItemWrapper(const JobItem& item);

        JobItemWrapper* next;
        JobItemWrapper* prev;
    };

    JobItemWrapper* head;

    int itemsCount;
};

#endif // __TEXTURE_CONVERTOR_WORK_H__
