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

#include "YamlEmitter.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlNode.h"
#include "yaml/yaml.h"

namespace DAVA 
{
static const int32 INDENTATION_INCREMENT = 4;
static const int32 UNESCAPED_UNICODE_CHARACTERS_ALLOWED = 1;
static const int32 PREFERRED_LINE_WIDTH = -1;//-1 means unlimited.

static yaml_scalar_style_t GetYamlScalarStyle(YamlNode::eStringRepresentation representation)
{
    yaml_scalar_style_t style;
    switch (representation)
    {
    case YamlNode::SR_DOUBLE_QUOTED_REPRESENTATION:
        style = YAML_DOUBLE_QUOTED_SCALAR_STYLE;
        break;
    case YamlNode::SR_PLAIN_REPRESENTATION:
        style = YAML_PLAIN_SCALAR_STYLE;
        break;
    default:
        style = YAML_ANY_SCALAR_STYLE;
        break;
    }
    return style;
}

static yaml_sequence_style_t GetYamlSequenceStyle(YamlNode::eArrayRepresentation representation)
{
    yaml_sequence_style_t style;
    switch (representation)
    {
    case YamlNode::AR_BLOCK_REPRESENTATION:
        style = YAML_BLOCK_SEQUENCE_STYLE;
        break;
    case YamlNode::AR_FLOW_REPRESENTATION:
        style = YAML_FLOW_SEQUENCE_STYLE;
        break;
    default:
        style = YAML_ANY_SEQUENCE_STYLE;
        break;
    }
    return style;
}

static yaml_mapping_style_t GetYamlMappingStyle(YamlNode::eMapRepresentation representation)
{
    yaml_mapping_style_t style;
    switch (representation)
    {
    case YamlNode::MR_BLOCK_REPRESENTATION:
        style = YAML_BLOCK_MAPPING_STYLE;
        break;
    case YamlNode::MR_FLOW_REPRESENTATION:
        style = YAML_FLOW_MAPPING_STYLE;
        break;
    default:
        style = YAML_ANY_MAPPING_STYLE;
        break;
    }
    return style;
}

int write_handler(void *ext, unsigned char *buffer, size_t size)//yaml_write_handler_t
{
    File *yamlFile = static_cast<File *>(ext);

    uint32 bytesWritten = yamlFile->Write(buffer, static_cast<uint32>(size));

    return (size == bytesWritten) ? 1 : 0;
}

DAVA::YamlEmitter::~YamlEmitter()
{
}

DAVA::YamlEmitter::YamlEmitter()
{
}

bool YamlEmitter::SaveToYamlFile(const FilePath &outFileName, const YamlNode *node, uint32 attr)
{
    ScopedPtr<File> outFile(File::Create(outFileName, attr));
    if ((File*)outFile == NULL)
    {
        Logger::Error("[YamlEmitter::Emit] Can't create file: %s for output", outFileName.GetStringValue().c_str());
        return false;
    }

    return SaveToYamlFile(node, outFile);
}

bool YamlEmitter::SaveToYamlFile(const YamlNode *node, File *outfile)
{
    ScopedPtr<YamlEmitter> emitter(new YamlEmitter());
    return emitter->Emit(node, outfile);
}

bool YamlEmitter::Emit(const YamlNode * node, File *outFile)
{
    yaml_emitter_t emitter;

    DVVERIFY(yaml_emitter_initialize(&emitter));
    yaml_emitter_set_encoding(&emitter, YAML_UTF8_ENCODING);
    yaml_emitter_set_break   (&emitter, YAML_ANY_BREAK);
    yaml_emitter_set_unicode (&emitter, UNESCAPED_UNICODE_CHARACTERS_ALLOWED);
    yaml_emitter_set_width   (&emitter, PREFERRED_LINE_WIDTH);
    yaml_emitter_set_indent  (&emitter, INDENTATION_INCREMENT);
    yaml_emitter_set_output  (&emitter, &write_handler, outFile);

    do
    {
        if (!EmitStreamStart(&emitter)) break;

        if (!EmitDocumentStart(&emitter)) break;

        if (!EmitYamlNode(&emitter, node)) break;

        if (!EmitDocumentEnd(&emitter)) break;

        if (!EmitStreamEnd(&emitter)) break;
    } while (0);

    if (YAML_NO_ERROR != emitter.error)
    {
        switch (emitter.error)
        {
        case YAML_MEMORY_ERROR:
            Logger::Error("[YamlEmitter::Emit] Memory error: Not enough memory for emitting");
            break;

        case YAML_WRITER_ERROR:
            Logger::Error("[YamlEmitter::Emit] Writer error: %s", emitter.problem);
            break;

        case YAML_EMITTER_ERROR:
            Logger::Error("[YamlEmitter::Emit] Emitter error: %s", emitter.problem);
            break;

        default:
            /* Couldn't happen. */
            Logger::Error("[YamlEmitter::Emit] Internal error\n");
            break;
        }
    }

    DVVERIFY(yaml_emitter_flush(&emitter));
    yaml_emitter_delete(&emitter);

    return true;
}

bool YamlEmitter::EmitYamlNode(yaml_emitter_t * emitter, const YamlNode * node)
{
    switch (node->GetType())
    {
    case YamlNode::TYPE_STRING:
        {
            if (!EmitScalar(emitter, node->AsString(), GetYamlScalarStyle(node->GetStringRepresentation())))
                return false;
        }
        break;
    case YamlNode::TYPE_ARRAY:
        {
            if (!EmitSequenceStart(emitter, GetYamlSequenceStyle(node->GetArrayRepresentation())))
                return false;

            int32 count = node->GetCount();
            for (int32 i = 0; i < count; ++i)
            {
                if (!EmitYamlNode(emitter, node->Get(i)))
                    return false;
            }

            if (!EmitSequenceEnd(emitter))
                return false;
        }
        break;
    case YamlNode::TYPE_MAP:
        {
            if (!EmitMappingStart(emitter, GetYamlMappingStyle(node->GetMapRepresentation())))
                return false;

            bool res = node->GetMapOrderRepresentation() ? EmitOrderedMap(emitter, node) : EmitUnorderedMap(emitter, node);
            if (!res)
                return false;

            if (!EmitMappingEnd(emitter))
                return false;
        }
        break;
    }

    return true;
}

bool YamlEmitter::EmitStreamStart(yaml_emitter_t * emitter)
{
    yaml_event_t event;
    if (!yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING) ||
        !yaml_emitter_emit(emitter, &event))
        return false;

    return true;
}

bool YamlEmitter::EmitStreamEnd(yaml_emitter_t * emitter)
{
    yaml_event_t event;
    if (!yaml_stream_end_event_initialize(&event) ||
        !yaml_emitter_emit(emitter, &event))
        return false;

    return true;
}

bool YamlEmitter::EmitDocumentStart(yaml_emitter_t * emitter)
{
    yaml_event_t event;
    if (!yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 1) ||
        !yaml_emitter_emit(emitter, &event))
        return false;

    return true;
}

bool YamlEmitter::EmitDocumentEnd(yaml_emitter_t * emitter)
{
    yaml_event_t event;
    if (!yaml_document_end_event_initialize(&event, 1) ||
        !yaml_emitter_emit(emitter, &event))
        return false;

    return true;
}

bool YamlEmitter::EmitSequenceStart(yaml_emitter_t * emitter, int32 sequenceStyle)
{
    yaml_event_t event;
    if (!yaml_sequence_start_event_initialize(&event, NULL, NULL, 0, (yaml_sequence_style_t)sequenceStyle) ||
        !yaml_emitter_emit(emitter, &event))
        return false;

    return true;
}

bool YamlEmitter::EmitSequenceEnd(yaml_emitter_t * emitter)
{
    yaml_event_t event;
    if (!yaml_sequence_end_event_initialize(&event) ||
        !yaml_emitter_emit(emitter, &event))
        return false;

    return true;
}

bool YamlEmitter::EmitMappingStart(yaml_emitter_t * emitter, int32 mappingStyle)
{
    yaml_event_t event;
    if (!yaml_mapping_start_event_initialize(&event, NULL, NULL, 0, (yaml_mapping_style_t)mappingStyle) ||
        !yaml_emitter_emit(emitter, &event))
        return false;

    return true;
}

bool YamlEmitter::EmitMappingEnd(yaml_emitter_t * emitter)
{
    yaml_event_t event;
    if (!yaml_mapping_end_event_initialize(&event) ||
        !yaml_emitter_emit(emitter, &event))
        return false;

    return true;
}

bool YamlEmitter::EmitScalar(yaml_emitter_t * emitter, const String &value, int32 scalarStyle)
{
    yaml_event_t event;
    if (!yaml_scalar_event_initialize(&event, NULL, (yaml_char_t *)YAML_DEFAULT_SCALAR_TAG, (yaml_char_t *)value.c_str(), -1, 1, 1, (yaml_scalar_style_t)scalarStyle) ||
        !yaml_emitter_emit(emitter, &event))
        return false;

    return true;
}

bool YamlEmitter::EmitUnorderedMap(yaml_emitter_t * emitter, const YamlNode * mapNode)
{
    int32 count = mapNode->GetCount();
    for ( int32 i = 0; i < count; ++i)
    {
        if (!EmitScalar(emitter, mapNode->GetItemKeyName(i), GetYamlScalarStyle(mapNode->GetMapKeyRepresentation())))
            return false;
        if (!EmitYamlNode(emitter, mapNode->Get(i)))
            return false;
    }
    return true;
}

bool YamlEmitter::EmitOrderedMap( yaml_emitter_t * emitter, const YamlNode * mapNode )
{
    const MultiMap<String, YamlNode*> &map = mapNode->AsMap();
    MultiMap<String, YamlNode*>::const_iterator iter = map.begin();
    MultiMap<String, YamlNode*>::const_iterator end = map.end();
    for (; iter != end; ++iter)
    {
        if (!EmitScalar(emitter, iter->first, GetYamlScalarStyle(mapNode->GetMapKeyRepresentation())))
            return false;
        if (!EmitYamlNode(emitter, iter->second))
            return false;
    }
    return true;
}

}