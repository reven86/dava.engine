#ifndef __DAVAENGINE_FILE_H__
#define __DAVAENGINE_FILE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"


#if defined(__DAVAENGINE_ANDROID__)
	#include "zip/zip.h"
#endif //#if defined(__DAVAENGINE_ANDROID__)

namespace DAVA
{
class File;

/**
	\ingroup filesystem
	\brief class to work with file on disk
 */
class File : public BaseObject
{
public:
    //! File attributes enumeration
    //! Remember engine support only
    //! EFA_OPEN | EFA_READ
    //! EFA_OPEN | EFA_READ | EFA_WRITE
    //! EFA_CREATE | EFA_WRITE
    //! EFA_APPEND | EFA_WRITE

    enum eFileAttributes
    {
        CREATE = 0x1,
        OPEN = 0x2,
        READ = 0x4,
        WRITE = 0x8,
        APPEND = 0x10
    };

    //! File seek enumeration
    enum eFileSeek
    {
        SEEK_FROM_START = 1, //! Seek from start of file
        SEEK_FROM_END = 2, //! Seek from end of file
        SEEK_FROM_CURRENT = 3, //! Seek from current file position relatively
    };

protected:
    File() = default;
    virtual ~File();

public:
    /**
		\brief function to create a file instance with give attributes.
        Use framework notation for paths.
		\param[in] filePath absolute or relative framework specific path to file
		\param[in] attributes combinations of eFileAttributes
		\returns file instance
	 */
    static File* Create(const FilePath& filePath, uint32 attributes);

    /**
        \brief funciton to create a file instance with give attributes
        this function must be used for opening existing files also
        \param[in] filePath absolute system path to file
        \param[in] attributes combinations of eFileAttributes
        \returns file instance
	 */
    static File* CreateFromSystemPath(const FilePath& filePath, uint32 attributes);

    /**
        \brief funciton to create a file instance with give attributes directly without framework path management.
        \param[in] filePath absolute system path to file
        \param[in] attributes combinations of eFileAttributes
        \returns file instance
     */
    static File* PureCreate(const FilePath& filePath, uint32 attributes);
    /**
		\brief Get this file name
		\returns name of this file
	 */
    virtual const FilePath& GetFilename();

    /**
		\brief Write [dataSize] bytes to this file from [pointerToData]
		\param[in] sourceBuffer function get data from this buffer
		\param[in] dataSize size of data we want to write
		\returns number of bytes actually written
	 */
    DAVA_DEPRECATED(virtual uint32 Write(const void* sourceBuffer, uint32 dataSize));

    /**
     * \brief   Writes [dataSize] bytes to file.
     *
     * \param   sourceBuffer    Buffer for source data.
     * \param   dataSize        Size of the data.
     *
     * \return  An uint32.
     */
    virtual uint64 Write64(const void* sourceBuffer, uint64 dataSize);

    /**
		\brief Write [sizeof(T)] bytes to this file from [value]
		\param[in] value function get data from this buffer
		\returns number of bytes actually written
	 */
    template <class T>
    uint32 Write(const T* value);

    /**
		\brief Write string.
		write null-terminated string from current position in file.
		\param[in] string string data loaded to this variable/
        \param[in] shouldNullBeWritten indicates does it require to save null terminator.
		\return true if success otherwise false
	 */
    virtual bool WriteString(const String& string, bool shouldNullBeWritten = true);

    /**
     \brief Write string
     write string without '\0' from current position in file
     \param[in] string - string data loaded to this variable
     \return true if success otherwise false
	 */
    virtual bool WriteNonTerminatedString(const String& string);

    /**
		\brief Write one line of text
		Write string and add /r/n in the end.
		\param[in] string - string to write
		\return true if success otherwise false
	 */
    virtual bool WriteLine(const String& string);

    /**
		\brief Read [dataSize] bytes from this file to [pointerToData]
		\param[in, out] destinationBuffer function write data to this pointer
		\param[in] dataSize size of data we want to read
		\return number of bytes actually read
	*/
    DAVA_DEPRECATED(virtual uint32 Read(void* destinationBuffer, uint32 dataSize));

    /**
     * \brief   Reads bytes into specified buffer.
     *
     * \param [in,out]  destBuf non-null, buffer for destination data.
     * \param   dataSize        Size of the data.
     *
     * \return  An uint64.
     */
    virtual uint64 Read64(void* destBuf, uint64 dataSize);

    /**
		\brief Read [sizeof(T)] bytes from this file to [value]
		\param[in, out] value function write data to this pointer
		\return number of bytes actually read
	 */
    template <class T>
    uint32 Read(T* value);

    /**
		\brief Read one line from text file to [pointerToData] buffer
		\param[in, out] destinationBuffer function write data to this buffer
		\param[in] bufferSize size of [pointerToData] buffer
		\return number of bytes actually read
	*/
    uint32 ReadLine(void* destinationBuffer, uint32 bufferSize);

    /**
    \brief Read one line from text file without line endings
     */
    String ReadLine();

    /**
		\brief Read string line from file to destination buffer with destinationBufferSize
		\param[in, out] destinationBuffer buffer for the data
		\param[in] destinationBufferSize size of the destinationBuffer, for security reasons
		\returns actual length of the string that was read
	 */
    virtual uint32 ReadString(char8* destinationBuffer, uint32 destinationBufferSize);
    uint32 ReadString(String& destinationString);

    /**
		\brief Get current file position
	*/
    DAVA_DEPRECATED(virtual uint32 GetPos() const);

    /**
     * \brief   Gets position in file stream.
     *
     * \return  The position as uint64.
     */
    virtual uint64 GetPos64() const;

    /**
		\brief Get current file size if writing
		       and get real file size if file for reading
	*/
    DAVA_DEPRECATED(virtual uint32 GetSize() const);

    /**
     * \brief   Gets size 64.
     *
     * \return  The size 64.
     */
    virtual uint64 GetSize64() const;

    /**
		\brief Set current file position
		\param position - position to set
		\param seekType - \ref IO::eFileSeek flag to set type of positioning
		\return true if successful otherwise false.
	*/
    DAVA_DEPRECATED(virtual bool Seek(int32 position, eFileSeek seekType));

    /**
     * \brief   Seek to new position in file.
     *
     * \param   position        The new position.
     * \param   seekDirection   The seek direction.
     *
     * \return  true if it succeeds, false if it fails.
     */
    virtual bool Seek64(int64 position, eFileSeek seekDirection);

    //! return true if end of file reached and false in another case
    virtual bool IsEof() const;

    /**
        \brief Truncate a file to a specified length
        \param size A size, that file is going to be truncated to
    */
    bool Truncate(uint64 size);

    /**
        \brief Flushes file buffers to output device
        \return true on success
    */
    virtual bool Flush();

    static String GetModificationDate(const FilePath& filePathname);

protected:
    FilePath filename;

private:
    // reads 1 byte from current line in the file and sets it in next char if it is not a line ending char. Returns true if read was successful.
    bool GetNextChar(uint8* nextChar);

    FILE* file = nullptr;
    uint64 size = 0;
};

template <class T>
uint32 File::Read(T* value)
{
    return static_cast<uint32>(Read64(value, sizeof(T)));
}

template <class T>
uint32 File::Write(const T* value)
{
    return static_cast<uint32>(Write64(value, sizeof(T)));
}
};



#endif