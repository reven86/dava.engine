#ifndef __REQUEST_H__
#define __REQUEST_H__

class Request
{
public:
    Request();

    void Accept();
    void Cancel();

    bool IsAccepted() const;

private:
    bool isAccepted;
};

#endif // __REQUEST_H__
