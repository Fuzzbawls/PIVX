// Copyright (c) 2019-2021 The PIVX Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txrow.h"
#include "ui_txrow.h"

#include "guiutil.h"
#include "qtutils.h"

TxRow::TxRow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TxRow)
{
    ui->setupUi(this);
    ui->lblAmountBottom->setVisible(false);
}

void TxRow::init(bool isLightTheme)
{
    setConfirmStatus(true);
    updateStatus(isLightTheme, false, false);
}

void TxRow::showHideSecondAmount(bool show) {
    if (show != isDoubleAmount) {
        isDoubleAmount = show;
        ui->lblAmountBottom->setVisible(show);
    }
}

void TxRow::setConfirmStatus(bool isConfirm){
    if(isConfirm){
        setCssProperty(ui->lblAddress, "text-list-body1");
        setCssProperty(ui->lblDate, "text-list-caption");
    } else {
        setCssProperty(ui->lblAddress, "text-list-body-unconfirmed");
        setCssProperty(ui->lblDate,"text-list-caption-unconfirmed");
    }
}

void TxRow::updateStatus(bool isLightTheme, bool isHover, bool isSelected)
{
    if (isLightTheme)
        ui->lblDivisory->setStyleSheet("background-color:#bababa");
    else
        ui->lblDivisory->setStyleSheet("background-color:#40ffffff");
}

void TxRow::setDate(QDateTime date)
{
    ui->lblDate->setText(GUIUtil::dateTimeStr(date));
}

void TxRow::setLabel(QString str)
{
    ui->lblAddress->setText(str);
}

void TxRow::setAmount(QString top, QString bottom)
{
    ui->lblAmountTop->setText(top);
    ui->lblAmountBottom->setText(bottom);
}

void TxRow::setType(bool isLightTheme, int type, bool isConfirmed)
{
    QString path;
    QString css;
    QString cssAmountBottom;
    bool sameIcon = false;
    switch (type) {
        case TransactionRecord::ZerocoinMint:
            path = "://ic-transaction-mint";
            css = "text-list-amount-send";
            break;
        case TransactionRecord::Generated:
        case TransactionRecord::StakeZPIV:
        case TransactionRecord::MNReward:
        case TransactionRecord::StakeMint:
        case TransactionRecord::BudgetPayment:
            path = "://ic-transaction-staked";
            css = "text-list-amount-receive";
            break;
        case TransactionRecord::RecvWithAddress:
        case TransactionRecord::RecvFromOther:
        case TransactionRecord::RecvFromZerocoinSpend:
        case TransactionRecord::RecvWithShieldedAddress:
            path = "://ic-transaction-received";
            css = "text-list-amount-receive";
            break;
        case TransactionRecord::RecvWithShieldedAddressMemo:
            path = "://ic-transaction-received-memo";
            css = "text-list-amount-receive";
            break;
        case TransactionRecord::SendToAddress:
        case TransactionRecord::SendToOther:
        case TransactionRecord::ZerocoinSpend:
        case TransactionRecord::ZerocoinSpend_Change_zPiv:
        case TransactionRecord::ZerocoinSpend_FromMe:
        case TransactionRecord::SendToShielded:
        case TransactionRecord::SendToNobody:
            path = "://ic-transaction-sent";
            css = "text-list-amount-send";
            break;
        case TransactionRecord::SendToSelf:
        case TransactionRecord::SendToSelfShieldToShieldChangeAddress:
            path = "://ic-transaction-mint";
            css = "text-list-amount-send";
            break;
        case TransactionRecord::StakeDelegated:
            path = "://ic-transaction-stake-delegated";
            css = "text-list-amount-receive";
            break;
        case TransactionRecord::StakeHot:
            path = "://ic-transaction-stake-hot";
            css = "text-list-amount-unconfirmed";
            break;
        case TransactionRecord::P2CSDelegationSent:
        case TransactionRecord::P2CSDelegationSentOwner:
            path = "://ic-transaction-cs-contract";
            css = "text-list-amount-send";
            break;
        case TransactionRecord::P2CSDelegation:
            path = "://ic-transaction-cs-contract";
            css = "text-list-amount-unconfirmed";
            break;
        case TransactionRecord::P2CSUnlockOwner:
        case TransactionRecord::P2CSUnlockStaker:
            path = "://ic-transaction-cs-contract";
            css = "text-list-amount-send";
            break;
        case TransactionRecord::SendToSelfShieldedAddress:
        case TransactionRecord::SendToSelfShieldToTransparent:
            path = "://ic-transaction-mint";
            css = "text-list-amount-unconfirmed";
            cssAmountBottom = "text-list-amount-send-small";
            break;
        default:
            path = "://ic-pending";
            sameIcon = true;
            css = "text-list-amount-unconfirmed";
            break;
    }

    if (!isLightTheme && !sameIcon){
        path += "-dark";
    }

    if (!isConfirmed){
        css = "text-list-amount-unconfirmed";
        cssAmountBottom = "text-list-amount-unconfirmed";
        path += "-inactive";
        setConfirmStatus(false);
    } else {
        setConfirmStatus(true);
    }
    setCssProperty(ui->lblAmountTop, css, true);
    if (isDoubleAmount) setCssProperty(ui->lblAmountBottom, cssAmountBottom, true);
    ui->icon->setIcon(QIcon(path));
}

TxRow::~TxRow()
{
    delete ui;
}
