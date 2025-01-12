#include "coreengine/filesupport.h"
#include "coreengine/settings.h"
#include "coreengine/gameconsole.h"

#include <QDirIterator>
#include <QCoreApplication>

const char* const Filesupport::LIST_FILENAME_ENDING = ".bl";

QByteArray Filesupport::getHash(const QStringList & filter, const QStringList & folders)
{
    QCryptographicHash myHash(QCryptographicHash::Sha512);
    QStringList fullList;

    QString userPath = Settings::getInstance()->getUserPath();
    for (const auto & folder : std::as_const(folders))
    {
        fullList.append(oxygine::Resource::RCC_PREFIX_PATH + folder);
        fullList.append(userPath + folder);
        if (!userPath.isEmpty())
        {
            fullList.append(folder);
        }
    }
    for (const auto & folder : std::as_const(fullList))
    {
        CONSOLE_PRINT_MODULE("Adding files for folder: " + folder, GameConsole::eDEBUG, GameConsole::eFileSupport);
        addHash(myHash, folder, filter);
    }
    return myHash.result();
}

void Filesupport::addHash(QCryptographicHash & hash, const QString & folder, const QStringList & filter)
{
    QDir dir(folder);
    auto list = dir.entryInfoList(filter, QDir::Files);
    for (auto & item : list)
    {
        QString filePath = item.filePath();
        CONSOLE_PRINT_MODULE("Adding file: " + filePath + " to hash", GameConsole::eDEBUG, GameConsole::eFileSupport);
        QFile file(filePath);
        file.open(QIODevice::ReadOnly);
        while (!file.atEnd())
        {
            hash.addData(file.readLine().trimmed());
        }
        file.close();
    }
    list = dir.entryInfoList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto & item : list)
    {
        QString path = item.filePath();
        addHash(hash, path, filter);
    }
}

QByteArray Filesupport::getRuntimeHash(const QStringList & mods)
{
    QStringList folders = mods;
    folders.append("resources/scripts");
    folders.append("resources/aidata");
    QStringList filter = {"*.js", "*.csv"};
    return getHash(filter, folders);
}

void Filesupport::writeByteArray(QDataStream& stream, const QByteArray& array)
{
    stream << static_cast<qint32>(array.size());
    for (qint32 i = 0; i < array.size(); i++)
    {
        stream << static_cast<qint8>(array[i]);
    }
}

void Filesupport::writeBytes(QDataStream& stream, const QByteArray& array)
{
    for (qint32 i = 0; i < array.size(); i++)
    {
        stream << static_cast<qint8>(array[i]);
    }
}

QByteArray Filesupport::readByteArray(QDataStream& stream)
{
    QByteArray array;
    qint32 size = 0;
    stream >> size;
    for (qint32 i = 0; i < size; i++)
    {
        qint8 value = 0;
        stream >> value;
        array.append(value);
    }
    return array;
}

void Filesupport::storeList(const QString & file, const QStringList & items, const QString & folder)
{
    QDir dir(folder);
    dir.mkpath(".");
    QFile dataFile(folder + file + LIST_FILENAME_ENDING);
    dataFile.open(QIODevice::WriteOnly);
    QDataStream stream(&dataFile);
    stream.setVersion(QDataStream::Version::Qt_6_5);
    stream << file;
    stream << static_cast<qint32>(items.size());
    for (qint32 i = 0; i < items.size(); i++)
    {
        stream << items[i];
    }
}

Filesupport::StringList Filesupport::readList(const QString & file, const QString & folder)
{
    return readList(folder + file);
}

Filesupport::StringList Filesupport::readList(const QString & file)
{
    QFile dataFile(file);
    StringList ret;
    if (dataFile.exists())
    {
        dataFile.open(QIODevice::ReadOnly);
        QDataStream stream(&dataFile);
        stream.setVersion(QDataStream::Version::Qt_6_5);
        stream >> ret.name;
        qint32 size = 0;
        stream >> size;
        for (qint32 i = 0; i < size; i++)
        {
            QString name;
            stream >> name;
            ret.items.append(name);
        }
    }
    else
    {
        CONSOLE_PRINT("Unable to open file: " + file + " using empty list", GameConsole::eWARNING);
    }
    return ret;
}

const char* const Filesupport::RESOURCE_NOT_FOUND = ":/file_should_not_exist";

QString Filesupport::locateResource(const QString& name) {
#ifndef USEAPPCONFIGPATH
    // USEAPPCONFIGPATH is primarily set on Linux, where the "current directory" of programs launched from the start
    // menu is normally the user's home directory. This is very unexpected behavior, and this should not be checked.
    if (QFile::exists(name)) return name;
#endif

    // Check for the file in the user path.
    QString newPath = Settings::getInstance()->getUserPath() + name;
    if (QFile::exists(newPath)) return newPath;

    // Check if we need to find resources deployed as a folder.
#ifdef DEPLOY_RESOURCES_AS_FOLDER
    static QString exeDir = QCoreApplication::applicationDirPath() + "/";
    newPath = exeDir + name;
    if (QFile::exists(newPath)) return newPath;

#ifdef __linux__
    static QString linuxExeDir = QCoreApplication::applicationDirPath() + "/../share/commander_wars/";
    newPath = linuxExeDir + name;
    if (QFile::exists(newPath)) return newPath;
#endif
#endif

    // Check for the file in the built-in resources.
    newPath = oxygine::Resource::RCC_PREFIX_PATH + name;
    if (QFile::exists(newPath)) return newPath;

    // Resource not found, return a sentinel value.
    return RESOURCE_NOT_FOUND;
}

QStringList Filesupport::createSearchPath(const QString& name, bool firstPriority) {
    QStringList searchFolders;

    // Check for the file in the built-in resources.
    searchFolders.append(QString(oxygine::Resource::RCC_PREFIX_PATH) + "resources/" + name);

    // Check if we need to find resources deployed as a folder.
#ifdef DEPLOY_RESOURCES_AS_FOLDER
    static QString exeDir = QCoreApplication::applicationDirPath() + "/";
    searchFolders.append(exeDir + "resources/" + name);

#ifdef __linux__
    static QString linuxExeDir = QCoreApplication::applicationDirPath() + "/../share/commander_wars/";
    searchFolders.append(linuxExeDir + "resources/" + name);
#endif
#endif

    // Check for the file in the user path.
    searchFolders.append(Settings::getInstance()->getUserPath() + "resources/" + name);

    // Add search directories from mods
    QStringList mods = Settings::getInstance()->getMods();
    for (const auto & mod : std::as_const(mods))
    {
        searchFolders.append(mod + "/" + name);
    }

    // Returns the final search path
    if (firstPriority) std::reverse(searchFolders.begin(), searchFolders.end());
    return std::move(searchFolders);
}
