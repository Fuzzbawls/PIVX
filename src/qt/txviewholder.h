// Copyright (c) 2019-2021 The PIVX Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PIVX_QT_TXVIEWHOLDER_H
#define PIVX_QT_TXVIEWHOLDER_H

#include "bitcoinunits.h"
#include "furlistrow.h"
#include "txrow.h"
#include <transactionfilterproxy.h>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class TxViewHolder : public FurListRow<QWidget*>
{
public:
    TxViewHolder();

    explicit TxViewHolder(bool _isLightTheme) : FurListRow(), isLightTheme(_isLightTheme){}

    QWidget* createHolder(int pos) override;

    void init(QWidget* holder,const QModelIndex &index, bool isHovered, bool isSelected) const override;

    QColor rectColor(bool isHovered, bool isSelected) override;

    ~TxViewHolder() override{};

    bool isLightTheme;

    void setDisplayUnit(int displayUnit){
        this->nDisplayUnit = displayUnit;
    }

    void setFilter(TransactionFilterProxy *_filter){
        this->filter = _filter;
    }

private:
    int nDisplayUnit{0};
    TransactionFilterProxy *filter{nullptr};
    TxRow* txRow{nullptr};
};

#endif // PIVX_QT_TXVIEWHOLDER_H
