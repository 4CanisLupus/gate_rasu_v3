#ifndef PLC_CONNECTOR_H
#define PLC_CONNECTOR_H

#include "loggingcategories.h"
#include "ws_signal.h"
#include "ws_gr_signal.h"
#include "dtsclient.h"

#include <cstring>
#include <iostream>
#include <math.h>
#include <string.h>
#include <time.h>
#include <thread>
#include <unistd.h>

#include <QObject>
#include <QCoreApplication>
#include <QDateTime>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QUdpSocket>
#include <QRegularExpression>
#include <QSettings>
#include <QtSerialBus/QModbusTcpClient>
#include <QtSerialBus/QModbusReply>
#include <QTimer>
#include <QTime>

//dts
#define ERROR_RET -1
#define OK_RET 0
#define WARNING_RET 1

//функция логгирования сообщений
extern void printLogs(QString func_name, QString log_word, Logs_Type type_log);

//test version 2
static Value value;

class QModbusClient;

class PLC_connector : public QObject
{
    Q_OBJECT


public:
    static PLC_connector* getInstance(QString params, int level);
     //explicit PLC_connector( QString params, int level/*, QObject *parent = nullptr*/);
    explicit PLC_connector( QString params, int level, QObject *parent = nullptr);
    PLC_connector( QObject *parent = nullptr);

    ~PLC_connector();

   // test version 2
    DTSClient client;

    QTimer* m_emit_timer;
    QTimer* m_emit_timer_diag;

    QString m_params{};//режим отладки
    int m_level{}; //уровень отладки (на перспективу)
    const QString WORK = "--WORK";
    const QString DEBUG = "--DEBUG"; // только ключевые точки
    const QString DEBUG_FULL = "--DEBUG_FULL"; // вся отладка
    const QString SIMUL = "--SIMUL"; // имитация
    //plc settings
     QString sig_fileName;
     const QString plc_settings = "plc_setting";    //заглавное поле свойств контроллеров
     const QString default_signal_title = "Signal"; //заглавие начало строки с данными
     const QString default_ip_title = "Host_IP";// имя настройки с ip
     const QString default_port_title = "Port";
     const QString default_server_adr_title = "Server_adress";
     const QString default_1st_regstr_title = "1st_rgstr"; //заглавие номер первого холдинг регистра
     const QString default_num_hld_regstr_title = "Num_rgstr"; //заглавие число холдинг регистров
     const QString default_1st_qual_regstr_title = "1st_qual_rgstr"; //заглавие номер первого регистра с качеством
     const QString default_resp_time_title = "Resposnse_time"; //заглавие время ответа
     const QString default_num_ofRetr_title = "Num_of_retries"; //заглавие число попыток

    //сигналы
     QVector <QString>  ma_typeSig = {"analog","discrete","integer","group"}; //поля типа сигнала 0,1,2,3 при чтении файла
     int m_type_sig; //текущий тип сигнала
     uint m_all_sig_num; // всего сигналов
     QVector <WS_signal*>  ma_a_signals;
     QVector <WS_signal*>  ma_d_signals;
     QVector <WS_signal*>  ma_int_signals;
     QVector <WS_gr_signal*>  ma_gr_signals;
     // для case & others для создания векторов
     enum sig_type_uses{S_ANA,S_DIS, S_INT,S_GROUP};
     //промежуточный вектор для формирования a_value
     QVector <int16_t> ma_a_int_value;

    QModbusTcpClient  *m_modbusClient; //= nullptr;
     quint16 m_num_hold_regs{66};
     quint16 m_start_reg{5199};
     quint16 m_start_qual_reg{5259};
     quint16 m_plc_port{502};
     QString m_plc_host{"192.168.1.233"}; //A3 := "192.168.0.233;
     quint16 m_server_adr{1};
     quint16 m_response_time{500};
     quint8 m_num_of_retries{3};
     //распределение регистров
     const quint16 m_sum_a_reg = 40;
     const quint16 m_sum_d_reg = 10;
     const quint16 m_sum_int_reg =10;
     quint16 m_sum_qual_reg;

    QList<QString> results;

    QModbusDataUnit readRequest(quint16 startAddress, quint16 count) const;
    void Init();


signals:

public slots:
    void connectOn();
    void onStateChanged(int state);
    void errorMessage(QString msg);
    void EmitTime();
    void EmitTimeDiag();

    int readSignalsFile(QString sig_fileName);
    void readIniSection(QString bufstr);
    void FindTypeSignal(QString bufstr);
    void PrepareDiffSigLists(int type_sig, QString bufstr);
    void prepareRead(quint16 startAddress, quint16 count);
    void readReady();
    void readMessage(uint unit_num, int unit_value);
    void readBit(int unit_value);
    void readQualityReg(uint unit_num, int unit_value);
    void printSingnals();
    void testSignals();

    static int rcvFunc( void * argPtr, Value & value, int32_t chnlId );
    int placeIn_PPD(DTSClient client);
    int placeIn_PPD_2_manual(DTSClient client);
};

#endif // PLC_CONNECTOR_H
