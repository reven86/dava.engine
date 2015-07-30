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

#include "Render/Renderer.h"
#include "Render/RenderHelper.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Utils/Utils.h"
#include "Render/DynamicBufferAllocator.h"

const DAVA::float32 CIRCLE_SEGMENT_LENGTH = 15.0f;

const DAVA::float32 ISO_X = 0.525731f;
const DAVA::float32 ISO_Z = 0.850650f;

std::array<DAVA::Vector3, 12> gIcosaVertexes = {
    DAVA::Vector3(-ISO_X,    0.0,  ISO_Z),
    DAVA::Vector3( ISO_X,    0.0,  ISO_Z),
    DAVA::Vector3(-ISO_X,    0.0, -ISO_Z),
    DAVA::Vector3( ISO_X,    0.0, -ISO_Z),
    DAVA::Vector3(   0.0,  ISO_Z,  ISO_X),
    DAVA::Vector3(   0.0,  ISO_Z, -ISO_X),
    DAVA::Vector3(   0.0, -ISO_Z,  ISO_X),
    DAVA::Vector3(   0.0, -ISO_Z, -ISO_X),
    DAVA::Vector3( ISO_Z,  ISO_X,    0.0),
    DAVA::Vector3(-ISO_Z,  ISO_X,    0.0),
    DAVA::Vector3( ISO_Z, -ISO_X,    0.0),
    DAVA::Vector3(-ISO_Z, -ISO_X,    0.0)
};

std::array<DAVA::uint16, 60> gWireIcosaIndexes = {
    0, 1,   1, 4,   4, 8,   8,  5,   5,  3,   3, 2,   2, 7,   7, 11,  11, 6,   6, 0, 
    0, 4,   4, 5,   5, 2,   2, 11,  11,  0,   1, 8,   8, 3,   3,  7,   7, 6,   6, 1,
    9, 0,   9, 4,   9, 5,   9,  2,   9, 11,  10, 1,  10, 8,  10,  3,  10, 7,  10, 6,
};

std::array<DAVA::uint16, 60> gSolidIcosaIndexes = {
    0,  4,  1,    0,  9,  4,    9,  5,  4,    4,  5,  8,    4,  8,  1,
    8, 10,  1,    8,  3, 10,    5,  3,  8,    5,  2,  3,    2,  7,  3,
    7, 10,  3,    7,  6, 10,    7, 11,  6,   11,  0,  6,    0,  1,  6,
    6,  1, 10,    9,  0, 11,    9, 11,  2,    9,  2,  5,    7,  2, 11,
};

std::array<DAVA::uint16, 24> gWireBoxIndexes = {
    0, 2,   2, 6,   6, 3,   3, 0,
    1, 4,   4, 7,   7, 5,   5, 1,
    0, 1,   2, 4,   6, 7,   3, 5,
};

std::array<DAVA::uint16, 36> gSolidBoxIndexes = {
    0, 6, 3,   0, 2, 6,   4, 5, 7,   4, 1, 5,
    2, 7, 6,   2, 4, 7,   1, 3, 5,   1, 0, 3,
    3, 7, 5,   3, 6, 7,   1, 2, 0,   1, 4, 2,
};

std::array<DAVA::uint16, 16> gWireArrowIndexes = {
    0, 1,   0, 2,   0, 3,   0, 4,
    1, 2,   2, 3,   3, 4,   4, 1,
};

std::array<DAVA::uint16, 18> gSolidArrowIndexes = {
    0, 1, 2,   0, 2, 3,   0, 3, 4,   
    0, 4, 1,   1, 4, 2,   2, 4, 3,
};

namespace DAVA
{

    RenderHelper::RenderHelper()
    {
        rhi::VertexLayout layout;
        layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
        layout.AddElement(rhi::VS_COLOR, 0, rhi::VDT_UINT8N, 4);
        coloredVertexLayoutUID = rhi::VertexLayout::UniqueId(layout);

        for (NMaterial * & material : materials)
            material = new NMaterial();

        materials[DRAW_WIRE_DEPTH]->SetFXName(NMaterialName::VERTEXCOLOR_OPAQUE);
        materials[DRAW_SOLID_DEPTH]->SetFXName(NMaterialName::VERTEXCOLOR_ALPHABLEND);
        materials[DRAW_WIRE_NO_DEPTH]->SetFXName(NMaterialName::VERTEXCOLOR_OPAQUE_NODEPTHTEST);
        materials[DRAW_SOLID_NO_DEPTH]->SetFXName(NMaterialName::VERTEXCOLOR_ALPHABLEND_NODEPTHTEST);

        for (NMaterial * material : materials)
            material->PreBuildMaterial(PASS_FORWARD);
    }

    RenderHelper::~RenderHelper()
    {
        for (int32 i = 0; i < DRAW_TYPE_COUNT; ++i)
            SafeRelease(materials[i]);
    }

    void RenderHelper::PreparePacket(rhi::Packet & packet, NMaterial * material, const std::pair<uint32, uint32> & buffersCount, ColoredVertex ** vBufferDataPtr, uint16 ** iBufferDataPtr)
    {
        material->BindParams(packet);
        packet.vertexStreamCount = 1;
        packet.vertexLayoutUID = coloredVertexLayoutUID;

        if (buffersCount.first)
        {
            DynamicBufferAllocator::AllocResultVB vb = DynamicBufferAllocator::AllocateVertexBuffer(sizeof(ColoredVertex), buffersCount.first);
            *vBufferDataPtr = reinterpret_cast<ColoredVertex *>(vb.data);
            packet.vertexStream[0] = vb.buffer;
            packet.vertexCount = vb.allocatedVertices;
            packet.baseVertex = vb.baseVertex;
        }
        if (buffersCount.second)
        {
            DynamicBufferAllocator::AllocResultIB ib = DynamicBufferAllocator::AllocateIndexBuffer(buffersCount.second);
            *iBufferDataPtr = ib.data;
            packet.indexBuffer = ib.buffer;
            packet.startIndex = ib.baseIndex;
        }
    }

    void RenderHelper::Present(rhi::HPacketList packetList, const Matrix4 * viewMatrix, const Matrix4 * projectionMatrix)
    {
        if (!commandQueue.size())
            return;

        rhi::Packet packet[DRAW_TYPE_COUNT];
        ColoredVertex *vBufferPtr[DRAW_TYPE_COUNT] = {};
        uint16 * iBufferPtr[DRAW_TYPE_COUNT] = {};
        uint32 vBufferOffset[DRAW_TYPE_COUNT] = {};

        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEW, viewMatrix, (pointer_size)viewMatrix);
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_PROJ, projectionMatrix, (pointer_size)projectionMatrix);

        for (int32 i = 0; i < DRAW_TYPE_COUNT; ++i)
            PreparePacket(packet[i], materials[i], buffersElemCount[i], &vBufferPtr[i], &iBufferPtr[i]);

        packet[DRAW_WIRE_DEPTH].primitiveType = packet[DRAW_WIRE_NO_DEPTH].primitiveType = rhi::PRIMITIVE_LINELIST;
        packet[DRAW_SOLID_DEPTH].primitiveType = packet[DRAW_SOLID_NO_DEPTH].primitiveType = rhi::PRIMITIVE_TRIANGLELIST;

        for (const DrawCommand & command : commandQueue)
        {
            ColoredVertex * & commandVBufferPtr = vBufferPtr[command.drawType];
            uint16 * & commandIBufferPtr = iBufferPtr[command.drawType];
            rhi::Packet & commandPacket = packet[command.drawType];
            uint32 & commandVBufferOffset = vBufferOffset[command.drawType];

            switch (command.id)
            {
                case COMMAND_DRAW_LINE:
                {
                    uint32 lineColor = rhi::NativeColorRGBA(command.params[6], command.params[7], command.params[8], command.params[9]);

                    commandVBufferPtr->position = Vector3(command.params[0], command.params[1], command.params[2]);
                    commandVBufferPtr->color = lineColor;
                    ++commandVBufferPtr;

                    commandVBufferPtr->position = Vector3(command.params[3], command.params[4], command.params[5]);
                    commandVBufferPtr->color = lineColor;
                    ++commandVBufferPtr;

                    *commandIBufferPtr = commandVBufferOffset;        ++commandIBufferPtr;
                    *commandIBufferPtr = commandVBufferOffset + 1;    ++commandIBufferPtr;

                    ++commandPacket.primitiveCount;
                    commandVBufferOffset += 2;
                } break;

                case COMMAND_DRAW_POLYGON:
                {
                    const uint32 primitiveColor = rhi::NativeColorRGBA(command.params[0], command.params[1], command.params[2], command.params[3]);
                    const uint32 pointCount = (command.params.size() - 4) / 3;

                    const Vector3 * const polygonPoints = reinterpret_cast<const Vector3 *>(&command.params[4]);
                    for (uint32 i = 0; i < pointCount; ++i)
                    {
                        (*commandVBufferPtr).position = polygonPoints[i];
                        (*commandVBufferPtr).color = primitiveColor;
                        ++commandVBufferPtr;
                    }

                    if (command.drawType & FLAG_DRAW_SOLID)
                    {
                        const uint32 triangleCount = pointCount - 2;
                        for (uint32 i = 0; i < triangleCount; ++i)
                        {
                            *commandIBufferPtr = commandVBufferOffset + i + 2;  ++commandIBufferPtr;
                            *commandIBufferPtr = commandVBufferOffset + i + 1;  ++commandIBufferPtr;
                            *commandIBufferPtr = commandVBufferOffset;          ++commandIBufferPtr;
                        }
                        commandPacket.primitiveCount += triangleCount;
                    }
                    else
                    {
                        const uint32 linesCount = pointCount - 1;
                        for (uint32 i = 0; i < linesCount; ++i)
                        {
                            *commandIBufferPtr = commandVBufferOffset + i;        ++commandIBufferPtr;
                            *commandIBufferPtr = commandVBufferOffset + i + 1;    ++commandIBufferPtr;
                        }
                        commandPacket.primitiveCount += linesCount;
                    }
                    commandVBufferOffset += pointCount;
                } break;

                case COMMAND_DRAW_BOX:
                {
                    const uint32 primitiveColor = rhi::NativeColorRGBA(command.params[12], command.params[13], command.params[14], command.params[15]);
                    const Vector3 basePoint(command.params.data()), xAxis(command.params.data() + 3), yAxis(command.params.data() + 6), zAxis(command.params.data() + 9);

                    (*commandVBufferPtr).position = basePoint;                          (*commandVBufferPtr).color = primitiveColor;    ++commandVBufferPtr;
                    (*commandVBufferPtr).position = basePoint + xAxis;                  (*commandVBufferPtr).color = primitiveColor;    ++commandVBufferPtr;
                    (*commandVBufferPtr).position = basePoint + yAxis;                  (*commandVBufferPtr).color = primitiveColor;    ++commandVBufferPtr;
                    (*commandVBufferPtr).position = basePoint + zAxis;                  (*commandVBufferPtr).color = primitiveColor;    ++commandVBufferPtr;

                    (*commandVBufferPtr).position = basePoint + xAxis + yAxis;          (*commandVBufferPtr).color = primitiveColor;    ++commandVBufferPtr;
                    (*commandVBufferPtr).position = basePoint + xAxis + zAxis;          (*commandVBufferPtr).color = primitiveColor;    ++commandVBufferPtr;
                    (*commandVBufferPtr).position = basePoint + yAxis + zAxis;          (*commandVBufferPtr).color = primitiveColor;    ++commandVBufferPtr;
                    (*commandVBufferPtr).position = basePoint + xAxis + yAxis + zAxis;  (*commandVBufferPtr).color = primitiveColor;    ++commandVBufferPtr;

                    if (command.drawType & FLAG_DRAW_SOLID)
                    {
                        commandIBufferPtr += FillIndeciesFromArray(commandIBufferPtr, commandVBufferOffset, gSolidBoxIndexes);
                        commandPacket.primitiveCount += gSolidBoxIndexes.size() / 3;
                    }
                    else
                    {
                        commandIBufferPtr += FillIndeciesFromArray(commandIBufferPtr, commandVBufferOffset, gWireBoxIndexes);
                        commandPacket.primitiveCount += gWireBoxIndexes.size() / 2;
                    }
                    commandVBufferOffset += 8;
                } break;

                case COMMAND_DRAW_ICOSA:
                {
                    const uint32 primitiveColor = rhi::NativeColorRGBA(command.params[4], command.params[5], command.params[6], command.params[7]);
                    const Vector3 icosaPosition(command.params[0], command.params[1], command.params[2]);
                    const float32 icosaSize = command.params[3];
                    for (const Vector3 & vPosition : gIcosaVertexes)
                    {
                        (*commandVBufferPtr).position = vPosition * icosaSize + icosaPosition;
                        (*commandVBufferPtr).color = primitiveColor;
                        ++commandVBufferPtr;
                    }

                    if (command.drawType & FLAG_DRAW_SOLID)
                    {
                        commandPacket.primitiveCount += gSolidIcosaIndexes.size() / 3;
                        commandIBufferPtr += FillIndeciesFromArray(commandIBufferPtr, commandVBufferOffset, gSolidIcosaIndexes);
                    }
                    else
                    {
                        commandPacket.primitiveCount += gWireIcosaIndexes.size() / 2;
                        commandIBufferPtr += FillIndeciesFromArray(commandIBufferPtr, commandVBufferOffset, gWireIcosaIndexes);
                    }
                    commandVBufferOffset += gIcosaVertexes.size();
                } break;
            
                case COMMAND_DRAW_ARROW:
                {
                    uint32 arrowColor = rhi::NativeColorRGBA(command.params[6], command.params[7], command.params[8], command.params[9]);
                    const Vector3 from(command.params.data());
                    const Vector3 to(command.params.data() + 3);

                    Vector3 direction = to - from;
                    float32 arrowlength = direction.Normalize();
                    float32 arrowWidth = arrowlength / 4.f;

                    const Vector3 ortho1 = Abs(direction.x) < Abs(direction.y) ? direction.CrossProduct(Vector3(1.f, 0.f, 0.f)) : direction.CrossProduct(Vector3(0.f, 1.f, 0.f));
                    const Vector3 ortho2 = ortho1.CrossProduct(direction);

                    (*commandVBufferPtr).position = to;                           (*commandVBufferPtr).color = arrowColor;     ++commandVBufferPtr;
                    (*commandVBufferPtr).position = from + ortho1 * arrowWidth;   (*commandVBufferPtr).color = arrowColor;     ++commandVBufferPtr;
                    (*commandVBufferPtr).position = from + ortho2 * arrowWidth;   (*commandVBufferPtr).color = arrowColor;     ++commandVBufferPtr;
                    (*commandVBufferPtr).position = from - ortho1 * arrowWidth;   (*commandVBufferPtr).color = arrowColor;     ++commandVBufferPtr;
                    (*commandVBufferPtr).position = from - ortho2 * arrowWidth;   (*commandVBufferPtr).color = arrowColor;     ++commandVBufferPtr;

                    if (command.drawType & FLAG_DRAW_SOLID)
                    {
                        commandPacket.primitiveCount += gSolidArrowIndexes.size() / 3;
                        commandIBufferPtr += FillIndeciesFromArray(commandIBufferPtr, commandVBufferOffset, gSolidArrowIndexes);
                    }
                    else
                    {
                        commandPacket.primitiveCount += gWireArrowIndexes.size() / 2;
                        commandIBufferPtr += FillIndeciesFromArray(commandIBufferPtr, commandVBufferOffset, gWireArrowIndexes);
                    }
                    commandVBufferOffset += 5;
                } break;

            default: break;
            }
        }

        for (int32 i = 0; i < DRAW_TYPE_COUNT; ++i)
        {
            if (packet[i].primitiveCount)
                rhi::AddPacket(packetList, packet[i]);
        }
    }

    void RenderHelper::Clear()
    {
        commandQueue.clear();
        for (int32 i = 0; i < DRAW_TYPE_COUNT; ++i)
            buffersElemCount[i].first = buffersElemCount[i].second = 0;
    }

    bool RenderHelper::IsEmpty()
    {
        return (commandQueue.size() == 0);
    }

    void RenderHelper::QueueCommand(const DrawCommand && command)
    {
        commandQueue.emplace_back(std::move(command));

        uint32 vertexCount = 0, indexCount = 0;
        GetRequestedVertexCount(commandQueue.back(), vertexCount, indexCount);

        buffersElemCount[commandQueue.back().drawType].first += vertexCount;
        buffersElemCount[commandQueue.back().drawType].second += indexCount;
    }

    void RenderHelper::GetRequestedVertexCount(const DrawCommand & command, uint32 & vertexCount, uint32 & indexCount)
    {
        switch (command.id)
        {
            case COMMAND_DRAW_LINE:
            {
                vertexCount = 2;
                indexCount = 2;
            } break;

            case COMMAND_DRAW_POLYGON:
            {
                vertexCount = (command.params.size() - 4) / 3;
                indexCount = (command.drawType & FLAG_DRAW_SOLID) ? (vertexCount - 2) * 3 : (vertexCount - 1) * 2;
            } break;

            case COMMAND_DRAW_BOX:
            {
                vertexCount = 8;
                indexCount = (command.drawType & FLAG_DRAW_SOLID) ? gSolidBoxIndexes.size() : gWireBoxIndexes.size();
            } break;

            case COMMAND_DRAW_ICOSA:
            {
                vertexCount = gIcosaVertexes.size();
                indexCount = (command.drawType & FLAG_DRAW_SOLID) ? gSolidIcosaIndexes.size() : gWireIcosaIndexes.size();
            } break;
            
            case COMMAND_DRAW_ARROW:
            {
                vertexCount = 5;
                indexCount = (command.drawType & FLAG_DRAW_SOLID) ? gSolidArrowIndexes.size() : gWireArrowIndexes.size();
            } break;

            default:
            {
                DVASSERT(false && "DrawCommand not implemented");
            } break;
        }
    }

    void RenderHelper::DrawLine(const Vector3 & pt1, const Vector3 & pt2, const Color & color, eDrawType drawType /*  = DRAW_WIRE_DEPTH */)
    {
        DVASSERT((drawType & FLAG_DRAW_SOLID) == 0);
        QueueCommand(DrawCommand{ COMMAND_DRAW_LINE, drawType, { pt1.x, pt1.y, pt1.z, pt2.x, pt2.y, pt2.z, color.r, color.g, color.b, color.a } });
    }
    void RenderHelper::DrawPolygon(const Polygon3 & polygon, const Color & color, eDrawType drawType)
    {
        Vector<float32> args(4 + polygon.pointCount * 3);
        Memcpy(args.data(), color.color, sizeof(Color));
        Memcpy(args.data() + 4, polygon.points.data(), sizeof(Vector3) * polygon.pointCount);
        QueueCommand(DrawCommand{ COMMAND_DRAW_POLYGON, drawType, args });
    }
    void RenderHelper::DrawAABox(const AABBox3 & box, const Color & color, eDrawType drawType)
    {
        QueueCommand(DrawCommand{ COMMAND_DRAW_BOX, drawType, { box.min.x, box.min.y, box.min.z,      //basePoint
                                                                box.max.x - box.min.x, 0.f, 0.f,      //xAxis
                                                                0.f, box.max.y - box.min.y, 0.f,      //yAxis
                                                                0.f, 0.f, box.max.z - box.min.z,      //zAxis
                                                                color.r, color.g, color.b, color.a } });
    }
    void RenderHelper::DrawOBox(const AABBox3 & box, const Matrix4 & matrix, const Color & color, eDrawType drawType)
    {
        Vector3 minPt = box.min * matrix;
        Vector3 xAxis = MultiplyVectorMat3x3(Vector3(box.max.x - box.min.x, 0.f, 0.f), matrix);
        Vector3 yAxis = MultiplyVectorMat3x3(Vector3(0.f, box.max.y - box.min.y, 0.f), matrix);
        Vector3 zAxis = MultiplyVectorMat3x3(Vector3(0.f, 0.f, box.max.z - box.min.z), matrix);
        QueueCommand(DrawCommand{ COMMAND_DRAW_BOX, drawType, { minPt.x, minPt.y, minPt.z,
                                                                xAxis.x, xAxis.y, xAxis.z,
                                                                yAxis.x, yAxis.y, yAxis.z,
                                                                zAxis.x, zAxis.y, zAxis.z,
                                                                color.r, color.g, color.b, color.a } });
    }
    void RenderHelper::DrawCornerAABox(const AABBox3 & box, const Color & color, eDrawType drawType)
    {
        float32 offs = ((box.max - box.min).Length()) * 0.1f + 0.1f;

        //1
        Vector3 point = box.min;
        DrawLine(point, point + Vector3(0, 0, offs), color, drawType);
        DrawLine(point, point + Vector3(0, offs, 0), color, drawType);
        DrawLine(point, point + Vector3(offs, 0, 0), color, drawType);

        //2
        point = box.max;
        DrawLine(point, point - Vector3(0, 0, offs), color, drawType);
        DrawLine(point, point - Vector3(0, offs, 0), color, drawType);
        DrawLine(point, point - Vector3(offs, 0, 0), color, drawType);

        //3
        point = Vector3(box.min.x, box.max.y, box.min.z);
        DrawLine(point, point + Vector3(0, 0, offs), color, drawType);
        DrawLine(point, point - Vector3(0, offs, 0), color, drawType);
        DrawLine(point, point + Vector3(offs, 0, 0), color, drawType);

        //4
        point = Vector3(box.max.x, box.max.y, box.min.z);
        DrawLine(point, point + Vector3(0, 0, offs), color, drawType);
        DrawLine(point, point - Vector3(0, offs, 0), color, drawType);
        DrawLine(point, point - Vector3(offs, 0, 0), color, drawType);

        //5
        point = Vector3(box.max.x, box.min.y, box.min.z);
        DrawLine(point, point + Vector3(0, 0, offs), color, drawType);
        DrawLine(point, point + Vector3(0, offs, 0), color, drawType);
        DrawLine(point, point - Vector3(offs, 0, 0), color, drawType);

        //6
        point = Vector3(box.min.x, box.max.y, box.max.z);
        DrawLine(point, point - Vector3(0, 0, offs), color, drawType);
        DrawLine(point, point - Vector3(0, offs, 0), color, drawType);
        DrawLine(point, point + Vector3(offs, 0, 0), color, drawType);

        //7
        point = Vector3(box.min.x, box.min.y, box.max.z);
        DrawLine(point, point - Vector3(0, 0, offs), color, drawType);
        DrawLine(point, point + Vector3(0, offs, 0), color, drawType);
        DrawLine(point, point + Vector3(offs, 0, 0), color, drawType);

        //8
        point = Vector3(box.max.x, box.min.y, box.max.z);
        DrawLine(point, point - Vector3(0, 0, offs), color, drawType);
        DrawLine(point, point + Vector3(0, offs, 0), color, drawType);
        DrawLine(point, point - Vector3(offs, 0, 0), color, drawType);
    }
    void RenderHelper::DrawArrow(const Vector3 & from, const Vector3 & to, float32 arrowLength, const Color & color, eDrawType drawType)
    {
        Vector3 direction = to - from;
        Vector3 lineEnd = to - (direction * arrowLength / direction.Length());

        QueueCommand(DrawCommand{ COMMAND_DRAW_ARROW, drawType, { lineEnd.x, lineEnd.y, lineEnd.z, to.x, to.y, to.z, color.r, color.g, color.b, color.a } });
        DrawLine(from, lineEnd, color, eDrawType(drawType & FLAG_DRAW_NO_DEPTH));

        /*
        Vector3 d = to - from;
        Vector3 c = to - (d * arrowLength / d.Length());

        DAVA::float32 k = arrowLength / 4;

        Vector3 n = c.CrossProduct(to);

        if (n.IsZero())
        {
            if (0 == to.x) n = Vector3(1, 0, 0);
            else if (0 == to.y) n = Vector3(0, 1, 0);
            else if (0 == to.z) n = Vector3(0, 0, 1);
        }

        n.Normalize();
        n *= k;

        Vector3 p1 = c + n;
        Vector3 p2 = c - n;

        Vector3 nd = d.CrossProduct(n);
        nd.Normalize();
        nd *= k;

        Vector3 p3 = c + nd;
        Vector3 p4 = c - nd;

        Polygon3 poly;
        poly.points.reserve(3);

        poly.AddPoint(p1);
        poly.AddPoint(p3);
        poly.AddPoint(p2);
        DrawPolygon(poly, color, drawType);

        poly.Clear();
        poly.AddPoint(p1);
        poly.AddPoint(p4);
        poly.AddPoint(p2);
        DrawPolygon(poly, color, drawType);

        poly.Clear();
        poly.AddPoint(p1);
        poly.AddPoint(p3);
        poly.AddPoint(to);
        DrawPolygon(poly, color, drawType);

        poly.Clear();
        poly.AddPoint(p1);
        poly.AddPoint(p4);
        poly.AddPoint(to);
        DrawPolygon(poly, color, drawType);

        poly.Clear();
        poly.AddPoint(p2);
        poly.AddPoint(p3);
        poly.AddPoint(to);
        DrawPolygon(poly, color, drawType);

        poly.Clear();
        poly.AddPoint(p2);
        poly.AddPoint(p4);
        poly.AddPoint(to);
        DrawPolygon(poly, color, drawType);

        DrawLine(from, c, color);

        */
    }
    void RenderHelper::DrawIcosahedron(const Vector3 & position, float32 radius, const Color & color, eDrawType drawType)
    {
        QueueCommand(DrawCommand{ COMMAND_DRAW_ICOSA, drawType, { position.x, position.y, position.z, radius, color.r, color.g, color.b, color.a } });
    }
    void RenderHelper::DrawCircle(const Vector3 & center, const Vector3 &directionVector, float32 radius, const Color & color, eDrawType drawType)
    {
        Polygon3 pts;
        MakeCirclePolygon(pts, center, directionVector, radius);
        DrawPolygon(pts, color, drawType);
    }
    void RenderHelper::DrawBSpline(BezierSpline3 * bSpline, int segments, float ts, float te, const Color & color, eDrawType drawType)
    {
        Polygon3 pts;
        pts.points.reserve(segments);
        for (int k = 0; k < segments; ++k)
        {
            pts.AddPoint(bSpline->Evaluate(0, ts + (te - ts) * ((float)k / (float)(segments - 1))));
        }
        DrawPolygon(pts, color, drawType);
    }
    void RenderHelper::DrawInterpolationFunc(Interpolation::Func func, const Rect & destRect, const Color & color, eDrawType drawType)
    {
        Polygon3 pts;
        int segmentsCount = 20;
        pts.points.reserve(segmentsCount);
        for (int k = 0; k < segmentsCount; ++k)
        {
            Vector3 v;
            v.x = destRect.x + ((float)k / (float)(segmentsCount - 1)) * destRect.dx;
            v.y = destRect.y + func(((float)k / (float)(segmentsCount - 1))) * destRect.dy;
            v.z = 0.0f;
            pts.AddPoint(v);
        }
        DrawPolygon(pts, color, drawType);
    }
    
    void RenderHelper::MakeCirclePolygon(Polygon3 & outPolygon, const Vector3 & center, const Vector3 &directionVector, float32 radius)
    {
        float32 angle = Min(PI / 6.0f, CIRCLE_SEGMENT_LENGTH / radius);// maximum angle 30 degrees
        int ptsCount = (int)(PI_2 / (DegToRad(angle))) + 1;

        outPolygon.points.reserve(ptsCount);
        for (int k = 0; k < ptsCount; ++k)
        {
            float32 angleA = ((float)k / (ptsCount - 1)) * PI_2;
            float sinAngle = 0.0f;
            float cosAngle = 0.0f;
            SinCosFast(angleA, sinAngle, cosAngle);

            Vector3 directionVector(radius * cosAngle,
                radius * sinAngle,
                0.0f);

            // Rotate the direction vector according to the current emission vector value.
            Vector3 zNormalVector(0.0f, 0.0f, 1.0f);
            Vector3 curEmissionVector = directionVector;
            if (FLOAT_EQUAL(curEmissionVector.Length(), 0.f) == false)
            {
                curEmissionVector.Normalize();
            }

            // This code rotates the (XY) plane with the particles to the direction vector.
            // Taking into account that a normal vector to the (XY) plane is (0,0,1) this
            // code is very simplified version of the generic "plane rotation" code.
            float32 length = curEmissionVector.Length();
            if (FLOAT_EQUAL(length, 0.0f) == false)
            {
                float32 cosAngleRot = curEmissionVector.z / length;
                float32 angleRot = acos(cosAngleRot);
                Vector3 axisRot(curEmissionVector.y, -curEmissionVector.x, 0);
                if (FLOAT_EQUAL(axisRot.Length(), 0.f) == false)
                {
                    axisRot.Normalize();
                }
                Matrix3 planeRotMatrix;
                planeRotMatrix.CreateRotation(axisRot, angleRot);
                Vector3 rotatedVector = directionVector * planeRotMatrix;
                directionVector = rotatedVector;
            }

            Vector3 pos = center - directionVector;
            outPolygon.AddPoint(pos);
        }
    }

    void RenderHelper::CreateClearPass(rhi::HTexture targetHandle, int32 passPriority, const Color & clearColor, const rhi::Viewport & viewport)
    {
        rhi::RenderPassConfig clearPassConfig;
        clearPassConfig.priority = passPriority;
        clearPassConfig.colorBuffer[0].texture = targetHandle;
        clearPassConfig.colorBuffer[0].clearColor[0] = clearColor.r;
        clearPassConfig.colorBuffer[0].clearColor[1] = clearColor.g;
        clearPassConfig.colorBuffer[0].clearColor[2] = clearColor.b;
        clearPassConfig.colorBuffer[0].clearColor[3] = clearColor.a;
        clearPassConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
        clearPassConfig.colorBuffer[0].storeAction = rhi::STOREACTION_NONE;
        clearPassConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
        clearPassConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
        clearPassConfig.viewport = viewport;

        rhi::HPacketList emptyPacketList;
        rhi::HRenderPass clearPass = rhi::AllocateRenderPass(clearPassConfig, 1, &emptyPacketList);

        rhi::BeginRenderPass(clearPass);
        rhi::BeginPacketList(emptyPacketList);
        rhi::EndPacketList(emptyPacketList);
        rhi::EndRenderPass(clearPass);
    }
};
