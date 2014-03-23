/*
    ClipGrab³
    Copyright (C) Philipp Schmieder
    http://clipgrab.de
    feedback [at] clipgrab [dot] de

    This file is part of ClipGrab.
    ClipGrab is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    ClipGrab is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ClipGrab.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "converter_copy.h"

converter_copy::converter_copy()
{
    this->_modes.append(tr("Original"));
}

void converter_copy::startConversion(QFile* inputFile, QString& target, QString originalExtension, QString /*metaTitle*/, QString /*metaArtist*/, int /*mode*/)
{
    QDir fileCheck;
    if (fileCheck.exists(target + originalExtension))
    {
        int i = 1;
        while (fileCheck.exists(target + "-" + QString::number(i) + originalExtension))
        {
            i++;
        }
        target.append("-");
        target.append(QString::number(i));

    }

    target.append(originalExtension);

    qDebug() << inputFile->fileName() << target;
    if (inputFile->copy(target))
    {
            inputFile->deleteLater();
    }
    else
    {
        emit error( "error writing data to " + target);
    }
    emit conversionFinished();


}

bool converter_copy::isAvailable()
{
    return true;
}

converter* converter_copy::createNewInstance()
{
    return new converter_copy();
}
