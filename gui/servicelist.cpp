/*
 * This file is part of the AbracaDABra project
 *
 * MIT License
 *
  * Copyright (c) 2019-2023 Petr Kopecký <xkejpi (at) gmail (dot) com>
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

#include <QLoggingCategory>
#include "servicelist.h"

Q_LOGGING_CATEGORY(serviceList, "ServiceList", QtInfoMsg)

ServiceList::ServiceList(QObject * parent) : QObject(parent)
{

}

ServiceList::~ServiceList()
{
    clear();
}

void ServiceList::clear()
{
    foreach (ServiceListItem * item, m_serviceList)
    {
        delete item;
    }
    m_serviceList.clear();

    foreach (EnsembleListItem * item, m_ensembleList)
    {
        delete item;
    }
    m_ensembleList.clear();

    emit empty();
}

void ServiceList::addService(const RadioControlEnsemble & e, const RadioControlServiceComponent & s, bool fav, int currentEns)
{
    if (!e.isValid())
    {   // invalid ensemble -> do nothing
        return;
    }

    bool newService = false;
    bool updatedService = false;

    qCInfo(serviceList, "          [%6.6X] %-18s %X : %d", e.ueid, s.label.toLocal8Bit().data(), s.SId.value(), s.SCIdS);

    ServiceListItem * pService = nullptr;
    ServiceListId servId(s);
    ServiceListIterator sit = m_serviceList.find(servId);
    if (m_serviceList.end() == sit)
    {  // not found
        pService = new ServiceListItem(s, fav, currentEns);
        m_serviceList.insert(servId, pService);
        newService = true;
    }
    else
    {  // found
        pService = *sit;
        updatedService = pService->update(s);
    }

    pService->setIsObsolete(false);

    EnsembleListItem * pEns = nullptr;
    ServiceListId ensId(e);
    uint32_t freq = e.frequency;
    EnsembleListIterator eit = m_ensembleList.begin();
    while (eit != m_ensembleList.end())
    {
        if (freq == (*eit)->frequency())
        {   // frequency is equal to ensemble to be added
            if (e.ueid == (*eit)->ueid())
            {   // ensemble is the same
                pEns = *eit;
            }
            else
            {   // different ensemble in channel -> this is not allowed ensemble to be removed
                // construct dummy RadioControlEnsemble so that removeEnsemble interface can be used
                RadioControlEnsemble dummyEnsToBeRemoved;
                dummyEnsToBeRemoved.frequency = (*eit)->frequency();
                dummyEnsToBeRemoved.ueid = (*eit)->ueid();
                dummyEnsToBeRemoved.label = (*eit)->label();
                dummyEnsToBeRemoved.labelShort = (*eit)->shortLabel();

                removeEnsemble(dummyEnsToBeRemoved);

                pEns = nullptr;
            }
            break;
        }
        ++eit;
    }
    if (nullptr == pEns)
    {   // not found
        pEns = new EnsembleListItem(e);
        m_ensembleList.insert(ensId, pEns);
    }

    // we have ens and service item => lets link them together
    pService->addEnsemble(pEns);
    if (pEns->addService(pService))
    {   // new service in ensemble
        emit serviceAddedToEnsemble(pEns->id(), pService->id());
    }
    if (newService)
    {   // emit signal when new service is added
        emit serviceAdded(pService->id());
    }

    if (updatedService)
    {
        emit serviceUpdated(pService->id());
        emit serviceUpdatedInEnsemble(pEns->id(), pService->id());
    }
}

void ServiceList::setServiceFavorite(const ServiceListId & servId, bool ena)
{
    ServiceListIterator sit = m_serviceList.find(servId);
    if (m_serviceList.end() != sit)
    {   // found
        (*sit)->setFavorite(ena);
    }
}

bool ServiceList::isServiceFavorite(const ServiceListId & servId) const
{
    ServiceListConstIterator sit = m_serviceList.find(servId);
    if (m_serviceList.end() != sit)
    {   // found
        return (*sit)->isFavorite();
    }
    return false;
}

int ServiceList::numEnsembles(const ServiceListId & servId) const
{
    if (!servId.isValid())
    {
        return 0;
    }
    else
    {
        ServiceListConstIterator sit = m_serviceList.find(servId);
        if (m_serviceList.end() != sit)
        {   // found
            return (*sit)->numEnsembles();
        }
        return 0;
    }
}

int ServiceList::currentEnsembleIdx(const ServiceListId & servId) const
{
    ServiceListConstIterator sit = m_serviceList.find(servId);
    if (m_serviceList.end() != sit)
    {   // found
        return (*sit)->currentEnsembleIdx();
    }
    return 0;
}

void ServiceList::save(QSettings & settings)
{
    // first sort service list by ID
    // this is needed to restore secondary services correctly
    QVector<uint64_t> idVect;
    for (auto & s : m_serviceList)
    {
        idVect.append(s->id().value());
    }
    std::sort(idVect.begin(), idVect.end());

    settings.beginWriteArray("ServiceList", m_serviceList.size());          
    int n = 0;
    for (auto id : idVect)
    {
        ServiceListIterator it = m_serviceList.find(ServiceListId(id));

        settings.setArrayIndex(n++);
        settings.setValue("SID", (*it)->SId().value());
        settings.setValue("SCIdS", (*it)->SCIdS());
        settings.setValue("Label", (*it)->label());
        settings.setValue("ShortLabel", (*it)->shortLabel());
        settings.setValue("Fav", (*it)->isFavorite());
        settings.setValue("LastEns", (*it)->currentEnsembleIdx());
        settings.beginWriteArray("Ensemble", (*it)->numEnsembles());
        for (int e = 0; e < (*it)->numEnsembles(); ++e)
        {
            settings.setArrayIndex(e);
            settings.setValue("UEID", (*it)->getEnsemble(e)->ueid());
            settings.setValue("Frequency", (*it)->getEnsemble(e)->frequency());
            settings.setValue("Label", (*it)->getEnsemble(e)->label());
            settings.setValue("ShortLabel", (*it)->getEnsemble(e)->shortLabel());
            //settings.setValue("SNR", (*it)->getEnsemble(e)->snr());
        }
        settings.endArray();
    }
    settings.endArray();
}

void ServiceList::load(QSettings & settings)
{
    int numServ = settings.beginReadArray("ServiceList");
    RadioControlServiceComponent item;
    RadioControlEnsemble ens;

    for (int s = 0; s < numServ; ++s)
    {
        bool ok = true;
        settings.setArrayIndex(s);
        // filling only what is necessary
        item.SId.set(settings.value("SID").toUInt(&ok));
        if (!ok)
        {
            qCWarning(serviceList) << "Problem loading SID item:" << s;
            continue;
        }
        item.SCIdS = settings.value("SCIdS").toUInt(&ok);
        if (!ok)
        {
            qCWarning(serviceList) << "Problem loading SCIdS item:" << s;
            continue;
        }
        item.label = settings.value("Label").toString();
        item.labelShort = settings.value("ShortLabel").toString();

        bool fav = settings.value("Fav").toBool();
        int currentEns = settings.value("LastEns").toInt();
        int numEns = settings.beginReadArray("Ensemble");
        for (int e = 0; e < numEns; ++e)
        {
            settings.setArrayIndex(e);
            ens.ueid = settings.value("UEID").toUInt(&ok);
            if (!ok)
            {
                qCWarning(serviceList) << "Problem loading service" << s << "ensemble UEID" << e;
                continue;
            }
            ens.frequency = settings.value("Frequency").toUInt(&ok);
            if (!ok)
            {
                qCWarning(serviceList) << "Problem loading service" << s << "ensemble frequency" << e;
                continue;
            }
            ens.label = settings.value("Label").toString();
            ens.labelShort = settings.value("ShortLabel").toString();
            //float snr = settings.value("SNR").toFloat();
            addService(ens, item, fav, currentEns);

        }       
        settings.endArray();
    }
    settings.endArray();
}

// this marks all services as obsolete
void ServiceList::beginEnsembleUpdate(const RadioControlEnsemble & e)
{
    EnsembleListItem * pEns = nullptr;
    ServiceListId ensId(e);
    EnsembleListIterator eit = m_ensembleList.find(ensId);
    if (m_ensembleList.end() != eit)
    {   // found
        // ensemble to be updated
        (*eit)->beginUpdate();
    }
    else
    {  /* do nothing, ensemble not found */ }
}

// this removed all services marked as obsolete
void ServiceList::endEnsembleUpdate(const RadioControlEnsemble & e)
{
    EnsembleListItem * pEns = nullptr;
    ServiceListId ensId(e);
    EnsembleListIterator eit = m_ensembleList.find(ensId);
    if (m_ensembleList.end() != eit)
    {  // found
        pEns = *eit;
    }
    else
    {   // do nothing, ensemble not found
        return;
    }

    // this removes all services marked as obsolete from the ensemble list
    pEns->endUpdate();

    // not we need to remove all obsolete services from the service list
    ServiceListIterator it = m_serviceList.begin();
    while (it != m_serviceList.end())
    {
        if ((*it)->isObsolete())
        {   // service is obsolete
            qCInfo(serviceList, "Removing service: [%6.6X] %-18s %X : %d", e.ueid, (*it)->label().toLocal8Bit().data(), (*it)->SId().value(), (*it)->SCIdS());

            emit serviceRemovedFromEnsemble(ensId, (*it)->id());

            if (false == (*it)->removeEnsemble(pEns))
            {   // this was the last ensemble -> remove service completely
                emit serviceRemoved((*it)->id());

                delete *it;                    // release memory
                it = m_serviceList.erase(it);  // remove record from hash                
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }
}

void ServiceList::removeEnsemble(const RadioControlEnsemble &e)
{
    ServiceListId ensId(e);
    EnsembleListIterator eit = m_ensembleList.find(ensId);
    if (m_ensembleList.end() != eit)
    {
        qCInfo(serviceList, "Removing ens %6.6X from service list", e.ueid);

        beginEnsembleUpdate(e);
        endEnsembleUpdate(e);

        delete *eit;
        m_ensembleList.erase(eit);

        emit ensembleRemoved(ensId);
    }
}



