#include "dbmaster.h"
#include "appsettings.h"
#include "config.h"

#include <QMutexLocker>

#ifdef USE_OSMSCOUT_MAP_QT

#include <osmscout/MapPainterQt.h>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QBuffer>

#endif // MAP QT


#ifdef USE_OSMSCOUT_MAP_CAIRO

#include <osmscout/MapPainterCairo.h>

#include <QMutexLocker>
#include <QBuffer>
#include <QDataStream>

// helper function used to write data to memory buffer by cairo
cairo_status_t cairo_write_to_buffer(void *closure,
                                     const unsigned char *data,
                                     unsigned int length)
{
    QDataStream* b = (QDataStream*)closure;
    qint64 l = length;
    if ( l != b->writeRawData((const char*)data, l) )
        return CAIRO_STATUS_WRITE_ERROR;
    return CAIRO_STATUS_SUCCESS;
}

#endif // MAP CAIRO


bool DBMaster::renderMap(bool daylight, double dpi, int zoom_level, int width, int height, double lat, double lon, QByteArray &result)
{
    if (m_error_flag) return false;

    // reading in settings - area protected by mutex
    m_mutex.lock();

    bool renderSea = m_render_sea;
    bool drawBackground = m_draw_background;
    float fontSize = m_font_size;

    std::list<std::string> paths;
    paths.push_back(m_icons_dir);

    if ( !loadStyle(daylight) )
        return false;

    m_mutex.unlock();
    // settings read in - mutex protected area ends

    osmscout::MercatorProjection  projection;
    osmscout::MapParameter        drawParameter;
    osmscout::AreaSearchParameter searchParameter;
    osmscout::MapData             data;

    osmscout::Magnification magnification;
    magnification.SetLevel(zoom_level);

    drawParameter.SetIconPaths(paths);
    //drawParameter.SetPatternPaths(paths);

    drawParameter.SetDebugData(false);
    drawParameter.SetDebugPerformance(false);

    drawParameter.SetFontSize(fontSize);

    // optimize process can reduce number of nodes before rendering
    // it helps for slow renderer backend, but it cost some cpu
    // it seems that it is better to disable it for mobile devices with slow cpu
    drawParameter.SetOptimizeWayNodes(osmscout::TransPolygon::none);
    drawParameter.SetOptimizeAreaNodes(osmscout::TransPolygon::none);

    drawParameter.SetRenderBackground(drawBackground || renderSea);
    drawParameter.SetRenderSeaLand(renderSea);

    // To get accurate label drawing at tile borders, we take into account labels
    // of other than the current tile, too.
    if (zoom_level >= 14)
        // but not for high zoom levels, it is too expensive
        drawParameter.SetDropNotVisiblePointLabels(true);
    else
        drawParameter.SetDropNotVisiblePointLabels(false);

    // https://github.com/Framstag/libosmscout/blob/master/Documentation/RenderTuning.txt
    //searchParameter.SetBreaker(dataLoadingBreaker);
    if (magnification.GetLevel() >= 15)
        searchParameter.SetMaximumAreaLevel(6);
    else
        searchParameter.SetMaximumAreaLevel(4);

    // multithreading is realized by performing calls to
    // this function from multiple threads
    searchParameter.SetUseMultithreading(false);
    searchParameter.SetUseLowZoomOptimization(true);

    projection.Set(osmscout::GeoCoord(lat,lon),
                   0,
                   magnification,
                   dpi,
                   width,
                   height);

    std::cout << "rm: " << dpi << " " << " " << zoom_level << " " << width << "x" << height << " " << lat << " " << lon << "\n";

    projection.SetLinearInterpolationUsage(zoom_level >= 10);

    {
        QMutexLocker lk(&m_mutex);

        std::list<osmscout::TileRef> tiles;

        m_map_service->LookupTiles(projection,tiles);
        m_map_service->LoadMissingTileData(searchParameter,*m_style_config,tiles);
        m_map_service->ConvertTilesToMapData(tiles,data);

        if (drawParameter.GetRenderSeaLand())
            m_map_service->GetGroundTiles(projection, data.groundTiles);
    }

#ifdef USE_OSMSCOUT_MAP_QT
    //    QPixmap *pixmap=new QPixmap(width,height);
    //    if (pixmap == NULL)
    //        return false;
    QImage image(width,height,QImage::Format_RGB32);

    QPainter* painter=new QPainter(&image);
    if (painter == nullptr)
        return false;

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    osmscout::MapPainterQt mapPainter(m_style_config);
#endif

#ifdef USE_OSMSCOUT_MAP_CAIRO
    cairo_surface_t *surface;
    cairo_t         *cairo;

    surface=cairo_image_surface_create(CAIRO_FORMAT_RGB24,width,height);

    if (surface==NULL || (cairo=cairo_create(surface))==NULL)
    {
        std::cerr << "Error allocating cairo" << std::endl;
        if (surface != NULL)
            cairo_surface_destroy(surface);
        return false;
    }

    osmscout::MapPainterCairo mapPainter(m_style_config);
#endif

    bool success = false;

    if (mapPainter.DrawMap(projection,
                           drawParameter,
                           data,
                       #ifdef USE_OSMSCOUT_MAP_QT
                           painter
                       #endif
                       #ifdef USE_OSMSCOUT_MAP_CAIRO
                           cairo
                       #endif
                           )
            )
    {
#ifdef USE_OSMSCOUT_MAP_QT
        QBuffer buffer(&result);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        success = true;
#endif

#ifdef USE_OSMSCOUT_MAP_CAIRO
        QDataStream datastream(&result, QIODevice::WriteOnly);
        if ( cairo_surface_write_to_png_stream (surface,
                                                cairo_write_to_buffer,
                                                &datastream) == CAIRO_STATUS_SUCCESS )
            success = true;
        else
        {
            std::cerr << "Error while writing cairo stream\n";
        }
#endif

    }

#ifdef USE_OSMSCOUT_MAP_QT
    delete painter;
#endif
#ifdef USE_OSMSCOUT_MAP_CAIRO
    cairo_destroy(cairo);
    cairo_surface_destroy(surface);
#endif

    return success;
}