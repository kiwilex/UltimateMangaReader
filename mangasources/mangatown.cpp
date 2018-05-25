#include "mangatown.h"

#include "configs.h"
#include "QTime"

MangaTown::MangaTown(QObject *parent, DownloadManager *dm):
    AbstractMangaSource(parent)
{
    AbstractMangaSource::nummangas = 0;
    AbstractMangaSource::name = "MangaTown";
    AbstractMangaSource::baseurl = "https://www.mangatown.com";
    AbstractMangaSource::downloadmanager = dm;
}

bool MangaTown::updateMangaList()
{
    QTime timer;
    timer.start();


    QString basedictlink = AbstractMangaSource::baseurl + "/directory/0-0-0-0-0-0/";
    QString basedictsuffix = ".htm?name.az";

    DownloadStringJob *job = AbstractMangaSource::downloadmanager->downloadAsString(basedictlink + "1" + basedictsuffix);

    if (!job->await(5000))
        return false;

    mangalist.links.clear();
    mangalist.titles.clear();

    qDebug() << "time" << timer.elapsed();


    QRegExp chsrx("(\\d+)</a><select");
    int cpos = job->buffer.indexOf("<div class=\"next-page\">");
    chsrx.indexIn(job->buffer, cpos);
    int pages = chsrx.cap(1).toInt();

//    pages = 1;

//    qDebug() << pages;

    QVector<DownloadStringJob *> jobs = QVector<DownloadStringJob *>(pages);
    jobs[0] = job;


    QRegExp rx("<P class=\"title\">[^\"]*\"([^\"]*)\" title=\"([^\"]*)");

    nummangas = 0;
    for (int batch = 0, dli = 1, rxi = 0; batch < (pages + maxparalleldownloads - 1) / maxparalleldownloads; batch++)
    {
        for (; dli < (batch + 1) * maxparalleldownloads && dli < pages; dli++)
        {
            jobs[dli] = AbstractMangaSource::downloadmanager->downloadAsString(basedictlink + QString::number(dli + 1) + basedictsuffix);
            qDebug() << "dl" << dli << "time:" << timer.elapsed();
        }
//        for (rxi = (batch + 1) * maxparalleldownloads - 1; rxi > (batch) * maxparalleldownloads && rxi > 0; rxi--)
        for (; rxi < (batch + 1) * maxparalleldownloads && rxi < pages; rxi++)
        {
            if (!jobs[rxi]->await(15000, true))
                return false;

            for (int pos = 0; (pos = rx.indexIn(jobs[rxi]->buffer, pos)) != -1; pos += rx.matchedLength())
            {
                QString l = rx.cap(1);
                l = l.right(l.length() - 19);
                mangalist.links.append(l);
                mangalist.titles.append(rx.cap(2));
                nummangas++;
//                qDebug() << l;
            }

            qDebug() << "rx" << rxi << "time:" << timer.elapsed();
            delete jobs[rxi];
        }
        emit progress(100 * rxi / pages);
    }


    emit progress(100);
    qDebug() << "time" << timer.elapsed();
    return true;
}


MangaInfo *MangaTown::getMangaInfo(QString mangalink)
{

    if (mangalink.left(5) != "https")
        mangalink = AbstractMangaSource::baseurl + mangalink;
    DownloadStringJob *job = AbstractMangaSource::downloadmanager->downloadAsString(mangalink);

    MangaInfo *info = new MangaInfo(this, this);
    info->mangasource = this;
    info->hostname = AbstractMangaSource::name;

    info->link = mangalink;

    QRegExp titlerx("<h1 class=\"title-top\">([^<]*)");
    QRegExp authorrx("Author\\(s\\):</b>[^>]*>([^<]*)");
    QRegExp artistrx("Artist\\(s\\):</b>[^>]*>([^<]*)");
    QRegExp statusrx("Status\\(s\\):</b>([^<&]*)");

    QRegExp summaryrx("<span id=\"show\"[^>]*>([^<&]*)");
//    summaryrx.setMinimal(true);


    if (!job->await(3000))
    {
        qDebug() << job->errorString;
        return info;
    }

    int spos = job->buffer.indexOf("<div class=\"article_content\">");

    if (titlerx.indexIn(job->buffer, spos) != -1)
        info->title = titlerx.cap(1).trimmed();
    if (authorrx.indexIn(job->buffer, spos) != -1)
        info->author = authorrx.cap(1);
    if (artistrx.indexIn(job->buffer, spos) != -1)
        info->artist = artistrx.cap(1);
    if (statusrx.indexIn(job->buffer, spos) != -1)
        info->status = statusrx.cap(1);
    if (summaryrx.indexIn(job->buffer, spos) != -1)
        info->summary = summaryrx.cap(1);
    info->summary = info->summary.remove("&quot;").remove("<br />");


    info->releaseyear = "";

    QRegExp coverrx("<div class=\"detail_info clearfix\">[^\"]*\"([^\"]*)");

    QString coverlink;
    if (coverrx.indexIn(job->buffer, spos) != -1)
        coverlink = coverrx.cap(1);

    int ind = coverlink.indexOf('?');
    if (ind == -1)
        ind = coverlink.length();
    QString filetype = coverlink.mid(ind - 4, 4);
    info->coverpath = mangainfodir(name, info->title) + "cover" + filetype;

//    qDebug() << coverlink << filetype;
//    qDebug() << info->coverpath;

    DownloadFileJob *coverjob = AbstractMangaSource::downloadmanager->downloadAsFile(coverlink, info->coverpath);

    QRegExp rx("<a href=\"([^\"]*)\">\\s+([^<]*)[^>]*>\\s*");
    QRegExp rx2("(.*)<span class");
    rx2.setMinimal(true);

//    QRegExp rx("<a href=\"([^\"]*)\">\\s*([^<]*)[^>]*>\\s*[^>]*>([^<]*)[^>]*>[^>]*>([^<]*)");
//    QRegExp rx("<a href=\"([^\"]*)\">[\r\n]*([^<]*)[^>]*>[\r\n]*[^>]*>([^<]*)");

    spos = job->buffer.indexOf("<ul class=\"chapter_list\">");
    int epos = job->buffer.indexOf("<div class=\"comment_content\">");

    info->numchapters = 0;
    for (int pos = spos; (pos = rx.indexIn(job->buffer, pos)) != -1 && pos < epos; pos += rx.matchedLength())
    {

        info->chapters.insert(0, MangaChapter("https:" + rx.cap(1), this));

        QString chname = "";
        if (rx2.indexIn(job->buffer, pos + rx.matchedLength()) != -1)
            chname = rx.cap(2) + rx2.cap(1).remove("<span>").remove("</span>");

        info->chapertitlesreversed.append(chname);
        info->numchapters++;
//        qDebug() << info->chapters[0].chapterlink;
//        qDebug() << rx.cap(2);
    }


    if (coverlink != "" && !coverjob->await(3000))
    {

        info->coverpath = "";
    }

    return info;
}

QStringList *MangaTown::getPageList(const QString &chapterlink)
{
    DownloadStringJob *job = AbstractMangaSource::downloadmanager->downloadAsString(chapterlink);
    QStringList *pageLinks = new QStringList();

    if (!job->await(3000))
        return pageLinks;


    QRegExp rx("var total_pages\\s*=\\s*(\\d*)");

    if (rx.indexIn(job->buffer, 0) == -1)
        return pageLinks;

    int numpages = rx.cap(1).toInt();

    pageLinks->append(chapterlink);

    for (int i = 2; i <= numpages; i++)
    {
//        qDebug() << chapterlink + QString::number(i) + ".html";
        pageLinks->append(chapterlink + QString::number(i) + ".html");
    }

    delete job;
    return pageLinks;
}


QString MangaTown::getImageLink(const QString &pagelink)
{
    DownloadStringJob *job = AbstractMangaSource::downloadmanager->downloadAsString(pagelink);

    if (!job->await(3000))
        return "";

    QRegExp rx("<img src=\"(https://mangatown.secure[^\"]*)");

    if (rx.indexIn(job->buffer, 0) == -1)
        return "";

//    qDebug() << rx.cap(1);

    return rx.cap(1);
}

