/***************************************************************************
 *   Copyright (C) 2006-2008, 2014, 2016 by Hanna Knutsson                 *
 *   hanna_k@fmgirl.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <QPushButton>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include "budget.h"
#include "editscheduledtransactiondialog.h"
#include "recurrence.h"
#include "recurrenceeditwidget.h"
#include "transactioneditwidget.h"


EditScheduledTransactionDialog::EditScheduledTransactionDialog(bool extra_parameters, int transaction_type, Security *security, bool select_security, Budget *budg, QWidget *parent, QString title, Account *account) : QDialog(parent), budget(budg), b_extra(extra_parameters) {
	
	setWindowTitle(title);
	setModal(true);
	
	QVBoxLayout *box1 = new QVBoxLayout(this);

	tabs = new QTabWidget();
	switch(transaction_type) {
		case TRANSACTION_TYPE_EXPENSE: {
			transactionEditWidget = new TransactionEditWidget(false, b_extra, transaction_type, false, false, security, SECURITY_ALL_VALUES, select_security, budget); 
			tabs->addTab(transactionEditWidget, tr("Expense")); 
			break;
		}
		case TRANSACTION_TYPE_INCOME: {			
			transactionEditWidget = new TransactionEditWidget(false, b_extra, transaction_type, false, false, security, SECURITY_ALL_VALUES, select_security, budget); 
			tabs->addTab(transactionEditWidget, tr("Income")); 
			break;
		}
		case TRANSACTION_TYPE_TRANSFER: {			
			transactionEditWidget = new TransactionEditWidget(false, b_extra, transaction_type, false, false, security, SECURITY_ALL_VALUES, select_security, budget); 
			tabs->addTab(transactionEditWidget, tr("Transfer")); 
			break;
		}
		case TRANSACTION_TYPE_SECURITY_BUY: { 
			transactionEditWidget = new TransactionEditWidget(false, b_extra, transaction_type, false, false, security, SECURITY_ALL_VALUES, select_security, budget); 
			tabs->addTab(transactionEditWidget, tr("Security Buy")); 
			break;
		}
		case TRANSACTION_TYPE_SECURITY_SELL: {
			transactionEditWidget = new TransactionEditWidget(false, b_extra, transaction_type, false, false, security, SECURITY_ALL_VALUES, select_security, budget); 
			tabs->addTab(transactionEditWidget, tr("Security Sell")); 
			break;
		}
	}
	transactionEditWidget->updateAccounts();
	transactionEditWidget->transactionsReset();
	if(account) transactionEditWidget->setAccount(account);
	recurrenceEditWidget = new RecurrenceEditWidget(transactionEditWidget->date(), budget);
	tabs->addTab(recurrenceEditWidget, tr("Recurrence"));
	
	box1->addWidget(tabs);
	
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
	connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
	connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
	box1->addWidget(buttonBox);
	
	connect(transactionEditWidget, SIGNAL(dateChanged(const QDate&)), recurrenceEditWidget, SLOT(setStartDate(const QDate&)));
	transactionEditWidget->focusDescription();
}

bool EditScheduledTransactionDialog::checkAccounts() {
	return transactionEditWidget->checkAccounts();
}
void EditScheduledTransactionDialog::accept() {
	if(!transactionEditWidget->validValues(true)) {tabs->setCurrentIndex(0); return;}
	recurrenceEditWidget->setStartDate(transactionEditWidget->date());
	if(!recurrenceEditWidget->validValues()) {tabs->setCurrentIndex(1); return;}
	QDialog::accept();
}
void EditScheduledTransactionDialog::setTransaction(Transaction *trans) {
	transactionEditWidget->setTransaction(trans);
	recurrenceEditWidget->setRecurrence(NULL);
}
void EditScheduledTransactionDialog::setScheduledTransaction(ScheduledTransaction *strans) {
	transactionEditWidget->setTransaction(strans->transaction());
	recurrenceEditWidget->setRecurrence(strans->recurrence());
}
ScheduledTransaction *EditScheduledTransactionDialog::createScheduledTransaction() {
	Transaction *trans = transactionEditWidget->createTransaction();
	if(!trans) {tabs->setCurrentIndex(0); return NULL;}
	recurrenceEditWidget->setStartDate(trans->date());
	return new ScheduledTransaction(budget, trans, recurrenceEditWidget->createRecurrence());
}
bool EditScheduledTransactionDialog::modifyScheduledTransaction(ScheduledTransaction *strans) {
	Transaction *trans = transactionEditWidget->createTransaction();
	if(!trans) {tabs->setCurrentIndex(0); return false;}
	recurrenceEditWidget->setStartDate(trans->date());
	strans->setRecurrence(recurrenceEditWidget->createRecurrence());
	strans->setTransaction(trans);
	return true;
}
bool EditScheduledTransactionDialog::modifyTransaction(Transaction *trans, Recurrence *&rec) {
	if(!transactionEditWidget->modifyTransaction(trans)) {tabs->setCurrentIndex(0); return false;}
	recurrenceEditWidget->setStartDate(trans->date());
	rec = recurrenceEditWidget->createRecurrence();
	return true;
}
ScheduledTransaction *EditScheduledTransactionDialog::newScheduledTransaction(int transaction_type, Budget *budg, QWidget *parent, Security *security, bool select_security, Account *account, bool extra_parameters) {
	EditScheduledTransactionDialog *dialog = NULL;
	switch(transaction_type) {
		case TRANSACTION_TYPE_EXPENSE: {dialog = new EditScheduledTransactionDialog(extra_parameters, transaction_type, security, select_security, budg, parent, tr("New Expense"), account); break;}
		case TRANSACTION_TYPE_INCOME: {
			if(security || select_security) dialog = new EditScheduledTransactionDialog(extra_parameters, transaction_type, security, select_security, budg, parent, tr("New Dividend"), account);
			else dialog = new EditScheduledTransactionDialog(extra_parameters, transaction_type, security, select_security, budg, parent, tr("New Income"), account);
			break;
		}
		case TRANSACTION_TYPE_TRANSFER: {dialog = new EditScheduledTransactionDialog(extra_parameters, transaction_type, security, select_security, budg, parent, tr("New Transfer"), account); break;}
		case TRANSACTION_TYPE_SECURITY_BUY: {dialog = new EditScheduledTransactionDialog(extra_parameters, transaction_type, security, select_security, budg, parent, tr("New Security Buy"), account); break;}
		case TRANSACTION_TYPE_SECURITY_SELL: {
			dialog = new EditScheduledTransactionDialog(extra_parameters, transaction_type, security, select_security, budg, parent, tr("New Security Sell"), account);
			//dialog->transactionEditWidget->setMaxSharesDate(QDate::currentDate());
			break;
		}
	}
	ScheduledTransaction *strans = NULL;
	if(dialog->checkAccounts() && dialog->exec() == QDialog::Accepted) {
		strans = dialog->createScheduledTransaction();
	}
	dialog->deleteLater();
	return strans;
}
bool EditScheduledTransactionDialog::editScheduledTransaction(ScheduledTransaction *strans, QWidget *parent, bool select_security, bool extra_parameters) {
	EditScheduledTransactionDialog *dialog = NULL;
	switch(strans->transaction()->type()) {
		case TRANSACTION_TYPE_EXPENSE: {dialog = new EditScheduledTransactionDialog(extra_parameters, strans->transaction()->type(), NULL, false, strans->budget(), parent, tr("Edit Expense")); break;}
		case TRANSACTION_TYPE_INCOME: {
			if(((Income*) strans->transaction())->security()) dialog = new EditScheduledTransactionDialog(extra_parameters, strans->transaction()->type(), ((Income*) strans->transaction())->security(), select_security, strans->budget(), parent, tr("Edit Dividend"));
			else dialog = new EditScheduledTransactionDialog(extra_parameters, strans->transaction()->type(), NULL, false, strans->budget(), parent, tr("Edit Income"));
			break;
		}
		case TRANSACTION_TYPE_TRANSFER: {dialog = new EditScheduledTransactionDialog(extra_parameters, strans->transaction()->type(), NULL, false, strans->budget(), parent, tr("Edit Transfer")); break;}
		case TRANSACTION_TYPE_SECURITY_BUY: {dialog = new EditScheduledTransactionDialog(extra_parameters, strans->transaction()->type(), ((SecurityTransaction*) strans->transaction())->security(), select_security, strans->budget(), parent, tr("Edit Securities Bought")); break;}
		case TRANSACTION_TYPE_SECURITY_SELL: {dialog = new EditScheduledTransactionDialog(extra_parameters, strans->transaction()->type(), ((SecurityTransaction*) strans->transaction())->security(), select_security, strans->budget(), parent, tr("Edit Securities Sold")); break;}
	}
	dialog->setScheduledTransaction(strans);
	bool b = false;
	if(dialog->checkAccounts() && dialog->exec() == QDialog::Accepted) {
		b = dialog->modifyScheduledTransaction(strans);
	}
	dialog->deleteLater();
	return b;
}
bool EditScheduledTransactionDialog::editTransaction(Transaction *trans, Recurrence *&rec, QWidget *parent, bool select_security, bool extra_parameters) {
	EditScheduledTransactionDialog *dialog = NULL;
	switch(trans->type()) {
		case TRANSACTION_TYPE_EXPENSE: {dialog = new EditScheduledTransactionDialog(extra_parameters, trans->type(), NULL, false, trans->budget(), parent, tr("Edit Expense")); break;}
		case TRANSACTION_TYPE_INCOME: {
			if(((Income*) trans)->security()) dialog = new EditScheduledTransactionDialog(extra_parameters, trans->type(), ((Income*) trans)->security(), select_security, trans->budget(), parent, tr("Edit Dividend"));
			else dialog = new EditScheduledTransactionDialog(extra_parameters, trans->type(), NULL, false, trans->budget(), parent, tr("Edit Income"));
			break;
		}
		case TRANSACTION_TYPE_TRANSFER: {dialog = new EditScheduledTransactionDialog(extra_parameters, trans->type(), NULL, false, trans->budget(), parent, tr("Edit Transfer")); break;}
		case TRANSACTION_TYPE_SECURITY_BUY: {dialog = new EditScheduledTransactionDialog(extra_parameters, trans->type(), ((SecurityTransaction*) trans)->security(), select_security, trans->budget(), parent, tr("Edit Securities Bought")); break;}
		case TRANSACTION_TYPE_SECURITY_SELL: {dialog = new EditScheduledTransactionDialog(extra_parameters, trans->type(), ((SecurityTransaction*) trans)->security(), select_security, trans->budget(), parent, tr("Edit Securities Sold")); break;}
	}
	dialog->setTransaction(trans);
	bool b = false;
	if(dialog->checkAccounts() && dialog->exec() == QDialog::Accepted) {
		b = dialog->modifyTransaction(trans, rec);
	}
	dialog->deleteLater();
	return b;
}
