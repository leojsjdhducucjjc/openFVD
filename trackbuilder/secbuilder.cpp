#include "secbuilder.h"

#include <QDebug>
#include <QVector>

#include <algorithm>
#include <cmath>
#include <limits>

#include "exportfuncs.h"
#include "track.h"

namespace {

const float kMinimumDistance = 1.0e-6f;
const float kMinimumNodeSpacing = 0.01f;
const int kMinimumLookupSamples = 64;
const int kMaximumLookupSamples = 4096;
const int kMaximumGeneratedNodes = 180000;
const int kBuilderPayloadVersion = 1;
const int kMaximumBuilderPayloadSize = 64*1024;

glm::vec3 transportedLateral(const mnode& previous, const glm::vec3& direction)
{
    const glm::vec3 previousDirection = glm::normalize(previous.vDir);
    const glm::vec3 nextDirection = glm::normalize(direction);
    const float cosine = glm::clamp(glm::dot(previousDirection, nextDirection), -1.f, 1.f);

    glm::vec3 lateral = previous.vLat;
    const glm::vec3 axisValue = glm::cross(previousDirection, nextDirection);
    const float axisLengthSquared = glm::dot(axisValue, axisValue);
    if(axisLengthSquared > 1.0e-12f) {
        const glm::vec3 axis = axisValue/std::sqrt(axisLengthSquared);
        lateral = glm::angleAxis(std::acos(cosine), axis)*lateral;
    } else if(cosine < 0.f) {
        // A cubic can contain a cusp. Preserve a usable frame even when two
        // adjacent sampled tangents point in opposite directions.
        glm::vec3 axis = previous.vNorm;
        if(glm::dot(axis, axis) <= 1.0e-12f) axis = previous.vLat;
        lateral = glm::angleAxis(F_PI, glm::normalize(axis))*lateral;
    }

    lateral -= nextDirection*glm::dot(lateral, nextDirection);
    if(glm::dot(lateral, lateral) <= 1.0e-12f) {
        const glm::vec3 reference = std::fabs(nextDirection.y) < 0.99f
            ? glm::vec3(0.f, 1.f, 0.f)
            : glm::vec3(0.f, 0.f, 1.f);
        lateral = glm::cross(nextDirection, reference);
    }
    return glm::normalize(lateral);
}

float curveControlLength(const BuilderCurve& curve)
{
    return glm::distance(curve.start, curve.startControl)
        + glm::distance(curve.startControl, curve.endControl)
        + glm::distance(curve.endControl, curve.end);
}

float parameterAtDistance(const QVector<float>& distances,
                          float targetDistance,
                          int& lookupIndex)
{
    while(lookupIndex+1 < distances.size()
          && distances[lookupIndex+1] < targetDistance) {
        ++lookupIndex;
    }

    if(lookupIndex+1 >= distances.size()) return 1.f;

    const float lowerDistance = distances[lookupIndex];
    const float upperDistance = distances[lookupIndex+1];
    const float interval = upperDistance-lowerDistance;
    const float fraction = interval > kMinimumDistance
        ? (targetDistance-lowerDistance)/interval
        : 0.f;
    return (lookupIndex+glm::clamp(fraction, 0.f, 1.f))/(distances.size()-1.f);
}

void writeFloat(std::stringstream& stream, float value)
{
    writeBytes(&stream, reinterpret_cast<const char*>(&value), sizeof(value));
}

void writeVector(std::stringstream& stream, const glm::vec3& value)
{
    // writeBytes reverses the entire supplied range. Write each component
    // independently so vectors round-trip as X/Y/Z instead of Z/Y/X.
    writeFloat(stream, value.x);
    writeFloat(stream, value.y);
    writeFloat(stream, value.z);
}

std::string builderPayload(const secbuilder& sectionValue)
{
    std::stringstream payload(std::ios::in | std::ios::out | std::ios::binary);
    const BuilderSegment& segment = sectionValue.segment();

    writeQString(&payload, sectionValue.sName);
    writeQString(&payload, segment.id().toString(QUuid::WithoutBraces));
    writeBytes(&payload,
               reinterpret_cast<const char*>(&sectionValue.bSpeed),
               sizeof(sectionValue.bSpeed));
    writeFloat(payload, sectionValue.fVel);
    writeVector(payload, segment.endOffset());
    writeVector(payload, segment.endDirection());
    writeFloat(payload, segment.startHandleLength());
    writeFloat(payload, segment.endHandleLength());
    writeFloat(payload, segment.endRollDegrees());
    return payload.str();
}

template<typename Stream>
void saveBuilderSection(Stream& stream, const secbuilder& sectionValue)
{
    const std::string payload = builderPayload(sectionValue);
    const int payloadSize = static_cast<int>(payload.size());

    stream << "BLD";
    writeBytes(&stream,
               reinterpret_cast<const char*>(&kBuilderPayloadVersion),
               sizeof(kBuilderPayloadVersion));
    writeBytes(&stream,
               reinterpret_cast<const char*>(&payloadSize),
               sizeof(payloadSize));
    if(payloadSize) stream.write(payload.data(), payloadSize);
}

template<typename Stream>
bool loadBuilderSection(Stream& stream,
                        BuilderSegment& segmentValue,
                        QString& nameValue,
                        bool& speedFromEnergyValue,
                        float& velocityValue)
{
    const int payloadVersion = readInt(&stream);
    const int payloadSize = readInt(&stream);
    if(!stream
       || payloadVersion != kBuilderPayloadVersion
       || payloadSize < 0
       || payloadSize > kMaximumBuilderPayloadSize) {
        stream.setstate(std::ios::failbit);
        return false;
    }

    const std::string serializedPayload = readString(&stream, payloadSize);
    if(!stream) return false;

    std::stringstream payload(serializedPayload, std::ios::in | std::ios::binary);
    const int nameLength = readInt(&payload);
    const QString loadedName = readQString(&payload, nameLength);
    const int idLength = readInt(&payload);
    const QString idString = readQString(&payload, idLength);
    const bool loadedSpeedFromEnergy = readBool(&payload);
    const float loadedVelocity = readFloat(&payload);
    const glm::vec3 loadedOffset = readVec3(&payload);
    const glm::vec3 loadedDirection = readVec3(&payload);
    const float loadedStartHandle = readFloat(&payload);
    const float loadedEndHandle = readFloat(&payload);
    const float loadedEndRoll = readFloat(&payload);

    const QUuid loadedId(idString);
    BuilderSegment loadedSegment(loadedId);
    const bool fieldsAreValid = payload
        && payload.peek() == std::char_traits<char>::eof()
        && !loadedId.isNull()
        && std::isfinite(loadedVelocity)
        && loadedVelocity >= 0.1f
        && loadedVelocity <= 1000.f
        && loadedSegment.setEndOffset(loadedOffset)
        && loadedSegment.setEndDirection(loadedDirection)
        && loadedSegment.setHandleLengths(loadedStartHandle, loadedEndHandle)
        && loadedSegment.setEndRollDegrees(loadedEndRoll)
        && loadedSegment.isValid();
    if(!fieldsAreValid) {
        stream.setstate(std::ios::failbit);
        return false;
    }

    segmentValue = loadedSegment;
    nameValue = loadedName;
    speedFromEnergyValue = loadedSpeedFromEnergy;
    velocityValue = loadedVelocity;
    return true;
}

}

secbuilder::secbuilder(track* parentValue, mnode* first)
    : section(parentValue, builder, first),
      mRevision(0)
{
    bOrientation = QUATERNION;
    bArgument = DISTANCE;
    bSpeed = false;
    fVel = first->fVel > 0.1f ? first->fVel : 10.f;
    length = 0.f;
    sName = QString("Builder");
    mSegment.setEndRollDegrees(first->fRoll);
}

BuilderSegment& secbuilder::segment()
{
    return mSegment;
}

const BuilderSegment& secbuilder::segment() const
{
    return mSegment;
}

quint64 secbuilder::revision() const
{
    return mRevision;
}

BuilderPose secbuilder::entryPose() const
{
    const mnode& entry = lNodes.first();
    return BuilderPose(entry.vPos, entry.vDir, entry.vLat, entry.fRoll);
}

int secbuilder::updateSection(int node)
{
    Q_UNUSED(node);
    ++mRevision;

    // A Builder preview may contain thousands of physics samples. Removing
    // index 1 repeatedly shifts the rest of the QVector each time and makes a
    // drag rebuild quadratic. Keep the entry node with one resize instead.
    if(lNodes.size() > 1) lNodes.resize(1);
    if(lNodes.isEmpty() || !mSegment.isValid()) {
        length = 0.f;
        return 0;
    }

    lNodes[0].updateNorm();
    const BuilderPose pose = entryPose();
    const BuilderCurve curve = mSegment.curve(pose);
    const int lookupSampleCount = glm::clamp(
        static_cast<int>(std::ceil(curveControlLength(curve)/0.1f)),
        kMinimumLookupSamples,
        kMaximumLookupSamples);

    QVector<float> distances(lookupSampleCount+1, 0.f);
    glm::vec3 previousPosition = curve.start;
    for(int i = 1; i <= lookupSampleCount; ++i) {
        const glm::vec3 position = mSegment.sample(pose, i/static_cast<float>(lookupSampleCount)).position;
        distances[i] = distances[i-1]+glm::distance(previousPosition, position);
        previousPosition = position;
    }

    const float curveLength = distances.last();
    if(curveLength <= kMinimumDistance) {
        length = 0.f;
        return 0;
    }

    const float referenceVelocity = bSpeed
        ? glm::max(lNodes.first().fVel, 0.1f)
        : glm::max(fVel, 0.1f);
    const float nodeSpacing = glm::max(referenceVelocity/F_HZ, kMinimumNodeSpacing);
    const int generatedNodeCount = glm::clamp(
        static_cast<int>(std::ceil(curveLength/nodeSpacing)),
        1,
        kMaximumGeneratedNodes);
    if(generatedNodeCount == kMaximumGeneratedNodes
       && curveLength/nodeSpacing > kMaximumGeneratedNodes) {
        qWarning("Builder section sampling capped at %d nodes", kMaximumGeneratedNodes);
    }

    lNodes.reserve(generatedNodeCount+1);

    int lookupIndex = 0;
    for(int i = 1; i <= generatedNodeCount; ++i) {
        const float targetDistance = curveLength*i/generatedNodeCount;
        const float parameter = parameterAtDistance(distances, targetDistance, lookupIndex);
        const BuilderSample sample = mSegment.sample(pose, parameter);

        lNodes.append(lNodes.last());
        mnode* current = &lNodes.last();
        mnode* previous = &lNodes[lNodes.size()-2];

        current->vPos = sample.position;
        current->vDir = glm::normalize(sample.direction);
        current->vLat = transportedLateral(*previous, current->vDir);
        current->updateRoll();
        current->setRoll(sample.rollDegrees-current->fRoll);
        current->updateNorm();

        current->fHeartDistFromLast = glm::distance(current->vPos, previous->vPos);
        current->fTotalHeartLength = previous->fTotalHeartLength+current->fHeartDistFromLast;
        current->fDistFromLast = glm::distance(
            current->vPosHeart(parent->fHeart), previous->vPosHeart(parent->fHeart));
        current->fTotalLength = previous->fTotalLength+current->fDistFromLast;
        current->fRollSpeed = (sample.rollDegrees-previous->fRoll)*F_HZ;
        current->fSmoothSpeed = 0.f;
        current->smoothNormal = 0.f;
        current->smoothLateral = 0.f;

        if(bSpeed) {
            current->fEnergy = previous->fEnergy
                -(previous->fVel*previous->fVel*previous->fVel/F_HZ*parent->fResistance);
            const float availableEnergy = current->fEnergy
                -F_G*(current->vPosHeart(parent->fHeart*0.9f).y
                     +current->fTotalLength*parent->fFriction);
            current->fVel = availableEnergy > 0.f ? std::sqrt(2.f*availableEnergy) : 0.1f;
        } else {
            current->fVel = referenceVelocity;
            current->fEnergy = 0.5f*current->fVel*current->fVel
                +F_G*(current->vPosHeart(parent->fHeart*0.9f).y
                     +current->fTotalLength*parent->fFriction);
        }

        calcDirFromLast(lNodes.size()-1);
        const float pitchCosine = std::cos(std::fabs(current->getPitch())*F_PI/180.f);
        current->fAngleFromLast = std::sqrt(
            pitchCosine*pitchCosine*current->fYawFromLast*current->fYawFromLast
            +current->fPitchFromLast*current->fPitchFromLast);

        glm::vec3 forceVector(0.f, 1.f, 0.f);
        if(current->fHeartDistFromLast > kMinimumDistance) {
            const float normalAngle = F_PI/180.f
                *(-current->fPitchFromLast*std::cos(current->fRoll*F_PI/180.f)
                  -pitchCosine*current->fYawFromLast*std::sin(current->fRoll*F_PI/180.f));
            const float lateralAngle = F_PI/180.f
                *(current->fPitchFromLast*std::sin(current->fRoll*F_PI/180.f)
                  -pitchCosine*current->fYawFromLast*std::cos(current->fRoll*F_PI/180.f));
            const float accelerationScale = current->fVel*current->fVel
                /(F_G*current->fHeartDistFromLast);
            forceVector += lateralAngle*accelerationScale*current->vLat
                +normalAngle*accelerationScale*current->vNorm;
        }
        current->forceNormal = -glm::dot(forceVector, glm::normalize(current->vNorm));
        current->forceLateral = -glm::dot(forceVector, glm::normalize(current->vLat));
    }

    length = lNodes.last().fTotalLength-lNodes.first().fTotalLength;
    return 0;
}

void secbuilder::saveSection(std::fstream& file)
{
    saveBuilderSection(file, *this);
}

void secbuilder::loadSection(std::fstream& file)
{
    loadBuilderSection(file, mSegment, sName, bSpeed, fVel);
}

void secbuilder::legacyLoadSection(std::fstream& file)
{
    loadBuilderSection(file, mSegment, sName, bSpeed, fVel);
}

void secbuilder::saveSection(std::stringstream& file)
{
    saveBuilderSection(file, *this);
}

void secbuilder::loadSection(std::stringstream& file)
{
    loadBuilderSection(file, mSegment, sName, bSpeed, fVel);
}

float secbuilder::getMaxArgument()
{
    return length;
}

bool secbuilder::isLockable(func* function)
{
    Q_UNUSED(function);
    return false;
}

bool secbuilder::isInFunction(int index, subfunc* function)
{
    Q_UNUSED(index);
    Q_UNUSED(function);
    return false;
}
