#ifndef __tatysoundviewer_H__
#define __tatysoundviewer_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#define UPDATE_INTERVAL	10

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "tatysoundviewer"

#if !defined(PACKAGE)
#define PACKAGE "be.wesdec.tatysoundviewer"
#endif

#endif /* __tatysoundviewer_H__ */
