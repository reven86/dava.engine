#include "Base/BaseTypes.h"

namespace DAVA
{
class UIKeyboard;
class UITextField;
struct Rect;

class UIKeyboardImpl
{
public:
    UIKeyboardImpl( UIKeyboard * keyboard );
    ~UIKeyboardImpl();
    
    //void Show( UITextField * davaTextField );
    //void Hide( UITextField * davaTextField );
    void SendWillShowNotification( const Rect &kbRect );
    void SendDidShowNotification( const Rect &kbRect );
    void SendWillHideNotification();
    void SendDidHideNotification();
private:
    UIKeyboard * keyboard;
    void * implPointer;
};
    
};
