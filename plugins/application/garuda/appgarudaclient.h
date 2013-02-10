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



#ifndef APPGARUDACLIENT_H
#define APPGARUDACLIENT_H

#include <QtCore>
#include <QtNetwork>

class AppGarudaClient : public QObject
{
    Q_OBJECT

    public:
        explicit AppGarudaClient(QObject *parent = 0);

        void deregisterWithCore(void);
        void loadFileIntoSoftware(QFileInfo fileInfo, QString softwareId,
                QString softwareVersion);
        void showCompatibleSoftwareFor(QString extension, QString format);
        bool connectedToServer(void) const;

    signals:
        void error(QTcpSocket::SocketError socketError);
        void newMessage(const QString& message);
        void fileOpenRequest(const QString& filename);
        void showCompatibleSoftwareResponse(const QVariantMap& response);

    public slots:
        void sendData(const QString& data);

    private slots:
        void readData();
        void displayError(QAbstractSocket::SocketError socketError);
        void parseMessage(const QString& message);

    private:
        void processRequest(const QString& request);
        void activateDunnartWithCore(void);
        void registerDunnart(void);
        void openSocket(void);
        void closeSocket(void);

        QTcpSocket *m_tcp_socket;
        int m_socket_descriptor;
        quint16 m_block_size;
        QString m_current_data;
        QString m_app_uuid;
};


#endif // APPGARUDACLIENT_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
