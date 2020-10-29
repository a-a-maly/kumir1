#ifndef __KASSERT_H
#define __KASSERT_H

#include <QDebug>

#define K_ASSERT(cond) if (cond) {} else qDebug("K_ASSERT: %s in %s : %i",#cond,__FILE__,__LINE__);

#endif
