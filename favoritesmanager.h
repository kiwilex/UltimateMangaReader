#ifndef READINGSTATEMANAGER_H
#define READINGSTATEMANAGER_H

#include <QTimer>
#include <QtConcurrent/QtConcurrent>

#include "abstractmangasource.h"
#include "favorite.h"
#include "networkmanager.h"

class FavoritesManager : public QObject
{
    Q_OBJECT
public:
    explicit FavoritesManager(const QMap<QString, AbstractMangaSource *> &mangasources,
                              QObject *parent = nullptr);

    bool isFavorite(QSharedPointer<MangaInfo> info);
    bool toggleFavorite(QSharedPointer<MangaInfo> info);

    void moveFavoriteToFront(int i);
    void updateInfos();
    void loadInfos();
    void serialize();
    void clearFavorites();
    void resetUpdatedStatus();

    QList<QSharedPointer<MangaInfo>> favoriteinfos;
    QList<Favorite> favorites;

private:
    const QMap<QString, AbstractMangaSource *> &mangasources;
    void deserialize();
};

#endif  // READINGSTATEMANAGER_H
