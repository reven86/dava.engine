#include "ParticlesEditorNodeNameHelper.h"
#include "../StringConstants.h"

namespace DAVA
{
String ParticlesEditorNodeNameHelper::GetBaseName(const String& name)
{
    String baseName = name;
    String numberName;
    // Get numbers string at the end of entity name
    const char* cName = baseName.c_str();
    for (int i = static_cast<int>(baseName.length()) - 1; i >= 0; --i)
    {
        char a = cName[i];
        if (a >= '0' && a <= '9')
            numberName = a + numberName;
        else
            break;
    }

    baseName = baseName.substr(0, name.length() - numberName.length());

    return baseName;
}

String ParticlesEditorNodeNameHelper::GetNewNodeName(const String& name, Entity* parentNode)
{
    // Don't change name for "root" nodes
    if (String::npos != name.find(ResourceEditor::EDITOR_BASE))
        return name;

    // Keep unique node name
    if (!IsNodeNameExist(name, parentNode))
    {
        return name;
    }

    // Increase node counter until we get unique name
    int i = 0;
    while (true)
    {
        String newName = String(Format("%s%i", name.c_str(), ++i));

        if (!IsNodeNameExist(newName, parentNode))
            return newName;
    }
}

bool ParticlesEditorNodeNameHelper::IsNodeNameExist(const String& name, Entity* parentNode)
{
    if (!parentNode)
        return false;

    int32 childrenCount = parentNode->GetChildrenCount();
    FastName fastName = FastName(name);

    for (int32 i = 0; i < childrenCount; ++i)
    {
        Entity* childNode = parentNode->GetChild(i);

        if (!childNode)
            continue;

        if (childNode->GetName() == fastName)
        {
            return true;
        }
    }

    return false;
}

String ParticlesEditorNodeNameHelper::GetNewLayerName(const String& name, ParticleEmitter* emitter)
{
    // Keep unique node name
    if (!IsLayerNameExist(name, emitter))
    {
        return name;
    }

    int i = 0;
    while (true)
    {
        String newName = String(Format("%s%i", ResourceEditor::LAYER_NODE_NAME.c_str(), ++i));

        if (!IsLayerNameExist(newName, emitter))
            return newName;
    }
}

bool ParticlesEditorNodeNameHelper::IsLayerNameExist(const String& name, ParticleEmitter* emitter)
{
    if (!emitter)
        return false;

    for (std::vector<ParticleLayer*>::const_iterator t = emitter->layers.begin(); t != emitter->layers.end(); ++t)
    {
        ParticleLayer* layer = *t;

        if (!layer)
            continue;

        if (layer->layerName == name)
        {
            return true;
        }
    }

    return false;
}
};