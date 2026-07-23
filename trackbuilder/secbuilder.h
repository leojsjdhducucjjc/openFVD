#ifndef SECBUILDER_H
#define SECBUILDER_H

#include "buildersegment.h"
#include "section.h"

// A directly manipulated track piece that participates in FVD++'s existing
// section pipeline. BuilderSegment owns the editable curve parameters;
// secbuilder turns those parameters into the complete mnode stream consumed by
// rendering, graphs, smoothing, POV, and exporters.
class secbuilder : public section
{
public:
    secbuilder(track* parent, mnode* first);

    BuilderSegment& segment();
    const BuilderSegment& segment() const;
    quint64 revision() const;

    virtual int updateSection(int node = 0);

    // BLD payloads have an independent version and byte length so future
    // builder fields can evolve without changing legacy section readers.
    virtual void saveSection(std::fstream& file);
    virtual void loadSection(std::fstream& file);
    virtual void legacyLoadSection(std::fstream& file);
    virtual void saveSection(std::stringstream& file);
    virtual void loadSection(std::stringstream& file);

    virtual float getMaxArgument();
    virtual bool isLockable(func* function);
    virtual bool isInFunction(int index, subfunc* function);

private:
    BuilderPose entryPose() const;
    BuilderSegment mSegment;
    quint64 mRevision;
};

#endif // SECBUILDER_H
