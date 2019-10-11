// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef WELCOMECONTENTWIDGET_H
#define WELCOMECONTENTWIDGET_H

#include <QDialog>
#include <QPushButton>
#include <QWidget>

class NetworkStyle;
class OptionsModel;

namespace Ui {
class WelcomeContentWidget;
}

class WelcomeContentWidget : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeContentWidget(Qt::WindowFlags f, const NetworkStyle* networkStyle);
    ~WelcomeContentWidget();
    int pos = 0;
    bool isOk = false;

    void setModel(OptionsModel *model);
    void checkLanguage();

signals:
    void onLanguageSelected();

public slots:
    void onNextClicked();
    void onBackClicked();
    void onSkipClicked();

private:
    Ui::WelcomeContentWidget *ui;
    QPushButton *icConfirm1;
    QPushButton *icConfirm2;
    QPushButton *icConfirm3;
    QPushButton *icConfirm4;
    QPushButton *backButton;
    QPushButton *nextButton;

    OptionsModel *model;

    void initLanguages();
};

#endif // WELCOMECONTENTWIDGET_H
