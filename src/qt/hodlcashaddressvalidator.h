// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HODLCASH_QT_HODLCASHADDRESSVALIDATOR_H
#define HODLCASH_QT_HODLCASHADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class HodlCashAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit HodlCashAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** HodlCash address widget validator, checks for a valid hodlcash address.
 */
class HodlCashAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit HodlCashAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // HODLCASH_QT_HODLCASHADDRESSVALIDATOR_H
