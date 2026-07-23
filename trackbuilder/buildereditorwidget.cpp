#include "buildereditorwidget.h"

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace {

QDoubleSpinBox* createSpinBox(const char* objectName,
                              double minimum,
                              double maximum,
                              const QString& suffix,
                              QWidget* parent)
{
    QDoubleSpinBox* result = new QDoubleSpinBox(parent);
    result->setObjectName(objectName);
    result->setRange(minimum, maximum);
    result->setDecimals(2);
    result->setSingleStep(1.0);
    result->setSuffix(suffix);
    result->setKeyboardTracking(false);
    result->setAlignment(Qt::AlignRight);
    return result;
}

QLabel* createParameterLabel(const QString& name,
                             const QString& color,
                             QWidget* parent)
{
    QLabel* result = new QLabel(
        QString("<span style='color:%1'>●</span>&nbsp; %2").arg(color, name),
        parent);
    result->setTextFormat(Qt::RichText);
    return result;
}

}

BuilderEditorWidget::BuilderEditorWidget(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("builderEditorFrame");
    setFrameShape(QFrame::NoFrame);
    setFixedWidth(250);
    setStyleSheet(QString::fromLatin1(
        "QFrame#builderEditorFrame { background-color: #1c2026; "
        "border: 1px solid #39444f; border-radius: 8px; } "
        "QFrame#builderEditorFrame QLabel { color: #edf2f6; } "
        "QLabel#builderTitle { color: #ffffff; font-size: 13px; font-weight: 700; } "
        "QLabel#builderModeBadge { color: #72e4f2; background-color: #19343b; "
        "border: 1px solid #2a6973; border-radius: 5px; padding: 2px 6px; "
        "font-size: 9px; font-weight: 700; } "
        "QLabel#builderEditingStatus { color: #bceef5; background-color: #252c34; "
        "border: 1px solid #35414c; border-radius: 5px; padding: 5px 7px; } "
        "QLabel#builderDragHint { color: #aeb9c3; font-size: 10px; } "
        "QLabel#builderSectionLabel { color: #7f8b96; font-size: 9px; font-weight: 700; } "
        "QFrame#builderEditorFrame QDoubleSpinBox { color: #f5f8fa; "
        "background-color: #12161b; border: 1px solid #414c57; border-radius: 5px; "
        "min-height: 26px; padding: 0 6px; selection-background-color: #2d9daf; } "
        "QFrame#builderEditorFrame QDoubleSpinBox:focus { border-color: #47d2ee; } "
        "QFrame#builderEditorFrame QPushButton { min-height: 27px; border-radius: 5px; "
        "padding: 0 10px; } "
        "QPushButton#builderCancelButton { color: #d7dee4; background-color: #2a3139; "
        "border: 1px solid #46515c; } "
        "QPushButton#builderCancelButton:hover { background-color: #343d46; } "
        "QPushButton#builderCommitButton { color: #071419; background-color: #47d2ee; "
        "border: 1px solid #72e4f2; font-weight: 700; } "
        "QPushButton#builderCommitButton:hover { background-color: #68def2; } "
        "QPushButton#builderCommitButton:pressed { background-color: #2eb8d3; }"));

    titleLabel = new QLabel(tr("New Track Piece"), this);
    titleLabel->setObjectName("builderTitle");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    modeLabel = new QLabel(tr("LIVE PREVIEW"), this);
    modeLabel->setObjectName("builderModeBadge");
    modeLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout* header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    header->addWidget(titleLabel);
    header->addStretch();
    header->addWidget(modeLabel);

    editingLabel = new QLabel(this);
    editingLabel->setObjectName("builderEditingStatus");
    QFont editingFont = editingLabel->font();
    editingFont.setBold(true);
    editingLabel->setFont(editingFont);
    setEditingStatus(tr("Piece settings"));

    hintLabel = new QLabel(
        tr("Drag the labeled handles on the track, or enter exact values below."),
        this);
    hintLabel->setObjectName("builderDragHint");
    hintLabel->setTextFormat(Qt::RichText);
    hintLabel->setWordWrap(true);

    QLabel* sectionLabel = new QLabel(tr("PIECE SHAPE"), this);
    sectionLabel->setObjectName("builderSectionLabel");

    lengthBox = createSpinBox("builderLengthBox", 0.25, 2000.0, tr(" m"), this);
    elevationBox = createSpinBox("builderElevationBox", -1000.0, 1000.0, tr(" m"), this);
    directionBox = createSpinBox("builderDirectionBox", -180.0, 180.0, QChar(0x00b0), this);
    bankBox = createSpinBox("builderBankBox", -180.0, 180.0, QChar(0x00b0), this);
    directionBox->setSingleStep(5.0);
    bankBox->setSingleStep(5.0);

    QGridLayout* parameters = new QGridLayout();
    parameters->setContentsMargins(0, 0, 0, 0);
    parameters->setHorizontalSpacing(8);
    parameters->setVerticalSpacing(6);
    parameters->setColumnStretch(1, 1);
    parameters->addWidget(createParameterLabel(tr("Length"), "#ffd54f", this), 0, 0);
    parameters->addWidget(lengthBox, 0, 1);
    parameters->addWidget(createParameterLabel(tr("Elevation"), "#66e08a", this), 1, 0);
    parameters->addWidget(elevationBox, 1, 1);
    parameters->addWidget(createParameterLabel(tr("Direction"), "#ff9f43", this), 2, 0);
    parameters->addWidget(directionBox, 2, 1);
    parameters->addWidget(createParameterLabel(tr("Bank"), "#d889ff", this), 3, 0);
    parameters->addWidget(bankBox, 3, 1);

    commitButton = new QPushButton(tr("Build Piece"), this);
    commitButton->setObjectName("builderCommitButton");
    cancelButton = new QPushButton(tr("Cancel"), this);
    cancelButton->setObjectName("builderCancelButton");
    commitButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setCursor(Qt::PointingHandCursor);

    QHBoxLayout* actions = new QHBoxLayout();
    actions->addWidget(cancelButton);
    actions->addWidget(commitButton);
    actions->setStretch(0, 1);
    actions->setStretch(1, 2);

    QVBoxLayout* content = new QVBoxLayout(this);
    content->setContentsMargins(10, 9, 10, 10);
    content->setSpacing(8);
    content->addLayout(header);
    content->addWidget(editingLabel);
    content->addWidget(hintLabel);
    content->addWidget(sectionLabel);
    content->addLayout(parameters);
    content->addLayout(actions);

    connect(lengthBox, SIGNAL(valueChanged(double)), this, SLOT(emitParameters()));
    connect(elevationBox, SIGNAL(valueChanged(double)), this, SLOT(emitParameters()));
    connect(directionBox, SIGNAL(valueChanged(double)), this, SLOT(emitParameters()));
    connect(bankBox, SIGNAL(valueChanged(double)), this, SLOT(emitParameters()));
    connect(commitButton, SIGNAL(released()), this, SIGNAL(commitRequested()));
    connect(cancelButton, SIGNAL(released()), this, SIGNAL(cancelRequested()));

    setParameters(10.0, 0.0, 0.0, 0.0);
}

void BuilderEditorWidget::adjustLength(double amount)
{
    setEditingStatus(tr("Length — %1").arg(lengthBox->text()));
    lengthBox->setValue(lengthBox->value()+amount);
}

void BuilderEditorWidget::adjustElevation(double amount)
{
    setEditingStatus(tr("Elevation — %1").arg(elevationBox->text()));
    elevationBox->setValue(elevationBox->value()+amount);
}

void BuilderEditorWidget::adjustDirection(double amount)
{
    setEditingStatus(tr("Direction — %1").arg(directionBox->text()));
    directionBox->setValue(directionBox->value()+amount);
}

void BuilderEditorWidget::adjustBank(double amount)
{
    setEditingStatus(tr("Bank — %1").arg(bankBox->text()));
    bankBox->setValue(bankBox->value()+amount);
}

void BuilderEditorWidget::setEditingStatus(const QString& status)
{
    editingLabel->setText(tr("Editing: %1").arg(status));
}

void BuilderEditorWidget::setParameters(double length,
                                        double elevation,
                                        double directionDegrees,
                                        double bankDegrees)
{
    const QSignalBlocker blockLength(lengthBox);
    const QSignalBlocker blockElevation(elevationBox);
    const QSignalBlocker blockDirection(directionBox);
    const QSignalBlocker blockBank(bankBox);
    lengthBox->setValue(length);
    elevationBox->setValue(elevation);
    directionBox->setValue(directionDegrees);
    bankBox->setValue(bankDegrees);
}

void BuilderEditorWidget::setPreviewMode(bool preview)
{
    titleLabel->setText(preview ? tr("New Track Piece") : tr("Track Piece"));
    modeLabel->setText(preview ? tr("LIVE PREVIEW") : tr("SELECTED"));
    hintLabel->setText(preview
        ? tr("Drag the labeled handles on the track, or enter exact values below.")
        : tr("This piece's exact shape values are shown below."));
    lengthBox->setEnabled(preview);
    elevationBox->setEnabled(preview);
    directionBox->setEnabled(preview);
    bankBox->setEnabled(preview);
    commitButton->setVisible(preview);
    cancelButton->setVisible(preview);
}

void BuilderEditorWidget::emitParameters()
{
    if(sender() == lengthBox)
        setEditingStatus(tr("Length — %1").arg(lengthBox->text()));
    else if(sender() == elevationBox)
        setEditingStatus(tr("Elevation — %1").arg(elevationBox->text()));
    else if(sender() == directionBox)
        setEditingStatus(tr("Direction — %1").arg(directionBox->text()));
    else if(sender() == bankBox)
        setEditingStatus(tr("Bank — %1").arg(bankBox->text()));
    emit parametersChanged(lengthBox->value(),
                           elevationBox->value(),
                           directionBox->value(),
                           bankBox->value());
}
