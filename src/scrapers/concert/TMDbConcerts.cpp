#include "TMDbConcerts.h"

#include <QDebug>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "network/NetworkRequest.h"
#include "ui/main/MainWindow.h"

namespace mediaelch {
namespace scraper {

TMDbConcerts::TMDbConcerts(QObject* parent) :
    ConcertScraperInterface(parent),
    m_apiKey{"5d832bdf69dcb884922381ab01548d5b"},
    m_locale{"en"},
    m_baseUrl{"http://image.tmdb.org/t/p/"}
{
    m_meta.identifier = TMDbConcerts::ID;
    m_meta.name = "TMDb Concerts";
    m_meta.description = tr("The Movie Database (TMDb) is a community built movie and TV database. "
                            "Every piece of data has been added by our amazing community dating back to 2008. "
                            "TMDb's strong international focus and breadth of data is largely unmatched and "
                            "something we're incredibly proud of. Put simply, we live and breathe community "
                            "and that's precisely what makes us different.");
    m_meta.website = "https://www.themoviedb.org/";
    m_meta.termsOfService = "https://www.themoviedb.org/terms-of-use";
    m_meta.privacyPolicy = "https://www.themoviedb.org/privacy-policy";
    m_meta.help = "https://www.themoviedb.org/talk";
    m_meta.supportedDetails = {            //
        ConcertScraperInfo::Title,         //
        ConcertScraperInfo::Tagline,       //
        ConcertScraperInfo::Rating,        //
        ConcertScraperInfo::Released,      //
        ConcertScraperInfo::Runtime,       //
        ConcertScraperInfo::Certification, //
        ConcertScraperInfo::Trailer,       //
        ConcertScraperInfo::Overview,      //
        ConcertScraperInfo::Poster,        //
        ConcertScraperInfo::Backdrop,      //
        ConcertScraperInfo::Genres,        //
        ConcertScraperInfo::ExtraArts};
    m_meta.supportedLanguages = {"ar-AE",
        "ar-SA",
        "be-BY",
        "bg-BG",
        "bn-BD",
        "ca-ES",
        "ch-GU",
        "cn-CN",
        "cs-CZ",
        "da-DK",
        "de-DE",
        "de-AT",
        "de-CH",
        "el-GR",
        "en-AU",
        "en-CA",
        "en-GB",
        "en-NZ",
        "en-US",
        "eo-EO",
        "es-ES",
        "es-MX",
        "et-EE",
        "eu-ES",
        "fa-IR",
        "fi-FI",
        "fr-CA",
        "fr-FR",
        "gl-ES",
        "he-IL",
        "hi-IN",
        "hu-HU",
        "hr-HR",
        "id-ID",
        "it-IT",
        "ja-JP",
        "ka-GE",
        "kk-KZ",
        "kn-IN",
        "ko-KR",
        "lt-LT",
        "lv-LV",
        "ml-IN",
        "ms-MY",
        "ms-SG",
        "nb-NO",
        "nl-NL",
        "no-NO",
        "pl-PL",
        "pt-BR",
        "pt-PT",
        "ro-RO",
        "ru-RU",
        "si-LK",
        "sk-SK",
        "sl-SI",
        "sq-AL",
        "sr-RS",
        "sv-SE",
        "ta-IN",
        "te-IN",
        "th-TH",
        "tl-PH",
        "tr-TR",
        "uk-UA",
        "vi-VN",
        "zh-CN",
        "zh-HK",
        "zh-TW",
        "zu-ZA"};
    m_meta.defaultLocale = "";

    m_widget = new QWidget(MainWindow::instance());
    m_box = new QComboBox(m_widget);

    // For officially supported languages, see:
    // https://developers.themoviedb.org/3/configuration/get-primary-translations
    m_box->addItem(tr("Arabic"), "ar");
    m_box->addItem(tr("Bulgarian"), "bg");
    m_box->addItem(tr("Chinese (T)"), "zh-TW");
    m_box->addItem(tr("Chinese (S)"), "zh-CN");
    m_box->addItem(tr("Croatian"), "hr");
    m_box->addItem(tr("Czech"), "cs");
    m_box->addItem(tr("Danish"), "da");
    m_box->addItem(tr("Dutch"), "nl");
    m_box->addItem(tr("English"), "en");
    m_box->addItem(tr("English (US)"), "en-US");
    m_box->addItem(tr("Finnish"), "fi");
    m_box->addItem(tr("French"), "fr");
    m_box->addItem(tr("French (Canada)"), "fr-CA");
    m_box->addItem(tr("German"), "de");
    m_box->addItem(tr("Greek"), "el");
    m_box->addItem(tr("Hebrew"), "he");
    m_box->addItem(tr("Hungarian"), "hu");
    m_box->addItem(tr("Italian"), "it");
    m_box->addItem(tr("Japanese"), "ja");
    m_box->addItem(tr("Korean"), "ko");
    m_box->addItem(tr("Norwegian"), "no");
    m_box->addItem(tr("Polish"), "pl");
    m_box->addItem(tr("Portuguese (Brazil)"), "pt-BR");
    m_box->addItem(tr("Portuguese (Portugal)"), "pt-PT");
    m_box->addItem(tr("Russian"), "ru");
    m_box->addItem(tr("Slovene"), "sl");
    m_box->addItem(tr("Spanish"), "es");
    m_box->addItem(tr("Spanish (Mexico)"), "es-MX");
    m_box->addItem(tr("Swedish"), "sv");
    m_box->addItem(tr("Turkish"), "tr");

    auto* layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);

    setup();
}

const ConcertScraperInterface::ScraperMeta& TMDbConcerts::meta() const
{
    return m_meta;
}

bool TMDbConcerts::hasSettings() const
{
    return true;
}

QWidget* TMDbConcerts::settingsWidget()
{
    return m_widget;
}

void TMDbConcerts::loadSettings(ScraperSettings& settings)
{
    m_locale = QLocale(settings.language().toString());
    if (m_locale.name() == "C") {
        m_locale = QLocale("en");
    }

    const QString locale = localeForTMDb();
    const QString lang = language();

    for (int i = 0, n = m_box->count(); i < n; ++i) {
        if (m_box->itemData(i).toString() == lang || m_box->itemData(i).toString() == locale) {
            m_box->setCurrentIndex(i);
            break;
        }
    }
}

/**
 * \brief Saves scrapers settings
 */
void TMDbConcerts::saveSettings(ScraperSettings& settings)
{
    const QString language = m_box->itemData(m_box->currentIndex()).toString();
    settings.setLanguage(language);
    loadSettings(settings);
}

/**
 * \brief Just returns a pointer to the scrapers network access manager
 * \return Network Access Manager
 */
mediaelch::network::NetworkManager* TMDbConcerts::network()
{
    return &m_network;
}

/**
 * \brief Returns a list of infos available from the scraper
 * \return List of supported infos
 */
QSet<ConcertScraperInfo> TMDbConcerts::scraperSupports()
{
    return m_meta.supportedDetails;
}

/**
 * \brief Loads the setup parameters from TMDb
 * \see TMDbConcerts::setupFinished
 */
void TMDbConcerts::setup()
{
    QUrl url(QString("https://api.themoviedb.org/3/configuration?api_key=%1").arg(m_apiKey));
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);
    QNetworkReply* reply = network()->getWithWatcher(request);
    connect(reply, &QNetworkReply::finished, this, &TMDbConcerts::setupFinished);
}

QString TMDbConcerts::localeForTMDb() const
{
    return m_locale.name().replace('_', '-');
}

/**
 * \return Two letter language code (lowercase)
 */
QString TMDbConcerts::language() const
{
    return m_locale.name().split('_').first();
}

/**
 * \return Two or three letter country code (uppercase)
 */
QString TMDbConcerts::country() const
{
    return m_locale.name().split('_').last();
}

/**
 * \brief Called when setup parameters were got
 *        Parses json and assigns the baseUrl
 */
void TMDbConcerts::setupFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(reply->readAll(), &parseError).object();
    reply->deleteLater();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing TMDb setup json " << parseError.errorString();
        return;
    }

    const auto imagesObject = parsedJson.value("images").toObject();
    m_baseUrl = imagesObject.value("base_url").toString();
    qDebug() << "TMDb base url:" << m_baseUrl;
}

/**
 * \brief Searches for a concert
 * \param searchStr The Concert name/search string
 * \see TMDbConcerts::searchFinished
 */
void TMDbConcerts::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    searchStr = QUrl::toPercentEncoding(searchStr);
    QUrl url;
    QRegExp rx("^tt\\d+$");
    QRegExp rxTmdbId("^id\\d+$");
    if (rx.exactMatch(searchStr)) {
        url.setUrl(QStringLiteral("https://api.themoviedb.org/3/movie/%1?api_key=%2&language=%3")
                       .arg(searchStr)
                       .arg(m_apiKey)
                       .arg(localeForTMDb()));
    } else if (rxTmdbId.exactMatch(searchStr)) {
        url.setUrl(QStringLiteral("https://api.themoviedb.org/3/movie/%1?api_key=%2&language=%3")
                       .arg(searchStr.mid(2))
                       .arg(m_apiKey)
                       .arg(localeForTMDb()));
    } else {
        url.setUrl(QStringLiteral("https://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&query=%3")
                       .arg(m_apiKey)
                       .arg(localeForTMDb())
                       .arg(searchStr));
    }
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);
    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("searchString", searchStr);
    reply->setProperty("results", Storage::toVariant(reply, QVector<ScraperSearchResult>()));
    connect(reply, &QNetworkReply::finished, this, &TMDbConcerts::searchFinished);
}

/**
 * \brief Called when the search result was downloaded
 *        Emits "searchDone" if there are no more pages in the result set
 * \see TMDbConcerts::parseSearch
 */
void TMDbConcerts::searchFinished()
{
    auto* searchReply = dynamic_cast<QNetworkReply*>(QObject::sender());
    QVector<ScraperSearchResult> results = searchReply->property("results").value<Storage*>()->results();

    if (searchReply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << searchReply->errorString();
        searchReply->deleteLater();
        emit searchDone(results);
        return;
    }

    QString searchString = searchReply->property("searchString").toString();
    QString msg = QString::fromUtf8(searchReply->readAll());
    int nextPage = -1;
    results.append(parseSearch(msg, nextPage));
    searchReply->deleteLater();

    if (nextPage == -1) {
        emit searchDone(results);
    } else {
        QUrl url(QStringLiteral("https://api.themoviedb.org/3/search/movie?api_key=%1&language=%2&page=%3&query=%4")
                     .arg(m_apiKey)
                     .arg(localeForTMDb())
                     .arg(nextPage)
                     .arg(searchString));
        QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
        request.setRawHeader("Accept", "application/json");
        QNetworkReply* reply = network()->getWithWatcher(request);
        reply->setProperty("searchString", searchString);
        reply->setProperty("results", Storage::toVariant(reply, results));
        connect(reply, &QNetworkReply::finished, this, &TMDbConcerts::searchFinished);
    }
}

/**
 * \brief Parses the JSON search results
 * \param json JSON string
 * \param nextPage This will hold the next page to get, -1 if there are no more pages
 * \return List of search results
 */
QVector<ScraperSearchResult> TMDbConcerts::parseSearch(QString json, int& nextPage)
{
    QVector<ScraperSearchResult> results;

    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing search json " << parseError.errorString();
        return results;
    }

    // only get the first 3 pages
    const int page = parsedJson.value("page").toInt();
    if (page < parsedJson.value("total_pages").toInt() && page < 3) {
        nextPage = page + 1;
    }

    if (parsedJson.value("results").isArray()) {
        const auto jsonResults = parsedJson.value("results").toArray();
        for (const auto& it : jsonResults) {
            const auto resultObj = it.toObject();
            if (resultObj.value("id").toInt() == 0) {
                continue;
            }
            ScraperSearchResult result;
            result.name = resultObj.value("title").toString();
            if (result.name.isEmpty()) {
                result.name = resultObj.value("original_title").toString();
            }
            result.id = QString::number(resultObj.value("id").toInt());
            result.released = QDate::fromString(resultObj.value("release_date").toString(), "yyyy-MM-dd");
            results.append(result);
        }

    } else if (parsedJson.value("id").toInt() > 0) {
        ScraperSearchResult result;
        result.name = parsedJson.value("title").toString();
        if (result.name.isEmpty()) {
            result.name = parsedJson.value("original_title").toString();
        }
        result.id = QString::number(parsedJson.value("id").toInt());
        result.released = QDate::fromString(parsedJson.value("release_date").toString(), "yyyy-MM-dd");
        results.append(result);
    }

    return results;
}

/**
 * \brief Starts network requests to download infos from TMDb
 * \param id TMDb movie ID
 * \param concert Concert object
 * \param infos List of infos to load
 * \see TMDbConcerts::loadFinished
 * \see TMDbConcerts::loadCastsFinished
 * \see TMDbConcerts::loadTrailersFinished
 * \see TMDbConcerts::loadImagesFinished
 * \see TMDbConcerts::loadReleasesFinished
 */
void TMDbConcerts::loadData(TmdbId id, Concert* concert, QSet<ConcertScraperInfo> infos)
{
    qDebug() << "Entered, id=" << id << "concert=" << concert->name();
    concert->setTmdbId(id);
    concert->clear(infos);

    QUrl url;
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(QUrl{});

    QVector<ScraperData> loadsLeft;

    // Infos
    {
        loadsLeft.append(ScraperData::Infos);
        url.setUrl(QStringLiteral("https://api.themoviedb.org/3/movie/%1?api_key=%2&language=%3")
                       .arg(id.toString(), m_apiKey, localeForTMDb()));
        request.setUrl(url);
        QNetworkReply* reply = network()->getWithWatcher(request);
        reply->setProperty("storage", Storage::toVariant(reply, concert));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TMDbConcerts::loadFinished);
    }

    // Trailers
    if (infos.contains(ConcertScraperInfo::Trailer)) {
        loadsLeft.append(ScraperData::Trailers);
        url.setUrl(QString("https://api.themoviedb.org/3/movie/%1/trailers?api_key=%2").arg(id.toString(), m_apiKey));
        request.setUrl(url);
        QNetworkReply* reply = network()->getWithWatcher(request);
        reply->setProperty("storage", Storage::toVariant(reply, concert));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TMDbConcerts::loadTrailersFinished);
    }

    // Images
    if (infos.contains(ConcertScraperInfo::Poster) || infos.contains(ConcertScraperInfo::Backdrop)) {
        loadsLeft.append(ScraperData::Images);
        url.setUrl(QString("https://api.themoviedb.org/3/movie/%1/images?api_key=%2").arg(id.toString(), m_apiKey));
        request.setUrl(url);
        QNetworkReply* reply = network()->getWithWatcher(request);
        reply->setProperty("storage", Storage::toVariant(reply, concert));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TMDbConcerts::loadImagesFinished);
    }

    // Releases
    if (infos.contains(ConcertScraperInfo::Certification)) {
        loadsLeft.append(ScraperData::Releases);
        url.setUrl(QString("https://api.themoviedb.org/3/movie/%1/releases?api_key=%2").arg(id.toString(), m_apiKey));
        request.setUrl(url);
        QNetworkReply* reply = network()->getWithWatcher(request);
        reply->setProperty("storage", Storage::toVariant(reply, concert));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, &QNetworkReply::finished, this, &TMDbConcerts::loadReleasesFinished);
    }
    concert->controller()->setLoadsLeft(loadsLeft);
}

/**
 * \brief Called when the concert infos are downloaded
 * \see TMDbConcerts::parseAndAssignInfos
 */
void TMDbConcerts::loadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Concert* concert = reply->property("storage").value<Storage*>()->concert();
    QSet<ConcertScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->concertInfosToLoad();
    reply->deleteLater();
    if (concert == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, concert, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    concert->controller()->removeFromLoadsLeft(ScraperData::Infos);
}

/**
 * \brief Called when the concert trailers are downloaded
 * \see TMDbConcerts::parseAndAssignInfos
 */
void TMDbConcerts::loadTrailersFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Concert* concert = reply->property("storage").value<Storage*>()->concert();
    QSet<ConcertScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->concertInfosToLoad();
    reply->deleteLater();
    if (concert == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, concert, infos);
    } else {
        qDebug() << "Network Error (trailers)" << reply->errorString();
    }
    concert->controller()->removeFromLoadsLeft(ScraperData::Trailers);
}

/**
 * \brief Called when the concert images are downloaded
 * \see TMDbConcerts::parseAndAssignInfos
 */
void TMDbConcerts::loadImagesFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Concert* concert = reply->property("storage").value<Storage*>()->concert();
    QSet<ConcertScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->concertInfosToLoad();
    reply->deleteLater();
    if (concert == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, concert, infos);
    } else {
        qWarning() << "Network Error (images)" << reply->errorString();
    }
    concert->controller()->removeFromLoadsLeft(ScraperData::Images);
}

/**
 * \brief Called when the concert releases are downloaded
 * \see TMDbConcerts::parseAndAssignInfos
 */
void TMDbConcerts::loadReleasesFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Concert* concert = reply->property("storage").value<Storage*>()->concert();
    QSet<ConcertScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->concertInfosToLoad();
    reply->deleteLater();
    if (concert == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, concert, infos);
    } else {
        qWarning() << "Network Error (releases)" << reply->errorString();
    }
    concert->controller()->removeFromLoadsLeft(ScraperData::Releases);
}

/**
 * \brief Parses JSON data and assigns it to the given concert object
 *        Handles all types of data from TMDb (info, releases, trailers, images)
 * \param json JSON data
 * \param concert Concert object
 * \param infos List of infos to load
 */
void TMDbConcerts::parseAndAssignInfos(QString json, Concert* concert, QSet<ConcertScraperInfo> infos)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing concert info json " << parseError.errorString();
        return;
    }

    // Infos
    if (!parsedJson.value("imdb_id").toString().isEmpty()) {
        concert->setImdbId(ImdbId(parsedJson.value("imdb_id").toString()));
    }
    if (infos.contains(ConcertScraperInfo::Title) && !parsedJson.value("title").toString().isEmpty()) {
        concert->setName(parsedJson.value("title").toString());
    }
    if (infos.contains(ConcertScraperInfo::Overview)) {
        const auto overviewStr = parsedJson.value("overview").toString();
        if (!overviewStr.isEmpty()) {
            concert->setOverview(overviewStr);
        }
    }
    if (infos.contains(ConcertScraperInfo::Rating) && parsedJson.value("vote_average").toDouble(-1) >= 0) {
        Rating rating;
        rating.rating = parsedJson.value("vote_average").toDouble();
        if (concert->ratings().isEmpty()) {
            concert->ratings().push_back(rating);
        } else {
            concert->ratings().first() = rating;
        }
        concert->setChanged(true);
    }
    if (infos.contains(ConcertScraperInfo::Tagline) && !parsedJson.value("tagline").toString().isEmpty()) {
        concert->setTagline(parsedJson.value("tagline").toString());
    }
    if (infos.contains(ConcertScraperInfo::Released) && !parsedJson.value("release_date").toString().isEmpty()) {
        concert->setReleased(QDate::fromString(parsedJson.value("release_date").toString(), "yyyy-MM-dd"));
    }
    if (infos.contains(ConcertScraperInfo::Runtime) && parsedJson.value("runtime").toInt(-1) >= 0) {
        concert->setRuntime(std::chrono::minutes(parsedJson.value("runtime").toInt()));
    }
    if (infos.contains(ConcertScraperInfo::Genres) && parsedJson.value("genres").isArray()) {
        const auto genres = parsedJson.value("genres").toArray();
        for (const auto& it : genres) {
            const auto genre = it.toObject();
            if (genre.value("id").toInt(-1) == -1) {
                continue;
            }
            concert->addGenre(helper::mapGenre(genre.value("name").toString()));
        }
    }

    // Trailers
    if (infos.contains(ConcertScraperInfo::Trailer) && parsedJson.value("youtube").isArray()) {
        // The trailer listed first is most likely also the best.
        const auto firstTrailer = parsedJson.value("youtube").toArray().first().toObject();
        if (!firstTrailer.value("source").toString().isEmpty()) {
            const QString youtubeSrc = firstTrailer.value("source").toString();
            concert->setTrailer(
                QUrl(helper::formatTrailerUrl(QStringLiteral("https://www.youtube.com/watch?v=%1").arg(youtubeSrc))));
        }
    }

    // Images
    if (infos.contains(ConcertScraperInfo::Backdrop) && parsedJson.value("backdrops").isArray()) {
        const auto backdrops = parsedJson.value("backdrops").toArray();
        for (const auto& it : backdrops) {
            const auto backdrop = it.toObject();
            const QString filePath = backdrop.value("file_path").toString();
            if (filePath.isEmpty()) {
                continue;
            }
            Poster b;
            b.thumbUrl = m_baseUrl + "w780" + filePath;
            b.originalUrl = m_baseUrl + "original" + filePath;
            b.originalSize.setWidth(backdrop.value("width").toInt());
            b.originalSize.setHeight(backdrop.value("height").toInt());
            concert->addBackdrop(b);
        }
    }

    if (infos.contains(ConcertScraperInfo::Poster) && parsedJson.value("posters").isArray()) {
        const auto posters = parsedJson.value("posters").toArray();
        for (const auto& it : posters) {
            const auto poster = it.toObject();
            const QString filePath = poster.value("file_path").toString();
            if (filePath.isEmpty()) {
                continue;
            }
            Poster b;
            b.thumbUrl = m_baseUrl + "w342" + filePath;
            b.originalUrl = m_baseUrl + "original" + filePath;
            b.originalSize.setWidth(poster.value("width").toInt());
            b.originalSize.setHeight(poster.value("height").toInt());
            b.language = poster.value("iso_639_1").toString();
            concert->addPoster(b);
        }
    }

    // Releases
    if (infos.contains(ConcertScraperInfo::Certification) && parsedJson.value("countries").isArray()) {
        Certification locale;
        Certification us;
        Certification gb;
        const auto countries = parsedJson.value("countries").toArray();
        for (const auto& it : countries) {
            const auto countryObj = it.toObject();
            const QString iso3166 = countryObj.value("iso_3166_1").toString();
            const Certification certification = Certification(countryObj.value("certification").toString());
            if (iso3166 == "US") {
                us = certification;
            }
            if (iso3166 == "GB") {
                gb = certification;
            }
            if (iso3166.toUpper() == country()) {
                locale = certification;
            }
        }

        if (us.isValid()) {
            concert->setCertification(helper::mapCertification(us));

        } else if (gb.isValid()) {
            concert->setCertification(helper::mapCertification(gb));

        } else if (locale.isValid()) {
            concert->setCertification(helper::mapCertification(locale));
        }
    }
}

} // namespace scraper
} // namespace mediaelch
