#include "buildersegment.h"

#include <cmath>

namespace {

const float kMinimumVectorLengthSquared = 1.0e-10f;

bool isFinite(float value)
{
    return std::isfinite(value);
}

bool isFinite(const glm::vec3& value)
{
    return isFinite(value.x) && isFinite(value.y) && isFinite(value.z);
}

glm::vec3 safeDirection(const glm::vec3& value, const glm::vec3& fallback)
{
    if(!isFinite(value) || glm::dot(value, value) <= kMinimumVectorLengthSquared) {
        return fallback;
    }
    return glm::normalize(value);
}

void buildEntryFrame(const BuilderPose& pose,
                     glm::vec3& right,
                     glm::vec3& up,
                     glm::vec3& forward)
{
    forward = safeDirection(pose.direction, glm::vec3(0.f, 0.f, -1.f));

    right = pose.lateral - forward*glm::dot(pose.lateral, forward);
    if(!isFinite(right) || glm::dot(right, right) <= kMinimumVectorLengthSquared) {
        const glm::vec3 reference = std::fabs(forward.y) < 0.99f
            ? glm::vec3(0.f, 1.f, 0.f)
            : glm::vec3(0.f, 0.f, 1.f);
        right = glm::cross(forward, reference);
    }
    right = glm::normalize(right);
    up = glm::normalize(glm::cross(right, forward));
}

glm::vec3 toWorldVector(const glm::vec3& local,
                        const glm::vec3& right,
                        const glm::vec3& up,
                        const glm::vec3& forward)
{
    return right*local.x + up*local.y + forward*local.z;
}

float smoothStep(float parameter)
{
    return parameter*parameter*(3.f-2.f*parameter);
}

}

BuilderPose::BuilderPose()
    : position(0.f),
      direction(0.f, 0.f, -1.f),
      lateral(1.f, 0.f, 0.f),
      rollDegrees(0.f)
{
}

BuilderPose::BuilderPose(const glm::vec3& positionValue,
                         const glm::vec3& directionValue,
                         const glm::vec3& lateralValue,
                         float rollValue)
    : position(positionValue),
      direction(directionValue),
      lateral(lateralValue),
      rollDegrees(rollValue)
{
}

BuilderSegment::BuilderSegment(const QUuid& idValue)
    : mId(idValue.isNull() ? QUuid::createUuid() : idValue),
      mEndOffset(0.f, 0.f, 10.f),
      mEndDirection(0.f, 0.f, 1.f),
      mStartHandleLength(10.f/3.f),
      mEndHandleLength(10.f/3.f),
      mEndRollDegrees(0.f)
{
}

QUuid BuilderSegment::id() const
{
    return mId;
}

glm::vec3 BuilderSegment::endOffset() const
{
    return mEndOffset;
}

glm::vec3 BuilderSegment::endDirection() const
{
    return mEndDirection;
}

float BuilderSegment::directionDegrees() const
{
    return std::atan2(mEndDirection.x, mEndDirection.z)*180.f/F_PI;
}

float BuilderSegment::horizontalLength() const
{
    const float horizontalChord = std::sqrt(mEndOffset.x*mEndOffset.x
                                            +mEndOffset.z*mEndOffset.z);
    const float turnRadians = std::fabs(directionDegrees()*F_PI/180.f);
    if(turnRadians < 1.0e-4f) return horizontalChord;

    const float halfAngleSine = std::sin(turnRadians/2.f);
    if(std::fabs(halfAngleSine) < 1.0e-5f) return horizontalChord;
    return horizontalChord*turnRadians/(2.f*halfAngleSine);
}

float BuilderSegment::startHandleLength() const
{
    return mStartHandleLength;
}

float BuilderSegment::endHandleLength() const
{
    return mEndHandleLength;
}

float BuilderSegment::endRollDegrees() const
{
    return mEndRollDegrees;
}

bool BuilderSegment::setEndOffset(const glm::vec3& offset)
{
    if(!isFinite(offset)) return false;
    mEndOffset = offset;
    return true;
}

bool BuilderSegment::setEndDirection(const glm::vec3& direction)
{
    if(!isFinite(direction) || glm::dot(direction, direction) <= kMinimumVectorLengthSquared) {
        return false;
    }
    mEndDirection = glm::normalize(direction);
    return true;
}

bool BuilderSegment::setHandleLengths(float startLength, float endLength)
{
    if(!isFinite(startLength) || !isFinite(endLength) || startLength < 0.f || endLength < 0.f) {
        return false;
    }
    mStartHandleLength = startLength;
    mEndHandleLength = endLength;
    return true;
}

bool BuilderSegment::setEndRollDegrees(float rollDegrees)
{
    if(!isFinite(rollDegrees)) return false;
    mEndRollDegrees = rollDegrees;
    return true;
}

bool BuilderSegment::configureFromEditor(float horizontalLength,
                                         float elevation,
                                         float directionDegrees,
                                         float rollDegrees)
{
    if(!isFinite(horizontalLength)
       || !isFinite(elevation)
       || !isFinite(directionDegrees)
       || !isFinite(rollDegrees)
       || horizontalLength <= 0.f) {
        return false;
    }

    const float turnRadians = directionDegrees*F_PI/180.f;
    float lateralOffset = 0.f;
    float forwardOffset = horizontalLength;
    if(std::fabs(turnRadians) > 1.0e-4f) {
        const float radius = horizontalLength/std::fabs(turnRadians);
        lateralOffset = std::copysign(
            radius*(1.f-std::cos(std::fabs(turnRadians))),
            turnRadians);
        forwardOffset = radius*std::sin(std::fabs(turnRadians));
    }

    // For a circular vertical arc that starts on the entry tangent, the
    // terminal pitch is twice the chord's rise/run angle. Using only the
    // average rise/run angle makes the curve flatten again at its endpoint.
    const float horizontalChord = std::sqrt(lateralOffset*lateralOffset
                                            +forwardOffset*forwardOffset);
    const float pitchRadians = 2.f*std::atan2(elevation, horizontalChord);
    const float pitchCosine = std::cos(pitchRadians);
    const glm::vec3 direction(std::sin(turnRadians)*pitchCosine,
                              std::sin(pitchRadians),
                              std::cos(turnRadians)*pitchCosine);

    const float spatialChord = std::sqrt(horizontalChord*horizontalChord
                                         +elevation*elevation);
    const float tangentAngle = std::acos(glm::clamp(direction.z, -1.f, 1.f));
    float handleLength = spatialChord/3.f;
    if(tangentAngle > 1.0e-4f) {
        const float radius = spatialChord/(2.f*std::sin(tangentAngle/2.f));
        handleLength = 4.f*radius*std::tan(tangentAngle/4.f)/3.f;
    }

    return setEndOffset(glm::vec3(lateralOffset, elevation, forwardOffset))
        && setEndDirection(direction)
        && setHandleLengths(handleLength, handleLength)
        && setEndRollDegrees(rollDegrees);
}

bool BuilderSegment::isValid() const
{
    return !mId.isNull()
        && isFinite(mEndOffset)
        && isFinite(mEndDirection)
        && glm::dot(mEndDirection, mEndDirection) > kMinimumVectorLengthSquared
        && isFinite(mStartHandleLength)
        && isFinite(mEndHandleLength)
        && mStartHandleLength >= 0.f
        && mEndHandleLength >= 0.f
        && isFinite(mEndRollDegrees);
}

BuilderCurve BuilderSegment::curve(const BuilderPose& entryPose) const
{
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 forward;
    buildEntryFrame(entryPose, right, up, forward);

    BuilderCurve result;
    result.start = entryPose.position;
    result.startControl = result.start + forward*mStartHandleLength;
    result.end = result.start + toWorldVector(mEndOffset, right, up, forward);

    const glm::vec3 endDirectionWorld = safeDirection(
        toWorldVector(mEndDirection, right, up, forward), forward);
    result.endControl = result.end - endDirectionWorld*mEndHandleLength;
    result.startRollDegrees = entryPose.rollDegrees;
    result.endRollDegrees = mEndRollDegrees;
    return result;
}

BuilderSample BuilderSegment::sample(const BuilderPose& entryPose, float parameter) const
{
    const float t = glm::clamp(parameter, 0.f, 1.f);
    const float oneMinusT = 1.f-t;
    const BuilderCurve builderCurve = curve(entryPose);

    BuilderSample result;
    result.position = oneMinusT*oneMinusT*oneMinusT*builderCurve.start
        + 3.f*oneMinusT*oneMinusT*t*builderCurve.startControl
        + 3.f*oneMinusT*t*t*builderCurve.endControl
        + t*t*t*builderCurve.end;

    const glm::vec3 derivative = 3.f*oneMinusT*oneMinusT
            *(builderCurve.startControl-builderCurve.start)
        + 6.f*oneMinusT*t*(builderCurve.endControl-builderCurve.startControl)
        + 3.f*t*t*(builderCurve.end-builderCurve.endControl);

    result.direction = safeDirection(derivative, entryPose.direction);
    const float rollParameter = smoothStep(t);
    result.rollDegrees = builderCurve.startRollDegrees
        + rollParameter*(builderCurve.endRollDegrees-builderCurve.startRollDegrees);
    return result;
}
