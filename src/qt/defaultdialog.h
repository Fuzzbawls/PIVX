// Copyright (c) 2019-2020 The PIVX Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PIVX_QT_DEFAULTDIALOG_H
#define PIVX_QT_DEFAULTDIALOG_H

#include "focuseddialog.h"

namespace Ui {
class DefaultDialog;
}

class DefaultDialog : public FocusedDialog
{
    Q_OBJECT

public:
    explicit DefaultDialog(QWidget *parent = nullptr);
    ~DefaultDialog();

    void setText(const QString& title = "", const QString& message = "", const QString& okBtnText = "", const QString& cancelBtnText = "");

    bool isOk = false;

public Q_SLOTS:
    void accept() override;

private:
    Ui::DefaultDialog *ui;
};

#endif // PIVX_QT_DEFAULTDIALOG_H
