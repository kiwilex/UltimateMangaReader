#ifndef MANGAPANDA_H
#define MANGAPANDA_H

#include "abstractmangasource.h"
#include "mangachapter.h"
#include "mangainfo.h"

class MangaPanda : public AbstractMangaSource
{
public:
    MangaPanda(QObject *parent, DownloadManager *dm);

    bool updateMangaList() override;
    QSharedPointer<MangaInfo> getMangaInfo(const QString &mangalink) override;
    void updateMangaInfoFinishedLoading(
        QSharedPointer<DownloadStringJob> job,
        QSharedPointer<MangaInfo> info) override;
    QStringList getPageList(const QString &chapterlink) override;
    QString getImageLink(const QString &pagelink) override;
};

#endif  // MANGAPANDA_H
