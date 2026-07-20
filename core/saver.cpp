/*
#    FVD++, an advanced coaster design tool for NoLimits
#    Copyright (C) 2012-2015, Stephan "Lenny" Alt <alt.stephan@web.de>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "saver.h"

#include <fstream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include "exportfuncs.h"

using namespace std;

saver::saver(const QString& fileName, projectWidget* _project, QMainWindow* _parent)
{
    sFileName = fileName;
    project = _project;
    parent = _parent;
}

QString saver::doSave()
{
    const QFileInfo targetInfo(sFileName);
    QDir targetDir = targetInfo.absoluteDir();
    if(!targetDir.exists()) {
        return QString("Error: Save folder does not exist: %1").arg(targetDir.absolutePath());
    }

    QTemporaryFile temporary(targetDir.filePath(QString(".%1.XXXXXX.tmp").arg(targetInfo.fileName())));
    temporary.setAutoRemove(true);
    if(!temporary.open()) {
        return QString("Error: Could not create a temporary save file: %1").arg(temporary.errorString());
    }
    const QString temporaryName = temporary.fileName();
    temporary.close();

    const QByteArray encodedTemporaryName = QFile::encodeName(temporaryName);
    fstream fout(encodedTemporaryName.constData(), ios::out | ios::binary | ios::trunc);
    if(!fout) {
        return QString("Error: Could not open the temporary save file");
    }

    const QString result = project->saveProject(fout);

    fout.flush();
    if(!fout.good()) {
        fout.close();
        return QString("Error: Failed while writing the project; the previous save was kept");
    }

    fout.close();

    // rename(2) replaces the destination atomically when both files are in
    // the same directory. A failed or interrupted save therefore cannot
    // truncate the user's existing project.
    const QByteArray encodedTargetName = QFile::encodeName(targetInfo.absoluteFilePath());
    if(::rename(encodedTemporaryName.constData(), encodedTargetName.constData()) != 0) {
        return QString("Error: Could not replace the project file: %1")
            .arg(QString::fromLocal8Bit(std::strerror(errno)));
    }
    temporary.setAutoRemove(false);
    return result;
}

QString saver::doLoad()
{
    const QByteArray encodedFileName = QFile::encodeName(sFileName);
    fstream fin(encodedFileName.constData(), ios::in | ios::binary);
    if(!fin) {
        return QString("Error: Could not open the project file");
    }

    QString temp = project->loadProject(fin);

    fin.close();
    return temp;
}
