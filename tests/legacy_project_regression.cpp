#include <QtTest>

#include <QApplication>
#include <QFile>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QGLFormat>
#include <QLabel>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTreeWidgetItem>

#include <cmath>
#include <limits>
#include <sstream>

#include "buildersegment.h"
#include "exportfuncs.h"
#include "glviewwidget.h"
#include "mainwindow.h"
#include "projectwidget.h"
#include "saver.h"
#include "secbuilder.h"
#include "sectionhandler.h"
#include "secstraight.h"
#include "trackhandler.h"
#include "trackwidget.h"
#include "undohandler.h"

extern glViewWidget* glView;

namespace {

QString decodeFixture(const QString& fixtureName, const QString& destination)
{
    const QString encodedPath = QFINDTESTDATA(qPrintable(QString("fixtures/%1").arg(fixtureName)));
    if(encodedPath.isEmpty()) return QString();

    QFile encoded(encodedPath);
    if(!encoded.open(QIODevice::ReadOnly)) return QString();

    const QByteArray contents = QByteArray::fromBase64(encoded.readAll().trimmed());
    if(contents.isEmpty()) return QString();

    QFile decoded(destination);
    if(!decoded.open(QIODevice::WriteOnly | QIODevice::Truncate)) return QString();
    if(decoded.write(contents) != contents.size()) return QString();
    decoded.close();
    return destination;
}

bool nearlyEqual(float left, float right)
{
    return std::fabs(left-right) < 0.0001f;
}

bool nearlyEqual(const glm::vec3& left, const glm::vec3& right)
{
    return nearlyEqual(left.x, right.x)
        && nearlyEqual(left.y, right.y)
        && nearlyEqual(left.z, right.z);
}

QString signedFixed(float value)
{
    QString result = QString::number(value, 'f', 3);
    if(!result.startsWith('-')) result.prepend('+');
    return result;
}

QImage renderViewport(glViewWidget* view)
{
    view->makeCurrent();
    view->paintGL();
    glFinish();

    GLint viewport[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    QImage image(viewport[2], viewport[3], QImage::Format_RGBA8888);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(viewport[0],
                 viewport[1],
                 viewport[2],
                 viewport[3],
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 image.bits());
    return image.mirrored();
}

}

class LegacyProjectRegression : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void serializationPrimitivesRoundTrip();
    void builderSegmentDefaultsToStraight();
    void builderSegmentUsesEntryRelativeFrame();
    void builderSegmentSupportsTangentsAndBanking();
    void builderEditorGeometryUsesSmoothArcEndpoints();
    void builderPiecesJoinWithoutEndpointKinks();
    void builderSegmentRejectsInvalidEdits();
    void builderSectionProducesCompleteTrackNodes();
    void builderRepeatedPreviewUpdatesStayResponsive();
    void builderSectionFollowsAdvancedSections();
    void builderSectionPersistenceRoundTrip();
    void builderUiPreviewCommitCancelAndRedo();
    void cameraUsesHoldToDrag();
    void builderSectionRejectsInvalidPayload_data();
    void builderSectionRejectsInvalidPayload();
    void legacyFixturesLoad_data();
    void legacyFixturesLoad();
    void projectSaveAndReloadRoundTrip();

private:
    MainWindow* window = NULL;
};

void LegacyProjectRegression::initTestCase()
{
    window = new MainWindow();
    window->show();

    QVERIFY2(QTest::qWaitForWindowExposed(window), "The FVD++ main window could not be exposed");
    QTRY_VERIFY_WITH_TIMEOUT(glView != NULL && glView->isValid(), 5000);
    QTRY_COMPARE_WITH_TIMEOUT(window->project->trackList.size(), 1, 5000);
}

void LegacyProjectRegression::cleanupTestCase()
{
    delete window;
    window = NULL;
}

void LegacyProjectRegression::serializationPrimitivesRoundTrip()
{
    std::stringstream stream;
    const int expectedInt = 0x12345678;
    const float expectedFloat = -123.25f;
    const bool expectedBool = true;
    const QString expectedString = QString::fromUtf8("Coaster – 日本語");

    writeBytes(&stream, reinterpret_cast<const char*>(&expectedInt), sizeof(expectedInt));
    writeBytes(&stream, reinterpret_cast<const char*>(&expectedFloat), sizeof(expectedFloat));
    writeBytes(&stream, reinterpret_cast<const char*>(&expectedBool), sizeof(expectedBool));
    writeQString(&stream, expectedString);

    QCOMPARE(readInt(&stream), expectedInt);
    QVERIFY(nearlyEqual(readFloat(&stream), expectedFloat));
    QCOMPARE(readBool(&stream), expectedBool);
    const int stringLength = readInt(&stream);
    QCOMPARE(readQString(&stream, stringLength), expectedString);
    QVERIFY(stream.good());
}

void LegacyProjectRegression::builderSegmentDefaultsToStraight()
{
    const QUuid id("{26d3b40a-383a-4ff4-a717-2430a3211224}");
    const BuilderSegment segment(id);
    const BuilderPose entry;

    QCOMPARE(segment.id(), id);
    QVERIFY(segment.isValid());

    const BuilderCurve curve = segment.curve(entry);
    QVERIFY(nearlyEqual(curve.start, glm::vec3(0.f, 0.f, 0.f)));
    QVERIFY(nearlyEqual(curve.end, glm::vec3(0.f, 0.f, -10.f)));
    QVERIFY(nearlyEqual(curve.startControl, glm::vec3(0.f, 0.f, -10.f/3.f)));
    QVERIFY(nearlyEqual(curve.endControl, glm::vec3(0.f, 0.f, -20.f/3.f)));

    const BuilderSample midpoint = segment.sample(entry, 0.5f);
    QVERIFY(nearlyEqual(midpoint.position, glm::vec3(0.f, 0.f, -5.f)));
    QVERIFY(nearlyEqual(midpoint.direction, glm::vec3(0.f, 0.f, -1.f)));
    QVERIFY(nearlyEqual(midpoint.rollDegrees, 0.f));
}

void LegacyProjectRegression::builderSegmentUsesEntryRelativeFrame()
{
    BuilderSegment segment;
    QVERIFY(segment.setEndOffset(glm::vec3(2.f, 3.f, 10.f)));

    const BuilderPose entry(glm::vec3(5.f, 7.f, 11.f),
                            glm::vec3(1.f, 0.f, 0.f),
                            glm::vec3(0.f, 0.f, 1.f),
                            12.f);
    const BuilderCurve curve = segment.curve(entry);

    QVERIFY(nearlyEqual(curve.start, glm::vec3(5.f, 7.f, 11.f)));
    QVERIFY(nearlyEqual(curve.end, glm::vec3(15.f, 10.f, 13.f)));
    QVERIFY(nearlyEqual(segment.sample(entry, 0.f).direction, glm::vec3(1.f, 0.f, 0.f)));
}

void LegacyProjectRegression::builderSegmentSupportsTangentsAndBanking()
{
    BuilderSegment segment;
    QVERIFY(segment.setEndOffset(glm::vec3(4.f, 2.f, 12.f)));
    QVERIFY(segment.setEndDirection(glm::vec3(1.f, 0.f, 1.f)));
    QVERIFY(segment.setHandleLengths(4.f, 5.f));
    QVERIFY(segment.setEndRollDegrees(70.f));

    const BuilderPose entry(glm::vec3(0.f),
                            glm::vec3(0.f, 0.f, -1.f),
                            glm::vec3(1.f, 0.f, 0.f),
                            10.f);

    const BuilderSample start = segment.sample(entry, 0.f);
    const BuilderSample midpoint = segment.sample(entry, 0.5f);
    const BuilderSample end = segment.sample(entry, 1.f);

    QVERIFY(nearlyEqual(start.position, entry.position));
    QVERIFY(nearlyEqual(start.direction, entry.direction));
    QVERIFY(nearlyEqual(start.rollDegrees, 10.f));
    QVERIFY(nearlyEqual(end.position, glm::vec3(4.f, 2.f, -12.f)));
    QVERIFY(nearlyEqual(end.direction, glm::normalize(glm::vec3(1.f, 0.f, -1.f))));
    QVERIFY(nearlyEqual(end.rollDegrees, 70.f));
    QVERIFY(nearlyEqual(midpoint.rollDegrees, 40.f));
}

void LegacyProjectRegression::builderEditorGeometryUsesSmoothArcEndpoints()
{
    BuilderSegment segment;
    QVERIFY(segment.configureFromEditor(20.f, 4.f, 90.f, 35.f));
    QVERIFY(nearlyEqual(segment.horizontalLength(), 20.f));
    QVERIFY(nearlyEqual(segment.directionDegrees(), 90.f));

    const float radius = 20.f/(F_PI/2.f);
    QVERIFY(nearlyEqual(segment.endOffset(), glm::vec3(radius, 4.f, radius)));
    const float horizontalChord = std::sqrt(2.f*radius*radius);
    const float pitchRadians = 2.f*std::atan2(4.f, horizontalChord);
    const glm::vec3 expectedDirection(std::cos(pitchRadians),
                                      std::sin(pitchRadians),
                                      0.f);
    QVERIFY(nearlyEqual(segment.endDirection(), expectedDirection));
    QVERIFY(nearlyEqual(segment.endRollDegrees(), 35.f));

    const BuilderPose pose;
    const BuilderSample start = segment.sample(pose, 0.f);
    const BuilderSample end = segment.sample(pose, 1.f);
    QVERIFY(nearlyEqual(start.direction, pose.direction));
    QVERIFY(nearlyEqual(end.direction, expectedDirection));

    BuilderSegment verticalArc;
    QVERIFY(verticalArc.configureFromEditor(20.f, 5.f, 0.f, 0.f));
    const float expectedEndPitch = 2.f*std::atan2(5.f, 20.f);
    QVERIFY(nearlyEqual(verticalArc.endDirection(),
                        glm::vec3(0.f,
                                  std::sin(expectedEndPitch),
                                  std::cos(expectedEndPitch))));
    float previousPitch = -std::numeric_limits<float>::infinity();
    for(int i = 0; i <= 20; ++i) {
        const BuilderSample sample = verticalArc.sample(pose, i/20.f);
        const float pitch = std::asin(glm::clamp(sample.direction.y, -1.f, 1.f));
        QVERIFY2(pitch+1.0e-5f >= previousPitch,
                 "Vertical Builder arc flattened before its endpoint");
        previousPitch = pitch;
    }
    QVERIFY(nearlyEqual(previousPitch, expectedEndPitch));
}

void LegacyProjectRegression::builderPiecesJoinWithoutEndpointKinks()
{
    track* coaster = window->project->trackList.first()->trackData;
    secbuilder first(coaster, coaster->anchorNode);
    QVERIFY(first.segment().configureFromEditor(20.f, 3.f, 60.f, 25.f));
    first.updateSection();
    QVERIFY(first.lNodes.size() > 2);

    secbuilder second(coaster, &first.lNodes.last());
    QVERIFY(second.segment().configureFromEditor(15.f, -1.f, -30.f, 25.f));
    second.updateSection();
    QVERIFY(second.lNodes.size() > 2);

    const mnode& outgoing = first.lNodes.last();
    const mnode& incoming = second.lNodes.first();
    QVERIFY(nearlyEqual(outgoing.vPos, incoming.vPos));
    QVERIFY(nearlyEqual(outgoing.vDir, incoming.vDir));
    QVERIFY(nearlyEqual(outgoing.vLat, incoming.vLat));
    QVERIFY(nearlyEqual(outgoing.fRoll, incoming.fRoll));
    QVERIFY(glm::dot(glm::normalize(outgoing.vDir),
                     glm::normalize(second.lNodes[1].vDir)) > 0.999f);
}

void LegacyProjectRegression::builderSegmentRejectsInvalidEdits()
{
    BuilderSegment segment;
    const float notANumber = std::numeric_limits<float>::quiet_NaN();

    QVERIFY(!segment.setEndOffset(glm::vec3(notANumber, 0.f, 1.f)));
    QVERIFY(!segment.setEndDirection(glm::vec3(0.f)));
    QVERIFY(!segment.setHandleLengths(-1.f, 2.f));
    QVERIFY(!segment.setEndRollDegrees(notANumber));

    QVERIFY(segment.isValid());
    QVERIFY(nearlyEqual(segment.endOffset(), glm::vec3(0.f, 0.f, 10.f)));
    QVERIFY(nearlyEqual(segment.endDirection(), glm::vec3(0.f, 0.f, 1.f)));
    QVERIFY(nearlyEqual(segment.startHandleLength(), 10.f/3.f));
    QVERIFY(nearlyEqual(segment.endHandleLength(), 10.f/3.f));
    QVERIFY(nearlyEqual(segment.endRollDegrees(), 0.f));
}

void LegacyProjectRegression::builderSectionProducesCompleteTrackNodes()
{
    glView->makeCurrent();
    window->project->init();
    track* coaster = window->project->trackList.first()->trackData;
    coaster->newSection(builder);
    glView->doneCurrent();

    secbuilder* builderSection = dynamic_cast<secbuilder*>(coaster->lSections.first());
    QVERIFY(builderSection != NULL);
    QCOMPARE(builderSection->type, builder);
    QVERIFY(builderSection->lNodes.size() > 100);
    QVERIFY(nearlyEqual(builderSection->length, 10.f));
    QVERIFY(nearlyEqual(builderSection->lNodes.first().vPos, coaster->anchorNode->vPos));
    QVERIFY(nearlyEqual(builderSection->lNodes.last().vPos, glm::vec3(0.f, 0.f, -10.f)));
    QVERIFY(nearlyEqual(builderSection->lNodes.last().vDir, glm::vec3(0.f, 0.f, -1.f)));

    float previousLength = builderSection->lNodes.first().fTotalLength;
    for(int i = 1; i < builderSection->lNodes.size(); ++i) {
        const mnode& node = builderSection->lNodes[i];
        QVERIFY(std::isfinite(node.vPos.x));
        QVERIFY(std::isfinite(node.vPos.y));
        QVERIFY(std::isfinite(node.vPos.z));
        QVERIFY(nearlyEqual(glm::length(node.vDir), 1.f));
        QVERIFY(nearlyEqual(glm::length(node.vLat), 1.f));
        QVERIFY(node.fTotalLength > previousLength);
        QVERIFY(node.fDistFromLast > 0.f);
        previousLength = node.fTotalLength;
    }
}

void LegacyProjectRegression::builderRepeatedPreviewUpdatesStayResponsive()
{
    glView->makeCurrent();
    window->project->init();
    track* coaster = window->project->trackList.first()->trackData;
    secbuilder preview(coaster, coaster->anchorNode);
    QVERIFY(preview.segment().configureFromEditor(100.f, 8.f, 35.f, 20.f));
    preview.updateSection();
    QVERIFY(preview.lNodes.size() > 5000);

    const quint64 firstRevision = preview.revision();
    QElapsedTimer timer;
    timer.start();
    for(int i = 0; i < 8; ++i) {
        QVERIFY(preview.segment().configureFromEditor(100.f+i,
                                                       8.f+i*0.25f,
                                                       35.f-i,
                                                       20.f+i));
        preview.updateSection();
        QVERIFY(preview.lNodes.size() > 5000);
    }
    const qint64 elapsed = timer.elapsed();
    glView->doneCurrent();

    QVERIFY(preview.revision() >= firstRevision+8);
    QVERIFY2(elapsed < 5000,
             qPrintable(QString("Eight long Builder rebuilds took %1 ms").arg(elapsed)));
}

void LegacyProjectRegression::builderSectionFollowsAdvancedSections()
{
    glView->makeCurrent();
    window->project->init();
    track* coaster = window->project->trackList.first()->trackData;
    coaster->newSection(straight);
    coaster->newSection(builder);
    coaster->newSection(straight);

    secstraight* leading = dynamic_cast<secstraight*>(coaster->lSections[0]);
    secbuilder* builderSection = dynamic_cast<secbuilder*>(coaster->lSections[1]);
    secstraight* trailing = dynamic_cast<secstraight*>(coaster->lSections[2]);
    QVERIFY(leading != NULL);
    QVERIFY(builderSection != NULL);
    QVERIFY(trailing != NULL);

    QVERIFY(builderSection->segment().setEndOffset(glm::vec3(4.f, 2.f, 12.f)));
    QVERIFY(builderSection->segment().setEndDirection(glm::vec3(0.5f, 0.2f, 1.f)));
    QVERIFY(builderSection->segment().setHandleLengths(4.f, 5.f));
    QVERIFY(builderSection->segment().setEndRollDegrees(30.f));
    coaster->updateTrack(1, 0);

    const glm::vec3 originalBuilderStart = builderSection->lNodes.first().vPos;
    const glm::vec3 originalBuilderEnd = builderSection->lNodes.last().vPos;
    QVERIFY(nearlyEqual(builderSection->lNodes.last().fRoll, 30.f));
    QVERIFY(nearlyEqual(trailing->lNodes.first().vPos, originalBuilderEnd));
    QVERIFY(nearlyEqual(trailing->lNodes.first().vDir, builderSection->lNodes.last().vDir));

    leading->rollFunc->setMaxArgument(15.f);
    coaster->updateTrack(0, 0);
    glView->doneCurrent();

    QVERIFY(nearlyEqual(builderSection->lNodes.first().vPos, leading->lNodes.last().vPos));
    QVERIFY(nearlyEqual(builderSection->lNodes.first().vDir, leading->lNodes.last().vDir));
    QVERIFY(nearlyEqual(trailing->lNodes.first().vPos, builderSection->lNodes.last().vPos));
    QVERIFY(nearlyEqual(trailing->lNodes.first().vDir, builderSection->lNodes.last().vDir));
    QVERIFY(glm::distance(builderSection->lNodes.first().vPos, originalBuilderStart) > 4.9f);
    QVERIFY(glm::distance(builderSection->lNodes.last().vPos, originalBuilderEnd) > 4.9f);
}

void LegacyProjectRegression::builderSectionPersistenceRoundTrip()
{
    const QString expectedName = QString::fromUtf8("Direct Builder – 保存");
    const glm::vec3 expectedOffset(4.25f, -2.5f, 17.75f);
    const glm::vec3 expectedDirection = glm::normalize(glm::vec3(-0.4f, 0.3f, 1.f));

    glView->makeCurrent();
    window->project->init();
    trackHandler* originalHandler = window->project->trackList.first();
    originalHandler->trackWidgetItem->addSection(straight);
    originalHandler->trackWidgetItem->addSection(builder);
    originalHandler->trackWidgetItem->addSection(straight);

    track* originalTrack = originalHandler->trackData;
    secbuilder* originalBuilder = dynamic_cast<secbuilder*>(originalTrack->lSections[1]);
    QVERIFY(originalBuilder != NULL);
    QVERIFY(originalBuilder->segment().setEndOffset(expectedOffset));
    QVERIFY(originalBuilder->segment().setEndDirection(expectedDirection));
    QVERIFY(originalBuilder->segment().setHandleLengths(6.5f, 3.25f));
    QVERIFY(originalBuilder->segment().setEndRollDegrees(-47.5f));
    originalBuilder->bSpeed = true;
    originalBuilder->fVel = 22.75f;
    originalHandler->trackWidgetItem->sectionList[2]->listItem->setText(1, expectedName);
    originalTrack->updateTrack(1, 0);

    const QUuid expectedId = originalBuilder->segment().id();
    const glm::vec3 expectedStart = originalBuilder->lNodes.first().vPos;
    const glm::vec3 expectedEnd = originalBuilder->lNodes.last().vPos;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString projectPath = directory.filePath("builder-round-trip.fvd");
    saver writer(projectPath, window->project, window);
    QCOMPARE(writer.doSave(), QString("Project Saved!"));

    QFile saved(projectPath);
    QVERIFY(saved.open(QIODevice::ReadOnly));
    const QByteArray serializedProject = saved.readAll();
    QCOMPARE(serializedProject.left(8), QByteArray("FVDv0.77"));
    QVERIFY(serializedProject.contains("BLD"));
    saved.close();

    saver reader(projectPath, window->project, window);
    QCOMPARE(reader.doLoad(), QString("Load Successfull."));
    glView->doneCurrent();

    QCOMPARE(window->project->trackList.size(), 1);
    track* restoredTrack = window->project->trackList.first()->trackData;
    QCOMPARE(restoredTrack->lSections.size(), 3);
    QCOMPARE(restoredTrack->lSections[0]->type, straight);
    QCOMPARE(restoredTrack->lSections[1]->type, builder);
    QCOMPARE(restoredTrack->lSections[2]->type, straight);

    secbuilder* restoredBuilder = dynamic_cast<secbuilder*>(restoredTrack->lSections[1]);
    QVERIFY(restoredBuilder != NULL);
    QCOMPARE(restoredBuilder->sName, expectedName);
    QCOMPARE(restoredBuilder->segment().id(), expectedId);
    QVERIFY(nearlyEqual(restoredBuilder->segment().endOffset(), expectedOffset));
    QVERIFY(nearlyEqual(restoredBuilder->segment().endDirection(), expectedDirection));
    QVERIFY(nearlyEqual(restoredBuilder->segment().startHandleLength(), 6.5f));
    QVERIFY(nearlyEqual(restoredBuilder->segment().endHandleLength(), 3.25f));
    QVERIFY(nearlyEqual(restoredBuilder->segment().endRollDegrees(), -47.5f));
    QCOMPARE(restoredBuilder->bSpeed, true);
    QVERIFY(nearlyEqual(restoredBuilder->fVel, 22.75f));
    QVERIFY(nearlyEqual(restoredBuilder->lNodes.first().vPos, expectedStart));
    QVERIFY(nearlyEqual(restoredBuilder->lNodes.last().vPos, expectedEnd));
    QVERIFY(nearlyEqual(restoredTrack->lSections[0]->lNodes.last().vPos,
                        restoredBuilder->lNodes.first().vPos));
    QVERIFY(nearlyEqual(restoredTrack->lSections[2]->lNodes.first().vPos,
                        restoredBuilder->lNodes.last().vPos));
}

void LegacyProjectRegression::builderUiPreviewCommitCancelAndRedo()
{
    glView->makeCurrent();
    window->project->init();
    trackHandler* handler = window->project->trackList.first();
    trackWidget* editor = handler->trackWidgetItem;
    track* coaster = handler->trackData;
    window->openTab(handler);
    QApplication::processEvents();

    QDoubleSpinBox* lengthBox = window->findChild<QDoubleSpinBox*>("builderLengthBox");
    QDoubleSpinBox* elevationBox = window->findChild<QDoubleSpinBox*>("builderElevationBox");
    QDoubleSpinBox* directionBox = window->findChild<QDoubleSpinBox*>("builderDirectionBox");
    QDoubleSpinBox* bankBox = window->findChild<QDoubleSpinBox*>("builderBankBox");
    QPushButton* commitButton = window->findChild<QPushButton*>("builderCommitButton");
    QPushButton* cancelButton = window->findChild<QPushButton*>("builderCancelButton");
    QPushButton* nextButton = window->findChild<QPushButton*>("builderNextButton");
    QLabel* editingLabel = window->findChild<QLabel*>("builderEditingStatus");
    QLabel* modeLabel = window->findChild<QLabel*>("builderModeBadge");
    QLabel* yawLabel = window->findChild<QLabel*>("infoDirLabel");
    QLabel* pitchLabel = window->findChild<QLabel*>("infoPitchLabel");
    QLabel* rollLabel = window->findChild<QLabel*>("infoRollLabel");
    QLabel* normalForceLabel = window->findChild<QLabel*>("infoNormalLabel");
    QLabel* lateralForceLabel = window->findChild<QLabel*>("infoLateralLabel");
    QLabel* lengthHandleLabel = window->findChild<QLabel*>("builderLengthHandleLabel");
    QLabel* directionHandleLabel = window->findChild<QLabel*>("builderDirectionHandleLabel");
    QLabel* elevationHandleLabel = window->findChild<QLabel*>("builderElevationHandleLabel");
    QLabel* bankHandleLabel = window->findChild<QLabel*>("builderBankHandleLabel");
    QVERIFY(lengthBox != NULL);
    QVERIFY(elevationBox != NULL);
    QVERIFY(directionBox != NULL);
    QVERIFY(bankBox != NULL);
    QVERIFY(commitButton != NULL);
    QVERIFY(cancelButton != NULL);
    QVERIFY(nextButton != NULL);
    QVERIFY(editingLabel != NULL);
    QVERIFY(modeLabel != NULL);
    QVERIFY(yawLabel != NULL);
    QVERIFY(pitchLabel != NULL);
    QVERIFY(rollLabel != NULL);
    QVERIFY(normalForceLabel != NULL);
    QVERIFY(lateralForceLabel != NULL);
    QVERIFY(lengthHandleLabel != NULL);
    QVERIFY(directionHandleLabel != NULL);
    QVERIFY(elevationHandleLabel != NULL);
    QVERIFY(bankHandleLabel != NULL);

    const QImage beforePreview = renderViewport(glView);
    editor->beginBuilderPiece();
    const QImage withPreview = renderViewport(glView);
    QVERIFY(editor->hasBuilderPreview());
    QCOMPARE(coaster->lSections.size(), 0);
    QVERIFY(commitButton->isVisible());
    QVERIFY(cancelButton->isVisible());
    QCOMPARE(modeLabel->text(), QString("Preview"));
    QVERIFY(!nextButton->isVisible());
    QWidget* builderFrame = window->findChild<QWidget*>("builderEditorFrame");
    QVERIFY(builderFrame != NULL);
    QVERIFY(editor->isAncestorOf(builderFrame));

    QPoint lengthHandle;
    QPoint directionHandle;
    QPoint elevationHandle;
    QPoint bankHandle;
    QVERIFY(glView->builderHandlePosition(glViewWidget::BuilderLengthHandle, &lengthHandle));
    QVERIFY(glView->builderHandlePosition(glViewWidget::BuilderDirectionHandle, &directionHandle));
    QVERIFY(glView->builderHandlePosition(glViewWidget::BuilderElevationHandle, &elevationHandle));
    QVERIFY(glView->builderHandlePosition(glViewWidget::BuilderBankHandle, &bankHandle));
    const double directionBeforeDrag = directionBox->value();
    const QPoint directionDragEnd = directionHandle+QPoint(20, 0);
    QMouseEvent pressDirection(QEvent::MouseButtonPress,
                               directionHandle,
                               glView->mapToGlobal(directionHandle),
                               Qt::LeftButton,
                               Qt::LeftButton,
                               Qt::NoModifier);
    QApplication::sendEvent(glView, &pressDirection);
    QMouseEvent moveDirection(QEvent::MouseMove,
                              directionDragEnd,
                              glView->mapToGlobal(directionDragEnd),
                              Qt::NoButton,
                              Qt::LeftButton,
                              Qt::NoModifier);
    QApplication::sendEvent(glView, &moveDirection);
    QMouseEvent releaseDirection(QEvent::MouseButtonRelease,
                                 directionDragEnd,
                                 glView->mapToGlobal(directionDragEnd),
                                 Qt::LeftButton,
                                 Qt::NoButton,
                                 Qt::NoModifier);
    QApplication::sendEvent(glView, &releaseDirection);
    QVERIFY(!nearlyEqual(static_cast<float>(directionBox->value()),
                         static_cast<float>(directionBeforeDrag)));
    QVERIFY(editingLabel->text().contains("Direction"));

    QCOMPARE(withPreview.size(), beforePreview.size());
    int changedPreviewPixels = 0;
    for(int y = 0; y < withPreview.height(); ++y) {
        for(int x = 0; x < withPreview.width(); ++x) {
            const QColor beforeColor(beforePreview.pixel(x, y));
            const QColor previewColor(withPreview.pixel(x, y));
            if(std::abs(beforeColor.red()-previewColor.red()) > 12
               || std::abs(beforeColor.green()-previewColor.green()) > 12
               || std::abs(beforeColor.blue()-previewColor.blue()) > 12) {
                ++changedPreviewPixels;
            }
        }
    }
    QVERIFY2(changedPreviewPixels > 5,
             "Builder preview did not produce visible viewport pixels");

    lengthBox->setValue(12.0);
    elevationBox->setValue(3.0);
    directionBox->setValue(30.0);
    bankBox->setValue(42.0);
    renderViewport(glView);

    secbuilder* preview = coaster->getBuilderPreview();
    QVERIFY(preview != NULL);
    const float turnRadians = 30.f*F_PI/180.f;
    const float radius = 12.f/turnRadians;
    const glm::vec3 expectedOffset(radius*(1.f-std::cos(turnRadians)),
                                   3.f,
                                   radius*std::sin(turnRadians));
    QVERIFY(nearlyEqual(preview->segment().endOffset(), expectedOffset));
    QVERIFY(nearlyEqual(preview->segment().endRollDegrees(), 42.f));
    QVERIFY(preview->lNodes.size() > 2);
    QVERIFY(lengthHandleLabel->isVisible());
    QVERIFY(directionHandleLabel->isVisible());
    QVERIFY(elevationHandleLabel->isVisible());
    QVERIFY(bankHandleLabel->isVisible());
    QVERIFY(lengthHandleLabel->text().contains("12.00 m"));
    QVERIFY(directionHandleLabel->text().contains("+30.00"));
    QVERIFY(elevationHandleLabel->text().contains("+3.00 m"));
    QVERIFY(bankHandleLabel->text().contains("+42.00"));

    mnode& previewEnd = preview->lNodes.last();
    QVERIFY(yawLabel->text().contains(
        signedFixed(previewEnd.getDirection()+coaster->startYaw)));
    QVERIFY(pitchLabel->text().contains(signedFixed(previewEnd.getPitch())));
    QVERIFY(rollLabel->text().contains(signedFixed(previewEnd.fRoll)));
    QVERIFY(normalForceLabel->text().contains(
        signedFixed(previewEnd.forceNormal+previewEnd.smoothNormal)));
    QVERIFY(lateralForceLabel->text().contains(
        signedFixed(previewEnd.forceLateral+previewEnd.smoothLateral)));

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString previewPath = directory.filePath("preview-not-persisted.fvd");
    saver previewWriter(previewPath, window->project, window);
    QCOMPARE(previewWriter.doSave(), QString("Project Saved!"));
    QFile previewFile(previewPath);
    QVERIFY(previewFile.open(QIODevice::ReadOnly));
    QVERIFY(!previewFile.readAll().contains("BLD"));

    editor->cancelBuilderPiece();
    QVERIFY(!editor->hasBuilderPreview());
    QCOMPARE(coaster->lSections.size(), 0);

    editor->beginBuilderPiece();
    editor->updateBuilderPiece(12.0, 3.0, 30.0, 42.0);
    const QUuid expectedId = coaster->getBuilderPreview()->segment().id();
    editor->commitBuilderPiece();

    QVERIFY(!editor->hasBuilderPreview());
    QVERIFY(builderFrame->isVisible());
    QCOMPARE(modeLabel->text(), QString("Selected"));
    QVERIFY(nextButton->isVisible());
    QVERIFY(!lengthBox->isEnabled());
    QVERIFY(!elevationBox->isEnabled());
    QVERIFY(!directionBox->isEnabled());
    QVERIFY(!bankBox->isEnabled());
    QCOMPARE(coaster->lSections.size(), 1);
    secbuilder* committed = dynamic_cast<secbuilder*>(coaster->lSections.first());
    QVERIFY(committed != NULL);
    QCOMPARE(committed->segment().id(), expectedId);
    QVERIFY(nearlyEqual(committed->segment().endOffset(), expectedOffset));
    QVERIFY(nearlyEqual(committed->segment().endRollDegrees(), 42.f));
    QCOMPARE(editor->sectionList.size(), 2);
    QCOMPARE(editor->sectionList.last()->type, builder);

    // A committed Builder piece can extend directly into another live piece
    // without reopening Add -> Builder Piece.
    QTest::mouseClick(nextButton, Qt::LeftButton);
    QVERIFY(editor->hasBuilderPreview());
    QCOMPARE(coaster->lSections.size(), 1);
    QCOMPARE(modeLabel->text(), QString("Preview"));
    QVERIFY(commitButton->isVisible());
    QVERIFY(cancelButton->isVisible());
    QVERIFY(!nextButton->isVisible());
    editor->cancelBuilderPiece();
    QVERIFY(!editor->hasBuilderPreview());
    QCOMPARE(coaster->lSections.size(), 1);
    QVERIFY(nextButton->isVisible());

    handler->mUndoHandler->doUndo();
    QCOMPARE(coaster->lSections.size(), 0);
    handler->mUndoHandler->doRedo();
    QCOMPARE(coaster->lSections.size(), 1);
    secbuilder* redone = dynamic_cast<secbuilder*>(coaster->lSections.first());
    QVERIFY(redone != NULL);
    QCOMPARE(redone->segment().id(), expectedId);
    QVERIFY(nearlyEqual(redone->segment().endOffset(), expectedOffset));
    QVERIFY(nearlyEqual(redone->segment().endRollDegrees(), 42.f));
    glView->doneCurrent();
}

void LegacyProjectRegression::cameraUsesHoldToDrag()
{
    const QPoint start = glView->rect().center();
    QTest::mousePress(glView, Qt::RightButton, Qt::NoModifier, start);
    QVERIFY(glView->moveMode);

    QTest::mouseMove(glView, start+QPoint(20, 10));
    QVERIFY(glView->moveMode);

    QTest::mouseRelease(glView,
                        Qt::RightButton,
                        Qt::NoModifier,
                        start+QPoint(20, 10));
    QVERIFY(!glView->moveMode);
    QVERIFY(nearlyEqual(glView->cameraMov.x, 0.f));
    QVERIFY(nearlyEqual(glView->cameraMov.y, 0.f));
    QVERIFY(nearlyEqual(glView->cameraMov.z, 0.f));
    QVERIFY(nearlyEqual(glView->cameraMov.w, 0.f));
}

void LegacyProjectRegression::builderSectionRejectsInvalidPayload_data()
{
    QTest::addColumn<QString>("corruption");

    QTest::newRow("unsupported-version") << QString("version");
    QTest::newRow("negative-payload-size") << QString("size");
    QTest::newRow("truncated-payload") << QString("truncate");
}

void LegacyProjectRegression::builderSectionRejectsInvalidPayload()
{
    QFETCH(QString, corruption);

    track* coaster = window->project->trackList.first()->trackData;
    secbuilder source(coaster, coaster->anchorNode);
    QVERIFY(source.segment().setEndOffset(glm::vec3(3.f, 4.f, 12.f)));

    std::stringstream output;
    source.saveSection(output);
    std::string serialized = output.str();
    QVERIFY(serialized.size() > 16);
    QCOMPARE(QByteArray(serialized.data(), 3), QByteArray("BLD"));

    if(corruption == "version") {
        serialized[6] = 2;
    } else if(corruption == "size") {
        serialized[7] = static_cast<char>(0xff);
        serialized[8] = static_cast<char>(0xff);
        serialized[9] = static_cast<char>(0xff);
        serialized[10] = static_cast<char>(0xff);
    } else if(corruption == "truncate") {
        serialized.resize(serialized.size()-5);
    }

    std::stringstream input(serialized, std::ios::in | std::ios::binary);
    QCOMPARE(QString::fromStdString(readString(&input, 3)), QString("BLD"));

    secbuilder restored(coaster, coaster->anchorNode);
    const glm::vec3 unchangedOffset(8.f, 1.f, 9.f);
    QVERIFY(restored.segment().setEndOffset(unchangedOffset));
    restored.loadSection(input);

    QVERIFY(input.fail());
    QVERIFY(nearlyEqual(restored.segment().endOffset(), unchangedOffset));
    QVERIFY(restored.segment().isValid());
}

void LegacyProjectRegression::legacyFixturesLoad_data()
{
    QTest::addColumn<QString>("fixtureName");
    QTest::addColumn<QString>("expectedPrefix");

    QTest::newRow("v0.30")
        << QString("v0.30-one-track.fvd.b64")
        << QString("Warning: Loaded old File Version");
    QTest::newRow("v0.77")
        << QString("v0.77-one-track.fvd.b64")
        << QString("Load Successfull.");
}

void LegacyProjectRegression::legacyFixturesLoad()
{
    QFETCH(QString, fixtureName);
    QFETCH(QString, expectedPrefix);

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString projectPath = decodeFixture(fixtureName, directory.filePath("fixture.fvd"));
    QVERIFY2(!projectPath.isEmpty(), "Could not decode the legacy fixture");

    glView->makeCurrent();
    saver loader(projectPath, window->project, window);
    const QString result = loader.doLoad();
    glView->doneCurrent();

    QVERIFY2(result.startsWith(expectedPrefix), qPrintable(result));
    QCOMPARE(window->project->trackList.size(), 1);
    QCOMPARE(window->project->texPath, QString(":/background.png"));
    QCOMPARE(window->project->trackList.first()->trackData->name,
             QString::fromUtf8("Milestone 0 – 保存"));
}

void LegacyProjectRegression::projectSaveAndReloadRoundTrip()
{
    const QString expectedName = QString::fromUtf8("Milestone 0 – 保存");

    glView->makeCurrent();
    window->project->init();
    glView->doneCurrent();

    QCOMPARE(window->project->trackList.size(), 1);
    trackHandler* originalHandler = window->project->trackList.first();
    track* original = originalHandler->trackData;

    original->name = expectedName;
    original->startPos = glm::vec3(12.5f, 7.25f, -42.0f);
    original->startYaw = 73.5f;
    original->startPitch = -4.0f;
    original->anchorNode->fRoll = 8.0f;
    original->anchorNode->fVel = 16.75f;
    original->fHeart = 1.35f;
    original->fFriction = 0.0175f;
    original->fResistance = 0.000031f;
    original->drawTrack = false;
    originalHandler->trackColors[0] = QColor(12, 34, 56);
    originalHandler->trackColors[1] = QColor(78, 90, 123);
    originalHandler->trackColors[2] = QColor(145, 167, 189);

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString projectPath = directory.filePath("round-trip.fvd");

    saver writer(projectPath, window->project, window);
    QCOMPARE(writer.doSave(), QString("Project Saved!"));

    QFile saved(projectPath);
    QVERIFY(saved.open(QIODevice::ReadOnly));
    QCOMPARE(saved.read(8), QByteArray("FVDv0.77"));
    saved.close();

    glView->makeCurrent();
    saver reader(projectPath, window->project, window);
    const QString loadResult = reader.doLoad();
    glView->doneCurrent();

    QCOMPARE(loadResult, QString("Load Successfull."));
    QCOMPARE(window->project->trackList.size(), 1);

    trackHandler* restoredHandler = window->project->trackList.first();
    track* restored = restoredHandler->trackData;
    QCOMPARE(restored->name, expectedName);
    QEXPECT_FAIL("", "Known v0.77 defect: glm::vec3 serialization swaps X and Z", Continue);
    QVERIFY(nearlyEqual(restored->startPos.x, 12.5f));
    QVERIFY(nearlyEqual(restored->startPos.y, 7.25f));
    QEXPECT_FAIL("", "Known v0.77 defect: glm::vec3 serialization swaps X and Z", Continue);
    QVERIFY(nearlyEqual(restored->startPos.z, -42.0f));
    QVERIFY(nearlyEqual(restored->startYaw, 73.5f));
    QVERIFY(nearlyEqual(restored->startPitch, -4.0f));
    QVERIFY(nearlyEqual(restored->anchorNode->fRoll, 8.0f));
    QVERIFY(nearlyEqual(restored->anchorNode->fVel, 16.75f));
    QVERIFY(nearlyEqual(restored->fHeart, 1.35f));
    QVERIFY(nearlyEqual(restored->fFriction, 0.0175f));
    QVERIFY(nearlyEqual(restored->fResistance, 0.000031f));
    QCOMPARE(restored->drawTrack, false);
    QCOMPARE(restoredHandler->trackColors[0], QColor(12, 34, 56));
    QCOMPARE(restoredHandler->trackColors[1], QColor(78, 90, 123));
    QCOMPARE(restoredHandler->trackColors[2], QColor(145, 167, 189));
}

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);

#ifdef Q_OS_MAC
    QGLFormat format;
    format.setProfile(QGLFormat::CoreProfile);
    format.setVersion(3, 2);
    format.setSampleBuffers(true);
    format.setSamples(4);
    QGLFormat::setDefaultFormat(format);
#endif

    LegacyProjectRegression regression;
    return QTest::qExec(&regression, argc, argv);
}

#include "legacy_project_regression.moc"
