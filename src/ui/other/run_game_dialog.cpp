/*
 * This file is part of EasyRPG Editor.
 *
 * EasyRPG Editor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Editor. If not, see <http://www.gnu.org/licenses/>.
 */

#include "core.h"
#include "common/dbstring.h"
#include "run_game_dialog.h"
#include "ui_run_game_dialog.h"
#include "ui/database/actor_delegate.h"

RunGameDialog::RunGameDialog(ProjectData& project, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::RunGameDialog),
	m_project(project)
{
	ui->setupUi(this);

	ui->tableInitialParty->setItemDelegate(new ActorDelegate(this));
	on_comboMode_currentIndexChanged(0);

	UpdateModels();
}

RunGameDialog::~RunGameDialog()
{
	delete ui;
}

void RunGameDialog::runHere(int map_id, int x, int y)
{
	UpdateModels();
	ui->comboMode->setCurrentIndex(1);
	ui->groupInitialPosition->setChecked(true);
	for (int i = 0; i < ui->comboMapId->count(); i++)
	{
		if (ui->comboMapId->itemData(i).toInt() == map_id)
		{
			ui->comboMapId->setCurrentIndex(i);
			break;
		}
	}
	ui->spinX->setValue(x);
	ui->spinY->setValue(y);
	this->exec();
}

void RunGameDialog::runBattle(int troop_id)
{
	UpdateModels();
	ui->comboMode->setCurrentIndex(3);
	ui->comboTroop->setCurrentIndex(troop_id-1);
}

void RunGameDialog::on_comboMode_currentIndexChanged(int index)
{
	switch (index)
	{
	case 0:
	case 1:
		ui->groupSave->setEnabled(false);
		ui->groupInitialPosition->setEnabled(true);
		ui->groupBattleOptions->setEnabled(false);
		ui->groupInitialParty->setEnabled(true);
		break;
	case 2:
		ui->groupSave->setEnabled(true);
		ui->groupInitialPosition->setEnabled(false);
		ui->groupBattleOptions->setEnabled(false);
		ui->groupInitialParty->setEnabled(false);
		break;
	case 3:
		ui->groupSave->setEnabled(false);
		ui->groupInitialPosition->setEnabled(false);
		ui->groupBattleOptions->setEnabled(true);
		ui->groupInitialParty->setEnabled(true);
		break;
	}
}

void RunGameDialog::UpdateModels()
{
	// Maps
	auto& tree = m_project.treeMap();
	ui->comboMapId->clear();
	for (size_t i = 1; i < tree.maps.size(); i++)
	{
		if (tree.maps[i].type == 1)
		{
			ui->comboMapId->addItem(ToQString(tree.maps[i].name),
									tree.maps[i].ID);
		}
	}

	// Troops
	auto& database = m_project.database();
	ui->comboTroop->clear();
	ui->comboTroop->addItem(tr("<Test Troop>"));
	for (size_t i = 1; i < database.troops.size(); i++)
		ui->comboTroop->addItem(ToQString(database.troops[i].name));

	bool auto_placement = static_cast<bool>(database.battlecommands.placement);
	ui->groupPartyFormation->setEnabled(auto_placement);
	ui->comboBattleCondition->clear();
	QStringList conditions;
	conditions << tr("No condition") << tr("Initiative") << tr("Back attack");
	if (auto_placement)
		conditions << tr("Surround attack") << tr("Pincers attack");
	ui->comboBattleCondition->addItems(conditions);

	ui->comboCustomFormation->clear();
	for (size_t i = 0; i < database.terrains.size(); i++)
		ui->comboCustomFormation->addItem(ToQString(database.terrains[i].name));

	battletest_data = database.system.battletest_data;
	for (size_t i = 0; i < battletest_data.size(); i++)
		ui->tableInitialParty->item(static_cast<int>(i),0)->setData(Qt::UserRole, battletest_data[i].actor_id);
}

void RunGameDialog::on_tableInitialParty_itemChanged(QTableWidgetItem *item)
{
	if (item->data(Qt::UserRole).toInt() == 0)
	{
		while (ui->tableInitialParty->rowCount() > item->row()+1)
		{
			ui->tableInitialParty->removeRow(item->row()+1);
		}
	}
	else if (item->row() == ui->tableInitialParty->rowCount()-1 &&
			 ui->tableInitialParty->rowCount() < 4)
	{
		QTableWidgetItem *n_item = new QTableWidgetItem(tr("<None>"));
		n_item->setData(Qt::UserRole, 0);
		ui->tableInitialParty->insertRow(item->row()+1);
		ui->tableInitialParty->setItem(item->row()+1, 0, n_item);
	}
}
