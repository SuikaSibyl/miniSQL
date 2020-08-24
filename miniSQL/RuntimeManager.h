#ifndef RUNTIMEMANAGER_H
#define RUNTIMEMANAGER_H

#include "BufferManager.h"
#include "IndexManager.h"
#include "RecordManager.h"
#include "CatalogManager.h"

class RuntimeManager
{
public:
    RecordManager* rm;
    IndexManager* im;
    BufferManager* bm;
    CatalogManager* cm;

    RuntimeManager()
    {
        im = new IndexManager();
        bm = new BufferManager();
        rm = new RecordManager(bm,im);
        cm = new CatalogManager();
    }
};

extern RuntimeManager runtimManager;

#endif // RUNTIMEMANAGER_H
