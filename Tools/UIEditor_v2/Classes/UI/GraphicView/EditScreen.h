//
//  EditScreen.h
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/11/14.
//
//

#ifndef __UIEditor_EditScreen_h__
#define __UIEditor_EditScreen_h__

#include "DAVAEngine.h"

class CheckeredCanvas: public DAVA::UIControl
{
public:
    CheckeredCanvas();
private:
    virtual ~CheckeredCanvas();
    
    virtual void Draw(const DAVA::UIGeometricData &geometricData);
    virtual void DrawAfterChilds(const DAVA::UIGeometricData &geometricData);
private:
};

class PackageCanvas: public DAVA::UIControl
{
public:
    PackageCanvas();
private:
    virtual ~PackageCanvas();
public:
    void LayoutCanvas();
private:
};

#endif // __UIEditor_EditScreen_h__
