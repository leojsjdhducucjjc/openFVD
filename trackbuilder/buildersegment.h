#ifndef BUILDERSEGMENT_H
#define BUILDERSEGMENT_H

#include <QUuid>

#include "lenassert.h"

// The direct builder stores every segment relative to the pose where that
// segment begins. If an earlier advanced or builder section changes, later
// builder sections therefore remain connected and follow the new entry pose.
struct BuilderPose
{
    BuilderPose();
    BuilderPose(const glm::vec3& position,
                const glm::vec3& direction,
                const glm::vec3& lateral,
                float rollDegrees);

    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 lateral;
    float rollDegrees;
};

struct BuilderCurve
{
    glm::vec3 start;
    glm::vec3 startControl;
    glm::vec3 endControl;
    glm::vec3 end;
    float startRollDegrees;
    float endRollDegrees;
};

struct BuilderSample
{
    glm::vec3 position;
    glm::vec3 direction;
    float rollDegrees;
};

class BuilderSegment
{
public:
    explicit BuilderSegment(const QUuid& id = QUuid());

    QUuid id() const;

    glm::vec3 endOffset() const;
    glm::vec3 endDirection() const;
    float horizontalLength() const;
    float directionDegrees() const;
    float startHandleLength() const;
    float endHandleLength() const;
    float endRollDegrees() const;

    bool setEndOffset(const glm::vec3& offset);
    bool setEndDirection(const glm::vec3& direction);
    bool setHandleLengths(float startLength, float endLength);
    bool setEndRollDegrees(float rollDegrees);
    bool configureFromEditor(float horizontalLength,
                             float elevation,
                             float directionDegrees,
                             float rollDegrees);

    bool isValid() const;
    BuilderCurve curve(const BuilderPose& entryPose) const;
    BuilderSample sample(const BuilderPose& entryPose, float parameter) const;

private:
    QUuid mId;
    glm::vec3 mEndOffset;
    glm::vec3 mEndDirection;
    float mStartHandleLength;
    float mEndHandleLength;
    float mEndRollDegrees;
};

#endif // BUILDERSEGMENT_H
