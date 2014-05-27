#include "Base/BaseTypes.h"

namespace DAVA
{
class UISystemKeyboard;
class UITextField;
struct Rect;

class UISystemKeyboardImpl
{
public:
    UISystemKeyboardImpl( UISystemKeyboard * keyboard );
    ~UISystemKeyboardImpl();
    
    //void Show( UITextField * davaTextField );
    //void Hide( UITextField * davaTextField );
    void SendWillShowNotification( const Rect &kbRect );
    void SendDidShowNotification( const Rect &kbRect );
    void SendWillHideNotification();
    void SendDidHideNotification();
private:
    UISystemKeyboard * keyboard;
    void * implPointer;
};
    
};
