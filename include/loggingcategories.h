#ifndef LOGGINGCATEGORIES_H
#define LOGGINGCATEGORIES_H

#include <QLoggingCategory>
#include <iostream>
#include <QTime>

Q_DECLARE_LOGGING_CATEGORY(logDebug)
Q_DECLARE_LOGGING_CATEGORY(logInfo)
Q_DECLARE_LOGGING_CATEGORY(logWarning)
Q_DECLARE_LOGGING_CATEGORY(logCritical)

//logs
enum Logs_Type {log_debug, log_info, log_warning, log_critical };
#endif // LOGGER_H
