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



#include "MathTest.h"

using namespace DAVA;

namespace
{
	float32 SquareDist(const Matrix4& m1, const Matrix4& m2)
	{
		float32 result = 0.0f;
		for (uint32 i = 0; i < 4; ++i)
			for (uint32 j = 0; j < 4; ++j)
				result += Abs(m1(i, j) - m2(i, j)) * Abs(m1(i, j) - m2(i, j));

		return result;
	}
}

MathTest::MathTest()
: TestTemplate<MathTest>("MathTest")
{
	RegisterFunction(this, &MathTest::MatrixTestFunction, "MatrixTestFunction", NULL);
}

void MathTest::LoadResources()
{
	GetBackground()->SetColor(Color::White);
}

void MathTest::UnloadResources()
{

}

void MathTest::MatrixTestFunction(PerfFuncData * data)
{
	TEST_VERIFY(TestMatrixDecomposition(Matrix4::MakeTranslation(Vector3(10.0f, 0.0f, 0.0f))) < 0.0001f);
	TEST_VERIFY(TestMatrixDecomposition(Matrix4::MakeRotation(Vector3(1.0f, 0.0f, 0.0f), PI_05)) < 0.0001f);
	TEST_VERIFY(TestMatrixDecomposition(Matrix4::MakeScale(Vector3(3.0f, 3.0f, 3.0f))) < 0.0001f);

	Vector3 axis(0.0f, 1.0f, 1.0f);
	axis.Normalize();
	TEST_VERIFY(
		TestMatrixDecomposition(
		Matrix4::MakeTranslation(Vector3(10.0f, 0.0f, 0.0f)) *
		Matrix4::MakeRotation(axis, PI_05 * 0.25f) *
		Matrix4::MakeScale(Vector3(3.0f, 3.0f, 3.0f))) < 0.0001f);
}

float32 MathTest::TestMatrixDecomposition(const Matrix4& mat)
{
	Vector3 position, scale;
	Quaternion rotation;
	mat.Decomposition(position, scale, rotation);

	Matrix4 reconstructedMatrix = rotation.GetMatrix() * Matrix4::MakeScale(scale);
	reconstructedMatrix.SetTranslationVector(position);

	return SquareDist(reconstructedMatrix, mat);
}