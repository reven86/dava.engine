#pragma once

#include "Base/BaseTypes.h"
#include "Base/Vector.h"
#include "FileSystem/XMLParser.h"
#include "Utils/BiDiHelper.h"

namespace DAVA
{
struct RichLink;
class UIControl;

class XMLRichContentBuilder final : public XMLParserDelegate
{
public:
    /** Constructor with specified RichLink pointer and editor mode flag. */
    XMLRichContentBuilder(RichLink* link_, bool editorMode = false);

    /** Parse specified text and build list of controls. */
    bool Build(const String& text);

    /** Return generated controls. */
    const Vector<RefPtr<UIControl>>& GetControls() const;

    // XMLParserDelegate interface implementation
    void OnElementStarted(const String& elementName, const String& namespaceURI, const String& qualifedName, const Map<String, String>& attributes) override;
    void OnElementEnded(const String& elementName, const String& namespaceURI, const String& qualifedName) override;
    void OnFoundCharacters(const String& chars) override;

private:
    /** Store class in stack. */
    void PutClass(const String& clazz);
    /** Remove top class from stack. */
    void PopClass();
    /** Return top class from stack. */
    const String& GetClass() const;

    /** Setup base parameters in specified control. */
    void PrepareControl(UIControl* ctrl, bool autosize);
    /** Process open tag. */
    void ProcessTagBegin(const String& tag, const Map<String, String>& attributes);
    /** Process close tag. */
    void ProcessTagEnd(const String& tag);
    /** Process text content. */
    void ProcessText(const String& text);

private:
    bool needLineBreak = false;
    bool isEditorMode = false;
    bool classesInheritance = false;
    BiDiHelper::Direction direction = BiDiHelper::Direction::NEUTRAL;
    String defaultClasses;
    Vector<String> classesStack;
    Vector<RefPtr<UIControl>> controls;
    BiDiHelper bidiHelper;
    RichLink* link = nullptr;
};
}