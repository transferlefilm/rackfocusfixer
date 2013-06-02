/********************************************************************************
** Form generated from reading UI file 'exportDialog.ui'
**
** Created: Sun Jun 2 11:35:51 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXPORTDIALOG_H
#define UI_EXPORTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ExportDialog
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *widget;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QComboBox *distanceCombo;
    QComboBox *rampCombo;
    QLabel *label;
    QWidget *widget_3;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QSpinBox *durationSpin;
    QCheckBox *durationCheck;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancelButton;
    QPushButton *exportButton;

    void setupUi(QDialog *ExportDialog)
    {
        if (ExportDialog->objectName().isEmpty())
            ExportDialog->setObjectName(QString::fromUtf8("ExportDialog"));
        ExportDialog->resize(425, 235);
        verticalLayout = new QVBoxLayout(ExportDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        widget = new QWidget(ExportDialog);
        widget->setObjectName(QString::fromUtf8("widget"));
        gridLayout = new QGridLayout(widget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_2 = new QLabel(widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 1, 1, 1);

        distanceCombo = new QComboBox(widget);
        distanceCombo->setObjectName(QString::fromUtf8("distanceCombo"));

        gridLayout->addWidget(distanceCombo, 1, 0, 1, 1);

        rampCombo = new QComboBox(widget);
        rampCombo->setObjectName(QString::fromUtf8("rampCombo"));

        gridLayout->addWidget(rampCombo, 1, 1, 1, 1);

        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);


        verticalLayout->addWidget(widget);

        widget_3 = new QWidget(ExportDialog);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));
        horizontalLayout_2 = new QHBoxLayout(widget_3);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_3 = new QLabel(widget_3);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_2->addWidget(label_3);

        durationSpin = new QSpinBox(widget_3);
        durationSpin->setObjectName(QString::fromUtf8("durationSpin"));
        durationSpin->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        durationSpin->setMaximum(99999);
        durationSpin->setSingleStep(25);
        durationSpin->setValue(10);

        horizontalLayout_2->addWidget(durationSpin);

        durationCheck = new QCheckBox(widget_3);
        durationCheck->setObjectName(QString::fromUtf8("durationCheck"));
        durationCheck->setChecked(true);

        horizontalLayout_2->addWidget(durationCheck);


        verticalLayout->addWidget(widget_3);

        widget_2 = new QWidget(ExportDialog);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        horizontalLayout = new QHBoxLayout(widget_2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(138, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        cancelButton = new QPushButton(widget_2);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout->addWidget(cancelButton);

        exportButton = new QPushButton(widget_2);
        exportButton->setObjectName(QString::fromUtf8("exportButton"));

        horizontalLayout->addWidget(exportButton);


        verticalLayout->addWidget(widget_2);


        retranslateUi(ExportDialog);

        QMetaObject::connectSlotsByName(ExportDialog);
    } // setupUi

    void retranslateUi(QDialog *ExportDialog)
    {
        ExportDialog->setWindowTitle(QApplication::translate("ExportDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ExportDialog", "Ramp Method", 0, QApplication::UnicodeUTF8));
        distanceCombo->clear();
        distanceCombo->insertItems(0, QStringList()
         << QApplication::translate("ExportDialog", "Linear", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ExportDialog", "Exponential", 0, QApplication::UnicodeUTF8)
        );
        rampCombo->clear();
        rampCombo->insertItems(0, QStringList()
         << QApplication::translate("ExportDialog", "Linear", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ExportDialog", "Exponential", 0, QApplication::UnicodeUTF8)
        );
        label->setText(QApplication::translate("ExportDialog", "Distance Method", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ExportDialog", "Duration (Frames)", 0, QApplication::UnicodeUTF8));
        durationCheck->setText(QApplication::translate("ExportDialog", "Use Best Duration", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("ExportDialog", "Cancel", 0, QApplication::UnicodeUTF8));
        exportButton->setText(QApplication::translate("ExportDialog", "Export", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ExportDialog: public Ui_ExportDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXPORTDIALOG_H
