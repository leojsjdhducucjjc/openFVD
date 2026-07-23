#ifndef BUILDEREDITORWIDGET_H
#define BUILDEREDITORWIDGET_H

#include <QFrame>

class QDoubleSpinBox;
class QLabel;
class QPushButton;

// Small, self-contained editor for one directly built track piece. Keeping
// this out of trackWidget.ui lets later builder tools evolve without growing
// the already large advanced-section form.
class BuilderEditorWidget : public QFrame
{
    Q_OBJECT

public:
    explicit BuilderEditorWidget(QWidget* parent = NULL);

    void setParameters(double length,
                       double elevation,
                       double directionDegrees,
                       double bankDegrees);
    void setPreviewMode(bool preview);
    void adjustLength(double amount);
    void adjustElevation(double amount);
    void adjustDirection(double amount);
    void adjustBank(double amount);
    void setEditingStatus(const QString& status);

signals:
    void parametersChanged(double length,
                           double elevation,
                           double directionDegrees,
                           double bankDegrees);
    void commitRequested();
    void cancelRequested();

private slots:
    void emitParameters();

private:
    QLabel* titleLabel;
    QLabel* modeLabel;
    QLabel* editingLabel;
    QLabel* hintLabel;
    QDoubleSpinBox* lengthBox;
    QDoubleSpinBox* elevationBox;
    QDoubleSpinBox* directionBox;
    QDoubleSpinBox* bankBox;
    QPushButton* commitButton;
    QPushButton* cancelButton;
};

#endif // BUILDEREDITORWIDGET_H
