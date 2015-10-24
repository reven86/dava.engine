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
#include "Render/DynamicBufferAllocator.h"
#include "Material/NMaterial.h"

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

std::array<DAVA::uint16, 48> gWireBoxCornersIndexes = {
    0,  8,   0,  9,   0, 10,   1, 11,   1, 12,   1, 13,
    2, 14,   2, 15,   2, 16,   3, 17,   3, 18,   3, 19,
    4, 20,   4, 21,   4, 22,   5, 23,   5, 24,   5, 25,
    6, 26,   6, 27,   6, 28,   7, 29,   7, 30,   7, 31,
};

std::array<DAVA::uint16, 72> gSolidBoxCornersIndexes = {
    0,  8, 10,   0, 10,  9,  0,  9,  8,  1, 12, 13,   1, 11, 12,  1, 13, 11,
    2, 14, 16,   2, 16, 15,  2, 15, 14,  3, 19, 17,   3, 17, 18,  3, 18, 19,
    4, 22, 21,   4, 20, 22,  4, 21, 20,  5, 23, 25,   5, 24, 23,  5, 25, 24,
    6, 27, 26,   6, 26, 28,  6, 28, 27,  7, 29, 30,   7, 30, 31,  7, 31, 29,
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

    void RenderHelper::InvalidateMaterials()
    {
        for (NMaterial*& material : materials)
            material->InvalidateRenderVariants();
    }

    RenderHelper::~RenderHelper()
    {
        for (int32 i = 0; i < DRAW_TYPE_COUNT; ++i)
            SafeRelease(materials[i]);
    }

    bool RenderHelper::PreparePacket(rhi::Packet& packet, NMaterial* material, const std::pair<uint32, uint32>& buffersCount, ColoredVertex** vBufferDataPtr, uint16** iBufferDataPtr)
    {
        if (!material->PreBuildMaterial(PASS_FORWARD))
            return false;
        material->BindParams(packet);
        packet.vertexStreamCount = 1;
        packet.vertexLayoutUID = coloredVertexLayoutUID;

        if (buffersCount.first)
        {
            DynamicBufferAllocator::AllocResultVB vb = DynamicBufferAllocator::AllocateVertexBuffer(sizeof(ColoredVertex), buffersCount.first);
            DVASSERT(vb.allocatedVertices == buffersCount.first);
            *vBufferDataPtr = reinterpret_cast<ColoredVertex *>(vb.data);
            packet.vertexStream[0] = vb.buffer;
            packet.vertexCount = vb.allocatedVertices;
            packet.baseVertex = vb.baseVertex;
        }
        if (buffersCount.second)
        {
            DynamicBufferAllocator::AllocResultIB ib = DynamicBufferAllocator::AllocateIndexBuffer(buffersCount.second);
            DVASSERT(ib.allocatedindices == buffersCount.second);
            *iBufferDataPtr = ib.data;
            packet.indexBuffer = ib.buffer;
            packet.startIndex = ib.baseIndex;
        }

        return true;
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

        bool valid = true;
        for (int32 i = 0; i < DRAW_TYPE_COUNT; ++i)
            valid &= PreparePacket(packet[i], materials[i], buffersElemCount[i], &vBufferPtr[i], &iBufferPtr[i]);

        if (!valid)
            return;

        packet[DRAW_WIRE_DEPTH].primitiveType = packet[DRAW_WIRE_NO_DEPTH].primitiveType = rhi::PRIMITIVE_LINELIST;
        packet[DRAW_SOLID_DEPTH].primitiveType = packet[DRAW_SOLID_NO_DEPTH].primitiveType = rhi::PRIMITIVE_TRIANGLELIST;

        for (const DrawCommand & command : commandQueue)
        {
            ColoredVertex * const commandVBufferPtr = vBufferPtr[command.drawType];
            uint16 * const commandIBufferPtr = iBufferPtr[command.drawType];
            const uint32 commandVBufferOffset = vBufferOffset[command.drawType];

            const bool isWireDraw = (command.drawType & FLAG_DRAW_SOLID) == 0;

            uint32 nativePrimitiveColor = rhi::NativeColorRGBA(command.params[0], command.params[1], command.params[2], command.params[3]);
            uint32 vertexCount = 0, indexCount = 0;
            GetRequestedVertexCount(command, vertexCount, indexCount);

            switch (command.id)
            {
                case COMMAND_DRAW_LINE:
                {
                    commandVBufferPtr[0].position = Vector3(command.params[4], command.params[5], command.params[6]);
                    commandVBufferPtr[0].color = nativePrimitiveColor;

                    commandVBufferPtr[1].position = Vector3(command.params[7], command.params[8], command.params[9]);
                    commandVBufferPtr[1].color = nativePrimitiveColor;

                    commandIBufferPtr[0] = commandVBufferOffset;
                    commandIBufferPtr[1] = commandVBufferOffset + 1;

                } break;

                case COMMAND_DRAW_POLYGON:
                {
                    const uint32 pointCount = static_cast<uint32>((command.params.size() - 4) / 3);

                    const Vector3 * const polygonPoints = reinterpret_cast<const Vector3 *>(command.params.data() + 4);
                    for (uint32 i = 0; i < pointCount; ++i)
                    {
                        commandVBufferPtr[i].position = polygonPoints[i];
                        commandVBufferPtr[i].color = nativePrimitiveColor;
                    }

                    FillPolygonIndecies(commandIBufferPtr, commandVBufferOffset, indexCount, vertexCount, isWireDraw);

                } break;

                case COMMAND_DRAW_BOX:
                {
                    const Vector3 basePoint(command.params.data() + 4), xAxis(command.params.data() + 7), yAxis(command.params.data() + 10), zAxis(command.params.data() + 13);
                    FillBoxVBuffer(commandVBufferPtr, basePoint, xAxis, yAxis, zAxis, nativePrimitiveColor);
                    FillIndeciesFromArray(commandIBufferPtr, vBufferOffset[command.drawType], isWireDraw ? gWireBoxIndexes.data() : gSolidBoxIndexes.data(), indexCount);

                } break;

                case COMMAND_DRAW_BOX_CORNERS:
                {
                    const Vector3 basePoint(command.params.data() + 4), xAxis(command.params.data() + 7), yAxis(command.params.data() + 10), zAxis(command.params.data() + 13);
                    FillBoxCornersVBuffer(commandVBufferPtr, basePoint, xAxis, yAxis, zAxis, nativePrimitiveColor);
                    FillIndeciesFromArray(commandIBufferPtr, vBufferOffset[command.drawType], isWireDraw ? gWireBoxCornersIndexes.data() : gSolidBoxCornersIndexes.data(), indexCount);

                } break;

                case COMMAND_DRAW_CIRCLE:
                { 
                    const uint32 pointCount = (uint32)(command.params[11]);
                    const Vector3 center(command.params.data() + 4), direction(command.params.data() + 7);
                    const float32 radius = command.params[10];
                    FillCircleVBuffer(commandVBufferPtr, center, direction, radius, pointCount, nativePrimitiveColor);

                    FillPolygonIndecies(commandIBufferPtr, commandVBufferOffset, indexCount, vertexCount, isWireDraw);
                    if (isWireDraw)
                    {
                        commandIBufferPtr[vertexCount * 2 - 2] = commandVBufferOffset + vertexCount - 1;
                        commandIBufferPtr[vertexCount * 2 - 1] = commandVBufferOffset;
                    }

                } break;

                case COMMAND_DRAW_ICOSA:
                {
                    const Vector3 icosaPosition(command.params[4], command.params[5], command.params[6]);
                    const float32 icosaSize = command.params[7];
                    for (size_t i = 0; i < gIcosaVertexes.size(); ++i)
                    {
                        commandVBufferPtr[i].position = gIcosaVertexes[i] * icosaSize + icosaPosition;
                        commandVBufferPtr[i].color = nativePrimitiveColor;
                    }
                    FillIndeciesFromArray(commandIBufferPtr, vBufferOffset[command.drawType], isWireDraw ? gWireIcosaIndexes.data() : gSolidIcosaIndexes.data(), indexCount);

                } break;
            
                case COMMAND_DRAW_ARROW:
                {
                    const Vector3 from(command.params.data() + 4);
                    const Vector3 to(command.params.data() + 7);
                    FillArrowVBuffer(commandVBufferPtr, from, to, nativePrimitiveColor);
                    FillIndeciesFromArray(commandIBufferPtr, vBufferOffset[command.drawType], isWireDraw ? gWireArrowIndexes.data() : gSolidArrowIndexes.data(), indexCount);

                } break;

            default: break;
            }

            vBufferPtr[command.drawType] += vertexCount;
            iBufferPtr[command.drawType] += indexCount;

            vBufferOffset[command.drawType] += vertexCount;
            packet[command.drawType].primitiveCount += isWireDraw ? indexCount / 2 : indexCount / 3;
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
        bool isSolidDraw = (command.drawType & FLAG_DRAW_SOLID) != 0;

        switch (command.id)
        {
            case COMMAND_DRAW_LINE:
            {
                vertexCount = 2;
                indexCount = 2;
            } break;

            case COMMAND_DRAW_POLYGON:
            {
                vertexCount = static_cast<uint32>((command.params.size() - 4) / 3);
                indexCount = isSolidDraw ? (vertexCount - 2) * 3 : (vertexCount - 1) * 2;
            } break;

            case COMMAND_DRAW_BOX:
            {
                vertexCount = 8;
                size_t count = isSolidDraw ? gSolidBoxIndexes.size() : gWireBoxIndexes.size();
                indexCount = static_cast<uint32>(count);
            } break;

            case COMMAND_DRAW_BOX_CORNERS:
            {
                vertexCount = 32;
                size_t count = isSolidDraw ? gSolidBoxCornersIndexes.size() : gWireBoxCornersIndexes.size();
                indexCount = static_cast<uint32>(count);
            } break;

            case COMMAND_DRAW_CIRCLE:
            {
                vertexCount = (uint32)(command.params[11]);
                indexCount = isSolidDraw ? (vertexCount - 2) * 3 : vertexCount * 2;
            } break;

            case COMMAND_DRAW_ICOSA:
            {
                vertexCount = static_cast<uint32>(gIcosaVertexes.size());
                size_t count = isSolidDraw ? gSolidIcosaIndexes.size() : gWireIcosaIndexes.size();
                indexCount = static_cast<uint32>(count);
            } break;
            
            case COMMAND_DRAW_ARROW:
            {
                vertexCount = 5;
                size_t count = isSolidDraw ? gSolidArrowIndexes.size() : gWireArrowIndexes.size();
                indexCount = static_cast<uint32>(count);
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
        QueueCommand(DrawCommand{ COMMAND_DRAW_LINE, drawType, { color.r, color.g, color.b, color.a, pt1.x, pt1.y, pt1.z, pt2.x, pt2.y, pt2.z} });
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
        QueueDrawBoxCommand(COMMAND_DRAW_BOX, box, nullptr, color, drawType);
    }
    void RenderHelper::DrawAABoxTransformed(const AABBox3 & box, const Matrix4 & matrix, const Color & color, eDrawType drawType)
    {
        QueueDrawBoxCommand(COMMAND_DRAW_BOX, box, &matrix, color, drawType);
    }
    void RenderHelper::DrawAABoxCorners(const AABBox3 & box, const Color & color, eDrawType drawType)
    {
        QueueDrawBoxCommand(COMMAND_DRAW_BOX_CORNERS, box, nullptr, color, drawType);
    }
    void RenderHelper::DrawAABoxCornersTransformed(const AABBox3 & box, const Matrix4 & matrix, const Color & color, eDrawType drawType)
    {
        QueueDrawBoxCommand(COMMAND_DRAW_BOX_CORNERS, box, &matrix, color, drawType);
    }
    void RenderHelper::DrawArrow(const Vector3 & from, const Vector3 & to, float32 arrowLength, const Color & color, eDrawType drawType)
    {
        Vector3 direction = to - from;
        Vector3 lineEnd = to - (direction * arrowLength / direction.Length());

        QueueCommand(DrawCommand{ COMMAND_DRAW_ARROW, drawType, { color.r, color.g, color.b, color.a, lineEnd.x, lineEnd.y, lineEnd.z, to.x, to.y, to.z } });
        DrawLine(from, lineEnd, color, eDrawType(drawType & FLAG_DRAW_NO_DEPTH));
    }
    void RenderHelper::DrawIcosahedron(const Vector3 & position, float32 radius, const Color & color, eDrawType drawType)
    {
        QueueCommand(DrawCommand{ COMMAND_DRAW_ICOSA, drawType, { color.r, color.g, color.b, color.a, position.x, position.y, position.z, radius } });
    }
    void RenderHelper::DrawCircle(const Vector3 & center, const Vector3 &direction, float32 radius, uint32 segmentCount, const Color & color, eDrawType drawType)
    {
        QueueCommand(DrawCommand{ COMMAND_DRAW_CIRCLE, drawType, { color.r, color.g, color.b, color.a,
                                                                   center.x, center.y, center.z,
                                                                   direction.x, direction.y, direction.z,
                                                                   radius, (float32)(segmentCount) } });
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

    void RenderHelper::QueueDrawBoxCommand(eDrawCommandID commandID, const AABBox3 & box, const Matrix4 * matrix, const Color & color, eDrawType drawType)
    {
        Vector3 minPt = box.min;
        Vector3 xAxis(box.max.x - box.min.x, 0.f, 0.f);
        Vector3 yAxis(0.f, box.max.y - box.min.y, 0.f);
        Vector3 zAxis(0.f, 0.f, box.max.z - box.min.z);

        if (matrix)
        {
            minPt = minPt * (*matrix);
            xAxis = MultiplyVectorMat3x3(xAxis, *matrix);
            yAxis = MultiplyVectorMat3x3(yAxis, *matrix);
            zAxis = MultiplyVectorMat3x3(zAxis, *matrix);
        }

        QueueCommand(DrawCommand{ commandID, drawType, { color.r, color.g, color.b, color.a,
                                                         minPt.x, minPt.y, minPt.z,
                                                         xAxis.x, xAxis.y, xAxis.z,
                                                         yAxis.x, yAxis.y, yAxis.z,
                                                         zAxis.x, zAxis.y, zAxis.z,
                                                        } });
    }

    void RenderHelper::FillBoxVBuffer(ColoredVertex * buffer, const Vector3 & basePoint, const Vector3 & xAxis, const Vector3 & yAxis, const Vector3 & zAxis, uint32 nativeColor)
    {
        buffer[0].position = basePoint;                          buffer[0].color = nativeColor;
        buffer[1].position = basePoint + xAxis;                  buffer[1].color = nativeColor;
        buffer[2].position = basePoint + yAxis;                  buffer[2].color = nativeColor;
        buffer[3].position = basePoint + zAxis;                  buffer[3].color = nativeColor;

        buffer[4].position = basePoint + xAxis + yAxis;          buffer[4].color = nativeColor;
        buffer[5].position = basePoint + xAxis + zAxis;          buffer[5].color = nativeColor;
        buffer[6].position = basePoint + yAxis + zAxis;          buffer[6].color = nativeColor;
        buffer[7].position = basePoint + xAxis + yAxis + zAxis;  buffer[7].color = nativeColor;
    }

    void RenderHelper::FillBoxCornersVBuffer(ColoredVertex * buffer, const Vector3 & basePoint, const Vector3 & xAxis, const Vector3 & yAxis, const Vector3 & zAxis, uint32 nativeColor)
    {
        FillBoxVBuffer(buffer, basePoint, xAxis, yAxis, zAxis, nativeColor);

        const float32 cornerLength = ((buffer[0].position - buffer[7].position).Length()) * 0.1f + 0.1f;

        const Vector3 xCorner = Normalize(xAxis) * cornerLength;
        const Vector3 yCorner = Normalize(yAxis) * cornerLength;
        const Vector3 zCorner = Normalize(zAxis) * cornerLength;

        buffer[8 + 0 * 3 + 0].position = buffer[0].position + xCorner;
        buffer[8 + 0 * 3 + 1].position = buffer[0].position + yCorner;
        buffer[8 + 0 * 3 + 2].position = buffer[0].position + zCorner;

        buffer[8 + 1 * 3 + 0].position = buffer[1].position - xCorner;
        buffer[8 + 1 * 3 + 1].position = buffer[1].position + yCorner;
        buffer[8 + 1 * 3 + 2].position = buffer[1].position + zCorner;

        buffer[8 + 2 * 3 + 0].position = buffer[2].position + xCorner;
        buffer[8 + 2 * 3 + 1].position = buffer[2].position - yCorner;
        buffer[8 + 2 * 3 + 2].position = buffer[2].position + zCorner;

        buffer[8 + 3 * 3 + 0].position = buffer[3].position + xCorner;
        buffer[8 + 3 * 3 + 1].position = buffer[3].position + yCorner;
        buffer[8 + 3 * 3 + 2].position = buffer[3].position - zCorner;

        buffer[8 + 4 * 3 + 0].position = buffer[4].position - xCorner;
        buffer[8 + 4 * 3 + 1].position = buffer[4].position - yCorner;
        buffer[8 + 4 * 3 + 2].position = buffer[4].position + zCorner;

        buffer[8 + 5 * 3 + 0].position = buffer[5].position - xCorner;
        buffer[8 + 5 * 3 + 1].position = buffer[5].position + yCorner;
        buffer[8 + 5 * 3 + 2].position = buffer[5].position - zCorner;

        buffer[8 + 6 * 3 + 0].position = buffer[6].position + xCorner;
        buffer[8 + 6 * 3 + 1].position = buffer[6].position - yCorner;
        buffer[8 + 6 * 3 + 2].position = buffer[6].position - zCorner;

        buffer[8 + 7 * 3 + 0].position = buffer[7].position - xCorner;
        buffer[8 + 7 * 3 + 1].position = buffer[7].position - yCorner;
        buffer[8 + 7 * 3 + 2].position = buffer[7].position - zCorner;

        for (int32 i = 8; i < 32; ++i)
            buffer[i].color = nativeColor;
    }
    void RenderHelper::FillCircleVBuffer(ColoredVertex * buffer, const Vector3 & center, const Vector3 & dir, float32 radius, uint32 pointCount, uint32 nativeColor)
    {
        const Vector3 direction = Normalize(dir);
        const Vector3 ortho = Abs(direction.x) < Abs(direction.y) ? direction.CrossProduct(Vector3(1.f, 0.f, 0.f)) : direction.CrossProduct(Vector3(0.f, 1.f, 0.f));

        Matrix4 rotationMx;
        float32 angleDelta = PI_2 / (float32)pointCount;
        for (uint32 i = 0; i < pointCount; ++i)
        {
            rotationMx.CreateRotation(direction, angleDelta * i);
            buffer[i].position = center + (ortho * radius) * rotationMx;
            buffer[i].color = nativeColor;
        }
    }
    void RenderHelper::FillArrowVBuffer(ColoredVertex * buffer, const Vector3 & from, const Vector3 & to, uint32 nativeColor)
    {
        Vector3 direction = to - from;
        float32 arrowlength = direction.Normalize();
        float32 arrowWidth = arrowlength / 4.f;

        const Vector3 ortho1 = Abs(direction.x) < Abs(direction.y) ? direction.CrossProduct(Vector3(1.f, 0.f, 0.f)) : direction.CrossProduct(Vector3(0.f, 1.f, 0.f));
        const Vector3 ortho2 = ortho1.CrossProduct(direction);

        buffer[0].position = to;                           buffer[0].color = nativeColor;
        buffer[1].position = from + ortho1 * arrowWidth;   buffer[1].color = nativeColor;
        buffer[2].position = from + ortho2 * arrowWidth;   buffer[2].color = nativeColor;
        buffer[3].position = from - ortho1 * arrowWidth;   buffer[3].color = nativeColor;
        buffer[4].position = from - ortho2 * arrowWidth;   buffer[4].color = nativeColor;
    }
    void RenderHelper::FillIndeciesFromArray(uint16 * buffer, uint16 baseIndex, uint16 * indexArray, uint32 indexCount)
    {
        for (uint32 i = 0; i < indexCount; ++i)
        {
            buffer[i] = baseIndex + indexArray[i];
        }
    }
    void RenderHelper::FillPolygonIndecies(uint16 * buffer, uint16 baseIndex, uint32 indexCount, uint32 vertexCount, bool isWire)
    {
        if (isWire)
        {
            const uint32 linesCount = vertexCount - 1;
            for (uint32 i = 0; i < linesCount; ++i)
            {
                buffer[i * 2 + 0] = baseIndex + i;
                buffer[i * 2 + 1] = baseIndex + i + 1;
            }
        }
        else
        {
            const uint32 triangleCount = vertexCount - 2;
            for (uint32 i = 0; i < triangleCount; ++i)
            {
                buffer[i * 3 + 0] = baseIndex + i + 2;
                buffer[i * 3 + 1] = baseIndex + i + 1;
                buffer[i * 3 + 2] = baseIndex;
            }
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
        clearPassConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
        clearPassConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
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
