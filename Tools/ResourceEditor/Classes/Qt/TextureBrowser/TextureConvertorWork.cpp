#include "TextureBrowser/TextureConvertorWork.h"

JobStack::JobStack()
    : head(NULL)
    , itemsCount(0)
{
}

JobStack::~JobStack()
{
    JobItemWrapper* item;

    while (NULL != head)
    {
        item = head;
        head = head->next;

        delete item;
    }
}

bool JobStack::push(const JobItem& item)
{
    bool ret = true;
    JobItemWrapper* i = head;

    // remember force value
    eTextureConvertMode convertMode = item.convertMode;

    // search for the same works in list and remove it
    while (NULL != i)
    {
        if (i->type == item.type && NULL != i->descriptor && i->descriptor == item.descriptor)
        {
            if (NULL != i->prev)
            {
                i->prev->next = i->next;
            }

            if (NULL != i->next)
            {
                i->next->prev = i->prev;
            }

            if (i == head)
            {
                head = i->next;
            }

            // if this job has more strict convert mode, we should move it to the new job
            if (i->convertMode < convertMode)
            {
                convertMode = i->convertMode;
            }

            delete i;
            itemsCount--;

            ret = false;
            break;
        }

        i = i->next;
    }

    // add new work
    i = new JobItemWrapper(item);

    // restore convert mode value
    i->convertMode = convertMode;

    if (NULL != head)
    {
        head->prev = i;
        i->next = head;
    }

    head = i;
    itemsCount++;

    return ret;
}

JobItem* JobStack::pop()
{
    JobItemWrapper* item = head;

    if (NULL != head)
    {
        head = head->next;
        if (NULL != head)
        {
            head->prev = NULL;
        }

        itemsCount--;
    }

    return item;
}

int JobStack::size()
{
    return itemsCount;
}

JobStack::JobItemWrapper::JobItemWrapper(const JobItem& item)
    : JobItem(item)
    , next(NULL)
    , prev(NULL)
{
}
