/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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
		CREATE	= 0x1,
		OPEN	= 0x2,
		READ	= 0x4,
		WRITE	= 0x8,
		APPEND	= 0x10
	};
	
	//! File seek enumeration
	enum eFileSeek
	{
		SEEK_FROM_START		= 1, //! Seek from start of file
		SEEK_FROM_END		= 2, //! Seek from end of file
		SEEK_FROM_CURRENT	= 3, //! Seek from current file position relatively
	};

protected:
    File() = default;
    virtual ~ File();

public:
	/** 
		\brief function to create a file instance with give attributes.
        Use framework notation for paths.
		\param[in] filePath absolute or relative framework specific path to file
		\param[in] attributes combinations of eFileAttributes
		\returns file instance
	 */
	static File * Create(const FilePath &filePath, uint32 attributes);

	/** 
        \brief funciton to create a file instance with give attributes
        this function must be used for opening existing files also
        \param[in] filePath absolute system path to file
        \param[in] attributes combinations of eFileAttributes
        \returns file instance
	 */
	static File * CreateFromSystemPath(const FilePath &filePath, uint32 attributes);

    /**
        \brief funciton to create a file instance with give attributes directly without framework path management.
        \param[in] filePath absolute system path to file
        \param[in] attributes combinations of eFileAttributes
        \returns file instance
     */
    static File * PureCreate(const FilePath & filePath, uint32 attributes);
	/**
		\brief Get this file name
		\returns name of this file
	 */
	virtual	const FilePath & GetFilename();
	
	/** 
		\brief Write [dataSize] bytes to this file from [pointerToData]
		\param[in] sourceBuffer function get data from this buffer
		\param[in] dataSize size of data we want to write
		\returns number of bytes actually written
	 */
	virtual uint32 Write(const void * sourceBuffer, uint32 dataSize);

	/**
		\brief Write [sizeof(T)] bytes to this file from [value]
		\param[in] value function get data from this buffer
		\returns number of bytes actually written
	 */
    template <class T>
    uint32 Write(const T * value);

	/** 
		\brief Write string.
		write null-terminated string from current position in file.
		\param[in] string string data loaded to this variable/
        \param[in] shouldNullBeWritten indicates does it require to save null terminator.
		\return true if success otherwise false
	 */
	virtual bool WriteString(const String & string, bool shouldNullBeWritten = true);

	/**
     \brief Write string
     write string without '\0' from current position in file
     \param[in] string - string data loaded to this variable
     \return true if success otherwise false
	 */
  	virtual bool WriteNonTerminatedString(const String & string);
	
    /**
		\brief Write one line of text
		Write string and add /r/n in the end.
		\param[in] string - string to write
		\return true if success otherwise false
	 */
	virtual bool WriteLine(const String & string);
	
	/** 
		\brief Read [dataSize] bytes from this file to [pointerToData] 
		\param[in, out] destinationBuffer function write data to this pointer
		\param[in] dataSize size of data we want to read
		\return number of bytes actually read
	*/
	virtual uint32 Read(void * destinationBuffer, uint32 dataSize);

	/**
		\brief Read [sizeof(T)] bytes from this file to [value]
		\param[in, out] value function write data to this pointer
		\return number of bytes actually read
	 */
    template <class T>
    uint32 Read(T * value);

	
    
    /**
		\brief Read one line from text file to [pointerToData] buffer
		\param[in, out] destinationBuffer function write data to this buffer
		\param[in] bufferSize size of [pointerToData] buffer
		\return number of bytes actually read
	*/
    uint32 ReadLine(void * destinationBuffer, uint32 bufferSize);

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
	virtual uint32 ReadString(char8 * destinationBuffer, uint32 destinationBufferSize);
    uint32 ReadString(String & destinationString);
    
	/** 
		\brief Get current file position
	*/
    virtual uint32 GetPos() const;

    /** 
		\brief Get current file size if writing
		\brief and get real file size if file for reading
	*/
    virtual uint32 GetSize() const;

    /** 
		\brief Set current file position
		\param position position to set
		\param seekType \ref IO::eFileSeek flag to set type of positioning
		\return true if successful otherwise false.
	*/
    virtual bool Seek(int32 position, uint32 seekType);

    //! return true if end of file reached and false in another case
    virtual bool IsEof() const;

    /**
        \brief Truncate a file to a specified length
        \param size A size, that file is going to be truncated to
    */
    bool Truncate(int32 size);

    /**
        \brief Flushes file buffers to output device
        \return true on success
    */
    virtual bool Flush();

    static String GetModificationDate(const FilePath & filePathname);

private:
    // reads 1 byte from current line in the file and sets it in next char if it is not a line ending char. Returns true if read was successful.
    bool GetNextChar(uint8 *nextChar);

private:
    FILE* file = nullptr;
    uint32 size = 0;

protected:
    FilePath filename;
};
    
    
template <class T>
uint32 File::Read(T * value)
{
    return Read(value, sizeof(T));
}

template <class T>
uint32 File::Write(const T * value)
{
    return Write(value, sizeof(T));
}

};



#endif