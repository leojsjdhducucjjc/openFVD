#ifndef LENASSERT_H
#define LENASSERT_H

/*
#    FVD++, an advanced coaster design tool for NoLimits
#    Copyright (C) 2012-2015, Stephan "Lenny" Alt <alt.stephan@web.de>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QErrorMessage>
#include "assert.h"
#include <cmath>

#define F_PI (3.141592653589793f)
#define F_PI_2 (1.570796326794896f)
#define F_PI_4 (0.785398163397448f)

#define F_DEG (57.29577951308232f)
#define F_RAD (0.0174532925199432958f)

#define TO_RAD(angle) ((angle)*F_RAD)
#define TO_DEG(angle) ((angle)*F_DEG)

#define F_G (9.80665f)

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/ext.hpp"

// GLM 1.0 removed the old two-vector angle overload used by FVD++. Keep the
// math explicit and in radians, matching GLM_FORCE_RADIANS above.
inline float fvdVectorAngle(const glm::vec3& a, const glm::vec3& b)
{
    const float aLengthSquared = glm::dot(a, a);
    const float bLengthSquared = glm::dot(b, b);
    if(aLengthSquared <= 1.0e-12f || bLengthSquared <= 1.0e-12f) return 0.0f;

    const float cosine = glm::clamp(
        glm::dot(a, b) / std::sqrt(aLengthSquared * bLengthSquared),
        -1.0f,
        1.0f);
    return std::acos(cosine);
}

#ifdef QT_NO_DEBUG
    #define lenAssert(c) if(!(c)) qCritical("Assertion '%s' in %s line %d failed.", #c, __FILE__, __LINE__);
#else
    #define lenAssert(c) assert(c);
#endif

#endif // LENASSERT_H
