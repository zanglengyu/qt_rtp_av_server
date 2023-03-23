/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "WriteDataDevice.h"

#include <QDebug>
#include <QtEndian>

#include "audiocapture.h"
WriteDataDevice::WriteDataDevice(QObject* parent) :
    QIODevice(parent)

{}

qint64 WriteDataDevice::readData(char* data, qint64 maxlen) {
    std::unique_lock<std::mutex> locker(buffer_mutex_);

    int buffer_size = kAudioBuffer * RTP_AUDIO_LENGTH;
    if (buffer_.size() < buffer_size) {
        return 0;
    }
    memcpy(data, buffer_.data(), maxlen);
    buffer_.remove(0, maxlen);
    return maxlen;
}

qint64 WriteDataDevice::writeData(const char* data, qint64 maxSize) {
    std::unique_lock<std::mutex> locker(buffer_mutex_);

    buffer_.append(data, maxSize);
    qDebug() << "WriteDataDevice::writeData size = " << maxSize << endl;
    return maxSize;
}

qint64 WriteDataDevice::read_last_data(char* data, qint64 maxlen) {
    std::unique_lock<std::mutex> locker(buffer_mutex_);

    if (buffer_.size() < maxlen) {
        maxlen = buffer_.size();
    }
    memcpy(data, buffer_.data(), maxlen);
    buffer_.remove(0, maxlen);
    return maxlen;
}

void WriteDataDevice::restart() {
    std::unique_lock<std::mutex> locker(buffer_mutex_);

    buffer_.clear();
}
