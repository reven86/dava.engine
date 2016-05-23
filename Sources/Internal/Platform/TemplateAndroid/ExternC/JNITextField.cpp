#include "AndroidLayer.h"
#include "UI/UITextFieldAndroid.h"
#include "Base/BaseTypes.h"
#include "Utils/UTF8Utils.h"
#include "Render/Image/ImageConvert.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

extern "C"
{
void Java_com_dava_framework_JNITextField_TextFieldShouldReturn(JNIEnv* env, jobject classthis, uint32_t id)
{
    DAVA::TextFieldPlatformImpl::TextFieldShouldReturn(id);
}

jbyteArray Java_com_dava_framework_JNITextField_TextFieldKeyPressed(JNIEnv* env, jobject classthis, uint32_t id, int replacementLocation, int replacementLength, jbyteArray replacementString)
{
    DAVA::WideString string;

    jbyte* bufferPtr = env->GetByteArrayElements(replacementString, NULL);
    jsize lengthOfArray = env->GetArrayLength(replacementString);

    DAVA::UTF8Utils::EncodeToWideString((uint8_t*)bufferPtr, lengthOfArray, string);

    env->ReleaseByteArrayElements(replacementString, bufferPtr, 0);

    bool res = DAVA::TextFieldPlatformImpl::TextFieldKeyPressed(id, replacementLocation, replacementLength, string);
    DAVA::String returnStr = res ? DAVA::UTF8Utils::EncodeToUTF8(string) : "";

    jbyteArray r = env->NewByteArray(returnStr.length());
    if (r == NULL)
        return NULL;
    env->SetByteArrayRegion(r, 0, returnStr.length(), (const jbyte*)returnStr.c_str());
    return r;
}

void Java_com_dava_framework_JNITextField_TextFieldOnTextChanged(JNIEnv* env, jobject classthis, uint32_t id, jbyteArray newText, jbyteArray oldText)
{
    DAVA::WideString newString, oldString;

    jbyte* bufferPtr = env->GetByteArrayElements(newText, NULL);
    jsize lengthOfArray = env->GetArrayLength(newText);
    DAVA::UTF8Utils::EncodeToWideString((uint8_t*)bufferPtr, lengthOfArray, newString);
    env->ReleaseByteArrayElements(newText, bufferPtr, 0);

    bufferPtr = env->GetByteArrayElements(oldText, NULL);
    lengthOfArray = env->GetArrayLength(oldText);
    DAVA::UTF8Utils::EncodeToWideString((uint8_t*)bufferPtr, lengthOfArray, oldString);
    env->ReleaseByteArrayElements(oldText, bufferPtr, 0);

    DAVA::TextFieldPlatformImpl::TextFieldOnTextChanged(id, newString, oldString);
}

void Java_com_dava_framework_JNITextField_TextFieldKeyboardShown(JNIEnv* env, jobject classthis, uint32_t id, int x, int y, int dx, int dy)
{
    // Recalculate to virtual coordinates.
    DAVA::Vector2 keyboardOrigin(x, y);
    keyboardOrigin = DAVA::VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(keyboardOrigin);

    DAVA::Vector2 keyboardSize(dx, dy);
    keyboardSize = DAVA::VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(keyboardSize);

    DAVA::TextFieldPlatformImpl::TextFieldKeyboardShown(id, DAVA::Rect(keyboardOrigin, keyboardSize));
}

void Java_com_dava_framework_JNITextField_TextFieldKeyboardHidden(JNIEnv* env, jobject classthis, uint32_t id)
{
    DAVA::TextFieldPlatformImpl::TextFieldKeyboardHidden(id);
}

void Java_com_dava_framework_JNITextField_TextFieldFocusChanged(JNIEnv* env, jobject classthis, uint32_t id, bool hasFocus)
{
    DAVA::TextFieldPlatformImpl::TextFieldFocusChanged(id, hasFocus);
}

void Java_com_dava_framework_JNITextField_TextFieldUpdateTexture(JNIEnv* env,
                                                                 jobject classthis, uint32_t id, jintArray pixels, int width, int height)
{
    static_assert(sizeof(jint) == sizeof(DAVA::int32), "o_O can't be");

    if (nullptr != pixels)
    {
        DVASSERT(width > 0);
        DVASSERT(height > 0);

        jboolean isCopy{ 0 };
        jint* rawData = env->GetIntArrayElements(pixels, &isCopy);

        DVASSERT(rawData != nullptr);
        DVASSERT(env->GetArrayLength(pixels) == width * height); // ARGB

        DAVA::int32* pixelsCopy{ nullptr };

        if (JNI_TRUE == isCopy)
        {
            pixelsCopy = reinterpret_cast<DAVA::int32*>(rawData);
            DAVA::TextFieldPlatformImpl::TextFieldUpdateTexture(id, pixelsCopy, width, height);
        }
        else
        {
            // we have to copy pixels from Java because different threads (Java, OpenGL)
            // and in Java main thread current pixel buffer can be rewritten
            const DAVA::uint8* data = reinterpret_cast<DAVA::uint8*>(rawData);
            DAVA::Image* image = DAVA::Image::CreateFromData(width, height, DAVA::FORMAT_RGBA8888, data);
            SCOPE_EXIT
            {
                SafeRelease(image);
            };

            pixelsCopy = reinterpret_cast<DAVA::int32*>(image->GetData());
            DAVA::TextFieldPlatformImpl::TextFieldUpdateTexture(id, pixelsCopy, width, height);
        }
        // JNI_ABORT free the buffer without copying back the possible changes
        env->ReleaseIntArrayElements(pixels, rawData, JNI_ABORT);
    }
    else
    {
        DAVA::TextFieldPlatformImpl::TextFieldUpdateTexture(id, nullptr, width, height);
    }
}
};
