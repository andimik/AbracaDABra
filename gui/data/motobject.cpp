/*
 * This file is part of the AbracaDABra project
 *
 * MIT License
 *
 * Copyright (c) 2019-2025 Petr Kopecký <xkejpi (at) gmail (dot) com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "motobject.h"

#include <QDebug>
#include <QLoggingCategory>

#include "dabtables.h"
Q_LOGGING_CATEGORY(motObject, "MOTObject", QtInfoMsg)

MOTEntity::MOTEntity()
{
    reset();
}

bool MOTEntity::isComplete() const
{
    if (m_numSegments < 0)
    {  // last segment was not received yet
        return false;
    }
    else
    { /* last segment was already received and we know expected size => we could have complete entity */
    }

    // lets check if all segments were received

    // [ETSI EN 301 234, 5.1 Segmentation of MOT entities]
    // MOT entities will be split up in segments with equal size. Only the last segment may have a smaller size
    // (to carry the remaining bytes of the MOT entity). Every MOT entity (e.g. every MOT body) can use a different segmentation size.
    int lastSegmentSize = m_segments.last().size();

    if (m_numSegments != m_segments.size())
    {
        qCWarning(motObject) << "numSegments != segments.size()";
    }

    for (int n = 0; n < m_numSegments - 1; ++n)
    {
        if (m_segments.at(n).size() < lastSegmentSize)
        {  // some segment is smaller than last segment thus not received
            return false;
        }
    }
    return true;
}

int MOTEntity::size()
{
    int receivedSize = 0;
    for (int n = 0; n < m_numSegments; ++n)
    {
        receivedSize += m_segments[n].size();
    }

    return receivedSize;
}

void MOTEntity::addSegment(const uint8_t *segment, uint16_t segmentNum, uint16_t segmentSize, bool lastFlag)
{
    if ((segmentNum >= 8192) || (segmentSize == 0))
    {
        return;
    }
    else
    { /* continue with adding */
    }

    if (lastFlag)
    {  // current segment is marked as last, thus we know number of segments
        m_numSegments = segmentNum + 1;
    }
    else
    { /* do nothing */
    }

    // all segments have the same size only the last can be shorter
    if (segmentNum > m_segments.size())
    {  // insert empty items
        for (int n = m_segments.size(); n < segmentNum; ++n)
        {
            m_segments.append(QByteArray());
        }
    }
    else
    { /* do nothing - segment is somewhere in the middle */
    }

    if (segmentNum < m_segments.size())
    {
        if (m_segments.at(segmentNum).size() != segmentSize)
        {  // current segment size is diffrent than recieved size, it could happen when no segment #segmentNum was received yet
            m_segments.replace(segmentNum, QByteArray((const char *)segment, segmentSize));
        }
        else
        { /* do nothing - segment was already received before */
        }
    }
    else if (segmentNum == m_segments.size())
    {  // add segment to the end
        m_segments.append(QByteArray((const char *)segment, segmentSize));
    }
}

void MOTEntity::reset()
{
    m_segments.clear();
    m_numSegments = -1;
}

QByteArray MOTEntity::getData() const
{
    return m_segments.join();
}

MOTObjectData::MOTObjectData(int_fast32_t transportId)
{
    m_id = transportId;
    m_bodySize = -1;
    m_objectIsComplete = false;
    m_objectIsObsolete = false;
}

MOTObjectData::MOTObjectData(const MOTObjectData &other) : QSharedData(other)
{
    m_id = other.m_id;
    m_bodySize = other.m_bodySize;
    m_objectIsComplete = other.m_objectIsComplete;
    m_objectIsObsolete = other.m_objectIsObsolete;

    m_contentType = other.m_contentType;
    m_contentSubType = other.m_contentSubType;
    m_contentName = other.m_contentName;

    m_header = other.m_header;
    m_body = other.m_body;

    m_userAppParams = other.m_userAppParams;
}

MOTObject::MOTObject(int_fast32_t transportId)
{
    d = new MOTObjectData(transportId);
}

void MOTObjectData::parseHeader()
{
    const QByteArray headerData = m_header.getData();

    // [ETSI EN 301 234, 6.1 Header core]
    // minium header size is 56 bits => 7 bytes (header core)
    if (headerData.size() < 7)
    {
        qCWarning(motObject) << "Unexpected header length";
        m_objectIsComplete = false;
        m_bodySize = -1;
    }

    // unsigned required
    const uint8_t *dataPtr = reinterpret_cast<const uint8_t *>(headerData.constBegin());

    // we know that at least header core was received
    // first check header size
    int headerSize = ((dataPtr[3] & 0x0F) << 9) | (dataPtr[4] << 1) | ((dataPtr[5] >> 7) & 0x01);

    // check is headerSize matches
    if (headerSize < headerData.size())
    {  // header size is not correct -> probably not received yet, but it should not happen
        m_objectIsComplete = false;
        m_bodySize = -1;
    }

    // it seems to be OK, we can parse the information
    m_bodySize = (dataPtr[0] << 20) | (dataPtr[1] << 12) | (dataPtr[2] << 4) | ((dataPtr[3] >> 4) & 0x0F);
    m_contentType = (dataPtr[5] >> 1) & 0x3F;
    m_contentSubType = ((dataPtr[5] & 0x01) << 8) | dataPtr[6];

    bool isOk = true;
    int n = 7;
    while (n < headerSize)
    {
        uint8_t PLI = (dataPtr[n] >> 6) & 0x03;
        uint8_t paramId = dataPtr[n++] & 0x3F;
        uint_fast8_t dataFieldLen = 0;

        switch (PLI)
        {
            case 0:
                // do nothing here
                break;
            case 1:
                dataFieldLen = 1;
                break;
            case 2:
                dataFieldLen = 4;
                break;
            case 3:
                if (n + 1 < headerSize)
                {
                    uint16_t dataLengthIndicator = dataPtr[n] & 0x7F;
                    if (dataPtr[n++] & 0x80)
                    {
                        if (n < headerSize)
                        {
                            dataLengthIndicator <<= 8;
                            dataLengthIndicator |= dataPtr[n++];
                        }
                        else
                        {  // somethign is wrong
                            isOk = false;
                            break;
                        }
                    }
                    dataFieldLen = dataLengthIndicator;
                }
                break;
        }

        // segment[n] is first byte of data field if dataFieldLen > 0

        if (n + dataFieldLen <= headerSize)
        {
#if MOTOBJECT_VERBOSE
            QString dataStr;
            for (int d = 0; d < dataFieldLen; ++d)
            {
                dataStr += QString("%1 ").arg((uint8_t)dataPtr[n + d], 2, 16, QLatin1Char('0'));
            }
            qCDebug(motObject, "%s: PLI=%d, ParamID = 0x%2.2X, DataLength = %d: DataField = %s", Q_FUNC_INFO, PLI, paramId, dataFieldLen,
                    dataStr.toStdString().c_str());
#endif  // MOTOBJECT_VERBOSE

            switch (DabMotExtParameter(paramId))
            {
                case DabMotExtParameter::ContentName:
                    // One MOT parameter is mandatory for both content provider and MOT decoder: ContentName.
                    m_contentName = DabTables::convertToQString((const char *)(dataPtr + n + 1), ((dataPtr[n] >> 4) & 0x0F), dataFieldLen - 1);
#if MOTOBJECT_VERBOSE
                    qCDebug(motObject) << contentName;
#endif
                    break;

                // [ETSI EN 301 234, 6.3 List of all MOT parameters in the MOT header extension]
                // Every MOT decoder shall check if an MOT body is compressed (MOT parameter CompressionType)
                // or scrambled (MOT parameter CAInfo). The MOT decoder does not necessarily (i.e. unless required
                // by the user application) have to be able to decompress or unscramble objects,
                // but it shall be able to identify and discard objects that it can not process.
                case DabMotExtParameter::CAInfo:
                    // ignoring scrembled data
                    qCWarning(motObject) << "MOT CA scrambled ignoring";
                    m_bodySize = -1;
                    isOk = false;
                    break;
                case DabMotExtParameter::CompressionType:
                    // ignoring compressed data
                    qCWarning(motObject) << "MOT compressed ignoring";
                    m_bodySize = -1;
                    isOk = false;
                    break;
                default:
                    // some user app parameter or parameter not handled by MOT decoder
#if 0
                if (userAppParams.end() != userAppParams.find(paramId))
                {
                    //qCWarning(motObject) << "Removing duplicate header parameter" << paramId;
                    userAppParams.remove(paramId);
                }
                else
                { /* paramId does not exist */ }
#endif
                    m_userAppParams.insert(paramId, QByteArray((const char *)(dataPtr + n), dataFieldLen));
                    break;
            }
        }
        else
        {  // something went wrong
            isOk = false;
        }

        n += dataFieldLen;
    }

    if (!isOk)
    {
        m_objectIsComplete = false;
        m_bodySize = -1;
    }
}

bool MOTObject::addSegment(const uint8_t *segment, uint16_t segmentNum, uint16_t segmentSize, bool lastFlag, bool isHeader)
{
    if (isHeader)
    {
        d->m_header.addSegment(segment, segmentNum, segmentSize, lastFlag);
        // lets check is header is complete
        if (d->m_header.isComplete())
        {  // header is complete -> lets set parameters for the object
            d->parseHeader();
            //            if (!parseHeader(d->header.getData()))
            //            {   // something is wrong - header could not be parsed, objects is not complete
            //                d->objectIsComplete = false;
            //                d->bodySize = -1;
            //            }
        }
        else
        { /* header not complete yet */
        }
    }
    else
    {
        d->m_body.addSegment(segment, segmentNum, segmentSize, lastFlag);
    }

    if (d->m_bodySize >= 0)
    {  // header was already received
        // lets check if we already have complete MOT object
        if (d->m_body.isComplete())
        {
            if (d->m_body.size() == d->m_bodySize)
            {  // correct, MOT object is complete
                d->m_objectIsComplete = true;
            }
            else
            {  // [ETSI EN 301 234, 6.1 Header core]
                // BodySize: This 28-bit field, coded as an unsigned binary number, indicates the total size of the body in bytes.
                // If the body size signalled by this parameter does not correspond to the size of the reassembled MOT body, then the
                // MOT body shall be discarded.
                d->m_body.reset();
                d->m_objectIsComplete = false;
            }
        }
    }

    return d->m_objectIsComplete;
}

QByteArray MOTObject::getBody() const
{
    if (d->m_objectIsComplete)
    {  // MOT object is complete
        return d->m_body.getData();
    }

    return QByteArray();
}

uint16_t MOTObject::getContentType() const
{
    return d->m_contentType;
}

uint16_t MOTObject::getContentSubType() const
{
    return d->m_contentSubType;
}

const QString &MOTObject::getContentName() const
{
    return d->m_contentName;
}

MOTDirectory::MOTDirectory(uint_fast32_t transportId, MOTObjectCache *cachePtr)
{
    m_id = transportId;

    // cache becomes carousel in MOT directory
    // decoder is still adding segments, even segments that do not exist in directoy yet
    // crousel/cache maintenence is performed when new directory is received
    m_carousel = cachePtr;
    m_numComplete = 0;
}

// retrurn true is directory is just completed
bool MOTDirectory::addSegment(const uint8_t *segment, uint16_t segmentNum, uint16_t segmentSize, bool lastFlag)
{
    if (!m_dir.isComplete())
    {
        m_dir.addSegment(segment, segmentNum, segmentSize, lastFlag);
        if (m_dir.isComplete())
        {
            qCDebug(motObject) << "MOT directory is complete";
            if (parse(m_dir.getData()))
            {
                return true;
            }
            // something is wrong - header could not be parsed, objects is not complete
            qCWarning(motObject) << "MOT directory parsing failed";
        }
        else
        {
            qCDebug(motObject) << "MOT directory segment received, not complete yet";
        }
    }
    return false;
}

// returns true if object is completed
bool MOTDirectory::addObjectSegment(uint_fast32_t transportId, const uint8_t *segment, uint16_t segmentNum, uint16_t segmentSize, bool lastFlag)
{  // first find if object already exists in carousel
    MOTObjectCache::iterator it = m_carousel->findMotObj(transportId);
    if (m_carousel->end() == it)
    {  // object does not exist in carousel - this should not happen for current directory
        qCDebug(motObject) << "New MOT object" << transportId << "number of objects in carousel" << m_carousel->size();

        // add new object to cache
        it = m_carousel->addMotObj(MOTObject(transportId));
    }
    else
    { /* do nothing - it already exists, just adding next segment */
    }

    if (!it->isComplete())
    {
        it->addSegment(segment, segmentNum, segmentSize, lastFlag);
        if (it->isComplete())
        {
            m_numComplete += 1;
            qCDebug(motObject) << "MOT complete: ID" << transportId;
            return true;
        }
    }
    return false;
}

bool MOTDirectory::parse(const QByteArray &dirData)
{
    // [ETSI EN 301 234, 7.2.3 MOT directory coding]
    // minimum dir size 13 bytes
    if (dirData.size() < 13)
    {
        qCWarning(motObject) << "Unexpected MOT directory length";
        return false;
    }

    // unsigned required
    const uint8_t *dataPtr = reinterpret_cast<const uint8_t *>(dirData.constBegin());

    // we know that at least directory without extension received
    // first check directory size
    int dirSize = ((dataPtr[0] & 0x3F) << 24) | (dataPtr[1] << 16) | (dataPtr[2] << 8) | dataPtr[3];

    // check is dirSize matches
    if (dirSize > dirData.size())
    {  // header size is not correct -> probably not received yet, but it should not happen
        return false;
    }

#if MOTOBJECT_VERBOSE > 1
    QString dirStr;
    for (int d = 0; d < dirSize; ++d)
    {
        dirStr += QString("%1 ").arg((uint8_t)dataPtr[d], 2, 16, QLatin1Char('0'));
    }
    qCDebug(motObject) << dirStr;
#endif

    int numberOfObjects = (dataPtr[4] << 8) | dataPtr[5];
    int dataCarouselPeriod = (dataPtr[6] << 16) | (dataPtr[7] << 8) | dataPtr[8];
    int segmentSize = ((dataPtr[9] & 0x1F) << 8) | dataPtr[10];
    int directoryExtensionLength = (dataPtr[11] << 8) | dataPtr[12];

    qCDebug(motObject,
            "\tDirectorySize = %d\n"
            "\tNumberOfObjects = %d\n"
            "\tDataCarouselPeriod = %d\n"
            "\tSegmentSize = %d\n"
            "\tDirectoryExtensionLength = %d\n",
            dirSize, numberOfObjects, dataCarouselPeriod, segmentSize, directoryExtensionLength);

    bool ret = true;
    int n = 13;
    while (n < 13 + directoryExtensionLength)
    {
        uint8_t PLI = (dataPtr[n] >> 6) & 0x03;
        uint8_t paramId = dataPtr[n++] & 0x3F;
        uint_fast8_t dataFieldLen = 0;

        switch (PLI)
        {
            case 0:
                // do nothing here
                break;
            case 1:
                dataFieldLen = 1;
                break;
            case 2:
                dataFieldLen = 4;
                break;
            case 3:
                if (n + 1 < directoryExtensionLength)
                {
                    uint16_t dataLengthIndicator = dataPtr[n] & 0x7F;
                    if (dataPtr[n++] & 0x80)
                    {
                        if (n < directoryExtensionLength)
                        {
                            dataLengthIndicator <<= 8;
                            dataLengthIndicator |= dataPtr[n++];
                        }
                        else
                        {  // somethign is wrong
                            ret = false;
                            break;
                        }
                    }
                    dataFieldLen = dataLengthIndicator;
                }
                break;
        }

        // segment[n] is first byte of data field if dataFieldLen > 0

        if (n + dataFieldLen <= 13 + directoryExtensionLength)
        {
#if MOTOBJECT_VERBOSE
            QString dataStr;
            for (int d = 0; d < dataFieldLen; ++d)
            {
                dataStr += QString("%1 ").arg((uint8_t)dataPtr[n + d], 2, 16, QLatin1Char('0'));
            }
            qCDebug(motObject, "%s: PLI=%d, ParamID = 0x%2.2X, DataLength = %d: DataField = %s", Q_FUNC_INFO, PLI, paramId, dataFieldLen,
                    dataStr.toStdString().c_str());
#endif  // MOTOBJECT_VERBOSE
        }
        else
        {  // something went wrong
            ret = false;
        }

        n += dataFieldLen;
    }

    // directory extension is parsed here
    qCDebug(motObject) << "Reading MOT objects";

    // set all object in carousel obsolete
    m_carousel->markAllObsolete();
    m_numComplete = 0;

    int numObjRead = 0;
    while (n < dirSize)
    {
        if (numObjRead++ >= numberOfObjects)
        {  // something is wrong - this is a protection condition
            qCWarning(motObject) << "Unexpected number of objects in MOT directory";
            break;
        }

        int objTransportID = (dataPtr[n] << 8) | dataPtr[n + 1];
        int headerSize = ((dataPtr[n + 2 + 3] & 0x0F) << 9) | (dataPtr[n + 2 + 4] << 1) | ((dataPtr[n + 2 + 5] >> 7) & 0x01);
        qCDebug(motObject) << "\t* ID" << objTransportID << "| header size " << headerSize;

        // mark all objects in directory as active (non-obsolete)
        MOTObjectCache::iterator it = m_carousel->markObjObsolete(objTransportID, false);
        if (m_carousel->end() == it)
        {  // not found create new object in carousel
            qCDebug(motObject) << "Object not found in the cache: ID" << objTransportID;
            it = m_carousel->addMotObj(MOTObject(objTransportID));
        }
        else
        { /* do nothing - object is marked as active (non-obsolete) */
        }

        // add header segment
        // number 0, last = true, size is the rest of the directory, object takes what it needs
        it->addSegment((const uint8_t *)(dataPtr + n + 2), 0, headerSize, true, true);
        n += 2 + headerSize;

        if (it->isComplete())
        {
            m_numComplete += 1;
        }
    }

    // done -> delete all remaining obsolete objects
    m_carousel->deleteObsolete();

#if MOTOBJECT_VERBOSE
    qCDebug(motObject) << "MOT directory carousel contents:";
    for (MOTObjectCache::const_iterator it = m_carousel->cbegin(); it < m_carousel->cend(); ++it)
    {
        qCDebug(motObject, "\tID: %d, isComplete = %d, body size = %lld, name = %s", it->getId(), it->isComplete(), it->getBody().size(),
                it->getContentName().toLocal8Bit().data());

        if (it->isComplete())
        {
            qCDebug(motObject, "\t\t%2.2X %2.2X %2.2X %2.2X %2.2X %2.2X ", uint8_t(it->getBody().at(0)), uint8_t(it->getBody().at(1)),
                    uint8_t(it->getBody().at(2)), uint8_t(it->getBody().at(3)), uint8_t(it->getBody().at(5)), uint8_t(it->getBody().at(5)));
        }
    }
#endif  // MOTOBJECT_VERBOSE

    return ret;
}

//=================================================================================
MOTObjectCache::MOTObjectCache()
{}

MOTObjectCache::~MOTObjectCache()
{
    clear();
}

void MOTObjectCache::clear()
{
    m_cache.clear();
}

MOTObjectCache::iterator MOTObjectCache::findMotObj(uint16_t transportId)
{
    MOTObjectCache::iterator it;
    for (it = m_cache.begin(); it < m_cache.end(); ++it)
    {
        if (it->getId() == transportId)
        {
            return it;
        }
    }
    return it;
}

MOTObjectCache::const_iterator MOTObjectCache::cfindMotObj(uint16_t transportId)
{
    MOTObjectCache::const_iterator it;
    for (it = m_cache.cbegin(); it < m_cache.cend(); ++it)
    {
        if (it->getId() == transportId)
        {
            return it;
        }
    }
    return it;
}

void MOTObjectCache::deleteMotObj(uint16_t transportId)
{
    for (int n = 0; n < m_cache.size(); ++n)
    {
        if (m_cache[n].getId() == transportId)
        {
            m_cache.removeAt(n);
            return;
        }
    }
}

MOTObjectCache::iterator MOTObjectCache::addMotObj(const MOTObject &obj)
{
    m_cache.append(obj);
    return --(m_cache.end());
}

void MOTObjectCache::markAllObsolete()
{
    for (int n = 0; n < m_cache.size(); ++n)
    {
        m_cache[n].setObsolete(true);
    }
}

MOTObjectCache::iterator MOTObjectCache::markObjObsolete(uint16_t transportId, bool obsolete)
{
    MOTObjectCache::iterator it;
    for (it = m_cache.begin(); it < m_cache.end(); ++it)
    {
        if (it->getId() == transportId)
        {
            it->setObsolete(obsolete);
            return it;
        }
    }
    return it;
}

void MOTObjectCache::deleteObsolete()
{
    QList<MOTObject>::iterator it = m_cache.begin();
    while (it != m_cache.end())
    {
        if (it->isObsolete())
        {
            it = m_cache.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
