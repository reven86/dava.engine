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


#ifndef __DAVAENGINE_YAML_EMITTER_H__
#define __DAVAENGINE_YAML_EMITTER_H__
#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "FileSystem/File.h"

typedef struct yaml_emitter_s yaml_emitter_t;

namespace DAVA
{
class YamlNode;
    /** 
     \ingroup yaml
     \brief this class is yaml saver and it used if you want to save data to yaml file
     */
class YamlEmitter: public BaseObject
{
    virtual ~YamlEmitter();
    YamlEmitter();
public:
    /**
     \brief Store content of node to file
     \returns true if success.
     */
    static bool SaveToYamlFile(const FilePath &outFileName, const YamlNode *node, uint32 attr = File::CREATE | File::WRITE);
    static bool SaveToYamlFile(const YamlNode *node, File *outfile);
protected:
    bool Emit(const YamlNode * node, File *outFile);

private:
    bool EmitStreamStart  (yaml_emitter_t * emitter);
    bool EmitStreamEnd    (yaml_emitter_t * emitter);
    bool EmitDocumentStart(yaml_emitter_t * emitter);
    bool EmitDocumentEnd  (yaml_emitter_t * emitter);
    bool EmitSequenceStart(yaml_emitter_t * emitter, int32 sequenceStyle/*yaml_sequence_style_t*/);
    bool EmitSequenceEnd  (yaml_emitter_t * emitter);
    bool EmitMappingStart (yaml_emitter_t * emitter, int32 mappingStyle/*yaml_mapping_style_t*/);
    bool EmitMappingEnd   (yaml_emitter_t * emitter);
    bool EmitScalar       (yaml_emitter_t * emitter, const String &value, int32 scalarStyle/*yaml_scalar_style_t*/);
    bool EmitYamlNode     (yaml_emitter_t * emitter, const YamlNode * node);
    bool EmitUnorderedMap (yaml_emitter_t * emitter, const YamlNode * mapNode);
    bool EmitOrderedMap   (yaml_emitter_t * emitter, const YamlNode * mapNode);
};

};
#endif // __DAVAENGINE_YAML_EMITTER_H__