// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Environment.h>
#include <noggit/MapHeaders.h>

Environment::Environment()
  : minRotation(0.0f)
  , maxRotation(360.0f)
  , minTilt(-5.0f)
  , maxTilt(5.0f)
  , minScale(0.9f)
  , maxScale(1.1f)
{
}

Environment* Environment::instance = 0;

Environment* Environment::getInstance()
{
  if (!instance)
    instance = new Environment();
  return instance;
}
