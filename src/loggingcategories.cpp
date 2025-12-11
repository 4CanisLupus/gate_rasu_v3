#include "include/loggingcategories.h"

Q_LOGGING_CATEGORY(logDebug,    "Debug")
Q_LOGGING_CATEGORY(logInfo,     "Info")
Q_LOGGING_CATEGORY(logWarning,  "Warning")
Q_LOGGING_CATEGORY(logCritical, "Critical")


void printLogs(QString func_name, QString log_msg, Logs_Type type_log)
{
    std::cout <<"FUNC: "<<func_name.toStdString()<<" MSG:"<<log_msg.toStdString()<<" TYPE:"<<type_log<<std::endl;
    switch( type_log )
    {
        case log_debug: { qDebug(logDebug()) <<QTime::currentTime()<<"Func: "<<func_name<<" MSG:"<<log_msg;}    break;
        case log_info:  { qInfo(logInfo()) <<QTime::currentTime()<<"Func: "<<func_name<<" MSG:"<<log_msg;}    break;
        case log_warning:  { qWarning(logWarning()) <<QTime::currentTime()<<"Func: "<<func_name<<" MSG:"<<log_msg;}    break;
        case log_critical: { qCritical(logCritical()) <<QTime::currentTime()<<"Func: "<<func_name<<" MSG:"<<log_msg;}    break;
    }
}
