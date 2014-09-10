// /*==================================================================================
//     Copyright (c) 2008, binaryzebra
//     All rights reserved.
// 
//     Redistribution and use in source and binary forms, with or without
//     modification, are permitted provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//     * Neither the name of the binaryzebra nor the
//     names of its contributors may be used to endorse or promote products
//     derived from this software without specific prior written permission.
// 
//     THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
//     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//     DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
//     DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//     ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// =====================================================================================*/
// 
// 
// 
// #ifndef __DAVAENGINE_MATERIALHELPER_H__
// #define __DAVAENGINE_MATERIALHELPER_H__
// 
// #include "Base/BaseTypes.h"
// #include "Render/Material.h"
// 
// namespace DAVA
// {
// 	class MaterialHelper
// 	{
// 	private:
// 		
// 		class MaterialTypePredicate
// 		{
// 		private:
// 			
// 			Material::eType materialType;
// 			
// 		public:
// 			
// 			MaterialTypePredicate();
// 			MaterialTypePredicate(const Material::eType type);
// 			
// 			bool Validate(const Material* m);
// 		};
// 		
// 		template<class T>
// 		class ConditionalMaterialAccum
// 		{
// 		private:
// 			
// 			Vector<Material*>* accumulatedMaterials;
// 			T predicate;
// 			
// 		public:
// 			
// 			ConditionalMaterialAccum(T pred, Vector<Material*>* container)
// 			{
// 				predicate = pred;
// 				accumulatedMaterials = container;
// 			}
// 						
// 			void operator() (Material* m)
// 			{
// 				if(predicate.Validate(m))
// 				{
// 					accumulatedMaterials->push_back(m);
// 				}
// 			}
// 		};
// 		
// 	public:
// 		
// 		template<class Container>
// 		static int FilterMaterialsByType(Container& allMaterials, const Material::eType type)
// 		{
// 			Vector<Material*> accumulatedMaterials;
// 			ConditionalMaterialAccum<MaterialTypePredicate> accum((MaterialTypePredicate(type)), &accumulatedMaterials);
// 			std::for_each(allMaterials.begin(), allMaterials.end(), accum);
// 			
// 			for(size_t i = 0; i < accumulatedMaterials.size(); ++i)
// 			{
// 				typename Container::iterator iter = std::find(allMaterials.begin(), allMaterials.end(), accumulatedMaterials[i]);
// 				
// 				if(iter != allMaterials.end())
// 				{
// 					allMaterials.erase(iter);
// 				}
// 			}
// 			
// 			return accumulatedMaterials.size();
// 		}
// 	};
// };
// 
// #endif /* defined(__DAVAENGINE_MATERIALHELPER_H__) */
