#include "buildereditorwidget.h"

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSizePolicy>
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
    result->setMinimumHeight(22);
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
    setFrameShape(QFrame::Panel);
    setFrameShadow(QFrame::Sunken);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    titleLabel = new QLabel(tr("Track Builder"), this);
    titleLabel->setObjectName("builderTitle");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    modeLabel = new QLabel(tr("Preview"), this);
    modeLabel->setObjectName("builderModeBadge");
    modeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QFont modeFont = modeLabel->font();
    modeFont.setItalic(true);
    modeFont.setPointSize(8);
    modeLabel->setFont(modeFont);

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
    editingLabel->setFrameShape(QFrame::Panel);
    editingLabel->setFrameShadow(QFrame::Sunken);
    editingLabel->setMargin(4);
    setEditingStatus(tr("Piece settings"));

    hintLabel = new QLabel(
        tr("Drag the labeled handles on the track, or enter exact values below."),
        this);
    hintLabel->setObjectName("builderDragHint");
    hintLabel->setTextFormat(Qt::RichText);
    hintLabel->setWordWrap(true);

    QLabel* sectionLabel = new QLabel(tr("Shape"), this);
    sectionLabel->setObjectName("builderSectionLabel");
    QFont sectionFont = sectionLabel->font();
    sectionFont.setBold(true);
    sectionLabel->setFont(sectionFont);

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
    nextButton = new QPushButton(tr("Build Next Piece"), this);
    nextButton->setObjectName("builderNextButton");

    QHBoxLayout* actions = new QHBoxLayout();
    actions->setContentsMargins(0, 0, 0, 0);
    actions->addWidget(cancelButton);
    actions->addWidget(commitButton);
    actions->addWidget(nextButton);
    actions->setStretch(0, 1);
    actions->setStretch(1, 2);
    actions->setStretch(2, 2);

    QVBoxLayout* content = new QVBoxLayout(this);
    content->setContentsMargins(4, 4, 4, 4);
    content->setSpacing(4);
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
    connect(nextButton, SIGNAL(released()), this, SIGNAL(nextPieceRequested()));

    setParameters(10.0, 0.0, 0.0, 0.0);
    setPreviewMode(true);
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
    titleLabel->setText(tr("Track Builder"));
    modeLabel->setText(preview ? tr("Preview") : tr("Selected"));
    hintLabel->setText(preview
        ? tr("Drag the labeled handles on the track, or enter exact values below.")
        : tr("This piece's exact shape values are shown below. Continue building from its endpoint."));
    if(!preview) editingLabel->setText(tr("Piece information"));
    lengthBox->setEnabled(preview);
    elevationBox->setEnabled(preview);
    directionBox->setEnabled(preview);
    bankBox->setEnabled(preview);
    commitButton->setVisible(preview);
    cancelButton->setVisible(preview);
    nextButton->setVisible(!preview);
    commitButton->setDefault(preview);
    nextButton->setDefault(!preview);
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
