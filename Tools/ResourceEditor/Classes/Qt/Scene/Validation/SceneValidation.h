#pragma once

namespace SceneValidation
{
/**
    For all parent models (i.e. models at which `scene` entities are referenced with "referenceToOwner" property)
    function checks whether local & world matrices are identity.
    If any validation check is failing, logger warning is emmited
*/
void ValidateMatrices(SceneEditor2* scene);

/**
    For all `scene` entities with same names function does following checks:
    a. "CollisionType", "CollisionTypeCrashed", "editor.ReferenceToOwner", "Health", "MaterialKind" corresponding properties should be equal.
    b. Sound components should be equal. This means that corresponding sound events should be equal
       Sound events are considered to be equal when
       1. event names are equal
       2. event min,max distance are equal
       3. event properties are equal
    c. Child effect entities should be equal. 
       Effects are considered to be equal when all corresponding effect entities have same names
    If any validation check is failing, logger warning is emmited
*/
void ValidateSameNames(SceneEditor2* scene);

/**
    For all `scene` entities that have "CollisionType" property specifed, "MaterialKind" or "FallType" properties are also should be specifed.
    Except for entites that have "CollisionType" = "Water".
    If any validation check is failing, logger warning is emmited
*/
void ValidateCollisionProperties(SceneEditor2* scene);

/**
    For all `scene` textures that have any converted textures, function checks whether converted textures are relevant, i.e. recompression is not needed
    If any validation check is failing, logger warning is emmited
*/
void ValidateTexturesRelevance(SceneEditor2* scene);

/**
    For all `scene` materials that have multi-quality material template, function checks whether quality group is specified.
    If any validation check is failing, logger warning is emmited
*/
void ValidateMaterialsGroups(SceneEditor2* scene);
};
