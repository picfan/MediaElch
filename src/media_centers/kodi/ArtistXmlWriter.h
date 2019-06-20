#pragma once

#include <QByteArray>
#include <QDomElement>
#include <QString>

class Artist;

namespace kodi {

class ArtistXmlWriter
{
public:
    ArtistXmlWriter(Artist& artist);
    QByteArray getArtistXml();

private:
    Artist& m_artist;
};

} // namespace kodi