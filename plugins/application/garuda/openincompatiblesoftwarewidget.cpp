/*
 * Garuda plugin - Enables communication with other Garuda enabled apps.
 *
 * Copyright (C) 2011-2012  Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 *
 * 
 * Author: Michael Wybrow <mjwybrow@users.sourceforge.net>
*/


#include <QtXml>

#include "libdunnartcanvas/canvas.h"
#include "openincompatiblesoftwarewidget.h"
#include "ui_openincompatiblesoftwarewidget.h"

OpenInCompatibleSoftwareWidget::OpenInCompatibleSoftwareWidget(
        dunnart::Canvas *canvas, QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::OpenInCompatibleSoftwareWidget),
    m_app_garuda_client(NULL),
    m_canvas(NULL)
{
    ui->setupUi(this);

    connect(ui->tableWidget, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(cellWasDoubleClicked(int,int)));

    changeCanvas(canvas);
}

OpenInCompatibleSoftwareWidget::~OpenInCompatibleSoftwareWidget()
{
    delete ui;
}

void OpenInCompatibleSoftwareWidget::setAppGarudaClient(
        AppGarudaClient *appGarudaClient)
{
    m_app_garuda_client = appGarudaClient;
}

void OpenInCompatibleSoftwareWidget::setContentsForResponse(const QVariantMap& response)
{
    QTableWidget *tableWidget = ui->tableWidget;

    tableWidget->clearContents();

    QTableWidgetItem *newItem = NULL;

    QVariantMap body = response["body"].toMap();
    QVariantList softwares = body["softwares"].toList();
    for (int i = 0; i < softwares.count(); ++i)
    {
        QVariantMap software = softwares.at(i).toMap();

        newItem = new QTableWidgetItem(software["id"].toString());
        tableWidget->setItem(i, 0, newItem);

        newItem = new QTableWidgetItem(software["version"].toString());
        tableWidget->setItem(i, 1, newItem);

        newItem = new QTableWidgetItem(software["name"].toString());
        tableWidget->setItem(i, 2, newItem);
    }
}

void OpenInCompatibleSoftwareWidget::cellWasDoubleClicked(int row, int column)
{
    QTableWidget *tableWidget = ui->tableWidget;

    // XXX hardcoded filename.
    m_app_garuda_client->loadFileIntoSoftware(
                QFileInfo("/Users/mjwybrow/ws-git/dunnart/MAPK.layout"),
                tableWidget->item(row, 0)->text(),
                tableWidget->item(row, 1)->text());
    this->hide();
    //qDebug() << "### " << row << tableWidget->item(row, 0)->text() << ", " << tableWidget->item(row, 1)->text();
}

void OpenInCompatibleSoftwareWidget::changeCanvas(dunnart::Canvas *canvas)
{
    if (m_canvas)
    {
        disconnect(m_canvas, 0, this, 0);
        disconnect(this, 0, m_canvas, 0);
    }
    m_canvas = canvas;

    connect(m_canvas, SIGNAL(diagramFilenameChanged(const QFileInfo& title)),
            m_canvas, SLOT(diagramFilenameChanged(const QFileInfo& title)));
}

void OpenInCompatibleSoftwareWidget::diagramFilenameChanged(
        const QFileInfo& fileInfo)
{

}

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
