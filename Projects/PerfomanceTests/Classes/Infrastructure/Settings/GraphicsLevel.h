/*
*  GraphicsLevel.h
*  WotBlitz
*
*  Created by Yury Danilov on 11/01/15.
*  Copyright 2015 wargaming.net All rights reserved.
*
*/

#ifndef __GRAPHICS_LEVEL_H__
#define __GRAPHICS_LEVEL_H__

#include "DAVAEngine.h"

class GraphicsLevel
{
public:
    GraphicsLevel(const DAVA::String & fileName, DAVA::YamlNode * node);
    ~GraphicsLevel();
    
    void Activate(void);

private:
   
    void ReadSettings(const DAVA::String & fileName, DAVA::YamlNode * node);

    DAVA::KeyedArchive* archive;

    DAVA::String  water;
    DAVA::String  vegetation;
    DAVA::String  tree_lighting;
    DAVA::String  landscape;
    DAVA::String  static_object;

    bool vegetationAnimation;
    bool stencilShadows;
};

#endif