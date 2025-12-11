#include "include/plc_connector.h"

PLC_connector* PLC_connector::getInstance(QString params, int level)
{
    static PLC_connector instance( params,  level);
    return &instance;
}

PLC_connector::PLC_connector(QString params, int level,QObject *parent) :  m_modbusClient(nullptr)
{
    m_params = params;
    m_level = level;
    Init();
}

PLC_connector::PLC_connector(QObject *parent) :  m_modbusClient(nullptr)

{
    m_params = WORK; //WORK
    m_level = 0;//
    Init();
}

PLC_connector::~PLC_connector()
{
    if(m_modbusClient)
        m_modbusClient->disconnectDevice();
    delete m_modbusClient;
}

void PLC_connector::Init()
{
    sig_fileName = "signals.ini";
    //чтение ini файла с сигналами
    int  result_read_ini = readSignalsFile(sig_fileName);
    if (result_read_ini < 0)
    {
        printLogs("PLC_connector::Init:",QTime::currentTime().toString() + " " +" PLC_Connector not INIT  ", log_critical);
        return;
    }
    m_all_sig_num = ma_a_signals.size() + ma_d_signals.size() + ma_int_signals.size() + ma_gr_signals.size();
    //количество регистров качества (в регистре два бита на сигнал)
    m_sum_qual_reg = m_all_sig_num / 8;
    printLogs("PLC_connector::Init:",QTime::currentTime().toString() + " Количество аналоговых сигналов:" + QString::number(ma_a_signals.size()), log_info);
    printLogs("PLC_connector::Init:",QTime::currentTime().toString() + " Количество дискретных сигналов:" + QString::number(ma_d_signals.size()), log_info);
    printLogs("PLC_connector::Init:",QTime::currentTime().toString() + " Количество целочисленных сигналов:" + QString::number(ma_int_signals.size()), log_info);
    printLogs("PLC_connector::Init:",QTime::currentTime().toString() + " Количество групповых сигналов:" + QString::number(ma_gr_signals.size()), log_info);
    printLogs("PLC_connector::Init:",QTime::currentTime().toString() + " Всего сигналов:" + QString::number(m_all_sig_num), log_info);
    printLogs("PLC_connector::Init:",QTime::currentTime().toString() + " Параметры:"+ m_params+ " level:" + QString::number(m_level), log_info);

    //
    DTSClient client(rcvFunc, NULL);
    if (m_params == SIMUL)
    {
        testSignals();
        printSingnals();
        placeIn_PPD(client);
    }
    else
    {
        connectOn();


    //todo разобраться в таймерах
        m_emit_timer = new QTimer(this);
        connect(m_emit_timer, &QTimer::timeout, this, &PLC_connector::EmitTime);
        m_emit_timer->start(1000/*500*/);

//        m_emit_timer_diag = new QTimer(this);
//        connect(m_emit_timer_diag, &QTimer::timeout, this, &PLC_connector::EmitTimeDiag);
//        m_emit_timer_diag->start(2033);
    }

}


// чтение ini-файла с сигналами
int PLC_connector::readSignalsFile(QString sig_fileName)
{
    //QString folderPath = "signals";
     QDir t_projectDir = QDir::current();

    if (!sig_fileName.isEmpty())
    {
        QFile inifile(sig_fileName);
        if (m_params == DEBUG || m_params == DEBUG_FULL)
            printLogs("readSignalsFile","read_signalsFile: ini-file not empty, dir name:"  + t_projectDir.dirName() ,log_info);

        if (inifile.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&inifile);
            QString str;
            //debug
            if (m_params == DEBUG || m_params == DEBUG_FULL)
                 printLogs("readSignalsFile","read_signalsFile: File name:"  + sig_fileName,log_info);

            while (!stream.atEnd())
            {
                str=stream.readLine();
                readIniSection(str);
            }
            if(stream.status()!=QTextStream::Ok)
            {
               printLogs("readSignalsFile","Coudn't read file",log_critical);
               return -1;
            }
            else
            {
                if (m_params == DEBUG || m_params == DEBUG_FULL)
                    printLogs("readSignalsFile","End read ini-file File name:"+sig_fileName,log_info);
            }

            inifile.close();
        }
    }
    else
    {
            printLogs("readSignalsFile","Not found ini-file",log_critical);
            return  -1;
    }
    return 0;
}

//разбор строк ini-файла
void PLC_connector::readIniSection(QString bufstr)
{
    quint8 t_set_count{8}; // количество полей настроек
    quint8 plc_set{0}; // для провеки наличия настроек

    if (m_params == DEBUG_FULL)
        printLogs("PLC_connector::readIniSection","ini line:" + bufstr,log_debug);

    bufstr =bufstr.remove(" ");
    //бесполезно
//    if (bufstr.contains(plc_settings))  { ++plc_set;  }

    //парсим настройки связей с плк
        if (bufstr.contains(default_ip_title,Qt::CaseSensitive))
        {
            bufstr.replace(',','.');        //для правильного отображения ip
            m_plc_host = bufstr.remove(default_ip_title + "=");
        }
        if (bufstr.contains(default_port_title))
        {
            m_plc_port = static_cast<quint16>((bufstr.remove(default_port_title + "=")).toUInt());
        }
        if (bufstr.contains(default_server_adr_title))
        {
            m_server_adr = static_cast<quint8>((bufstr.remove(default_server_adr_title + "=")).toUInt());
        }
        if (bufstr.contains(default_1st_regstr_title))
        {
            m_start_reg =  static_cast<quint16>((bufstr.remove(default_1st_regstr_title + "=")).toUInt());
        }
        if (bufstr.contains(default_num_hld_regstr_title))
        {
            m_num_hold_regs = static_cast<quint16>((bufstr.remove(default_num_hld_regstr_title + "=")).toUInt());
        }
        if (bufstr.contains(default_1st_qual_regstr_title))
        {
            m_start_qual_reg = static_cast<quint16>((bufstr.remove(default_1st_qual_regstr_title + "=")).toUInt());
        }
        if (bufstr.contains(default_resp_time_title))
        {
            m_response_time = static_cast<quint16>((bufstr.remove(default_resp_time_title + "=")).toUInt());
        }
        if (bufstr.contains(default_num_ofRetr_title))
        {
            m_num_of_retries = static_cast<quint8>((bufstr.remove(default_num_ofRetr_title + "=")).toUInt());
            plc_set = t_set_count;
        }

    if ((plc_set == t_set_count) && ((m_params == DEBUG) || (m_params == DEBUG_FULL)))
    {
        printLogs("PLC_connector::readIniSection","host ip:" + m_plc_host,log_debug);
        printLogs("PLC_connector::readIniSection","port:" + QString::number(m_plc_port),log_debug);
        printLogs("PLC_connector::readIniSection","serv adrress:" + QString::number(m_server_adr),log_debug);
        printLogs("PLC_connector::readIniSection","1st regstr:" + QString::number(m_start_reg),log_debug);
        printLogs("PLC_connector::readIniSection","num regstr:" + QString::number(m_num_hold_regs),log_debug);
        printLogs("PLC_connector::readIniSection","1st quality regstr:" + QString::number(m_start_qual_reg),log_debug);
        printLogs("PLC_connector::readIniSection","resp time:" + QString::number(m_response_time),log_debug);
        printLogs("PLC_connector::readIniSection","num of retr:" + QString::number(m_num_of_retries),log_debug);
    }

   //парсим сигналы
    FindTypeSignal(bufstr);

    if (bufstr.contains(default_signal_title))
    {
        if ( m_type_sig >= 0)
        {
             PrepareDiffSigLists( m_type_sig, bufstr);
        }
    }

}

// выяснение типа сигнала
void PLC_connector::FindTypeSignal(QString bufstr)
{
    for(auto& ma: ma_typeSig)
    {
        if (bufstr.contains(ma, Qt::CaseSensitive))
        {
            //test                    printLogs("PLC_connector::FindTypeSignal","bufstr:" + bufstr,log_debug);
            m_type_sig = ma_typeSig.indexOf(ma);
        }
    }
}

//создание векторов из ini-файла по типу обрабатываемых сигналов
void PLC_connector::PrepareDiffSigLists(int type_sig, QString bufstr)
{
    QString t_str;
    qint8 t_type{-1};

    QRegularExpression reg0("Signal[0-9]{1,3}\\=");//отсечение "сигнала"
        //расчленяем строку
        bufstr.remove(' ');
        t_str = bufstr.remove(reg0);
        QStringList t=t_str.split(QChar(','));

        switch (type_sig) // t_type
        {
        case S_ANA:
            {
                WS_signal *A_sig = new WS_signal(static_cast<int8_t>(t_type), t.at(0),static_cast<uint16_t>(t.at(1).toUInt()),
                                                 static_cast<uint16_t>(t.at(2).toUInt()),t.at(3).toFloat());// (uint8_t sigT, QString sigN, uint16_t sigHW, uint16_t sigQW, float sigSA);
                if (m_params == DEBUG_FULL)
                    printLogs("PLC_connector::PrepareDiffSigLists","ANA signal name:" + t.at(0) + " HoldReg:" + QString::number(t.at(1).toUInt()) +
                              " QualReg:" + QString::number(t.at(2).toUInt()) + " Simul:" + t.at(3),  log_debug);
                ma_a_signals.push_back(A_sig);
                break;
            }
        case S_DIS:
            {
                 WS_signal *D_sig = new WS_signal(static_cast<int8_t>(t_type), t.at(0),static_cast<uint16_t>(t.at(1).toUInt()),
                                             static_cast<uint16_t>(t.at(2).toUInt()),static_cast<int32_t>(t.at(3).toLong()));// (uint8_t sigT, QString sigN, uint16_t sigHW, uint16_t sigQW, int32_t sigSDI);
                 if (m_params == DEBUG_FULL)
                     printLogs("PLC_connector::PrepareDiffSigLists","Dis signal name:" + t.at(0) + " HoldReg:" + QString::number(t.at(1).toUInt()) +
                          " QualReg:" + QString::number(t.at(2).toUInt()) + " Simul:" + t.at(3),  log_debug);

                 ma_d_signals.push_back(D_sig);
                break;
            }
        case S_INT:
            {
                WS_signal *INT_sig = new WS_signal(static_cast<int8_t>(t_type), t.at(0),static_cast<uint16_t>(t.at(1).toUInt()),
                                        static_cast<uint16_t>(t.at(2).toUInt()),static_cast<int32_t>(t.at(3).toLong()));// (uint8_t sigT, QString sigN, uint16_t sigHW, uint16_t sigQW, int32_t sigSDI);
               if (m_params == DEBUG_FULL)
                   printLogs("PLC_connector::PrepareDiffSigLists","INT signal name:" + t.at(0) + " HoldReg:" + QString::number(t.at(1).toUInt()) +
                     " QualReg:" + QString::number(t.at(2).toUInt()) + " Simul:" + t.at(3),  log_debug);

                   ma_int_signals.push_back(INT_sig);                   
                break;
            }
        case S_GROUP:
            {
            //todo
                WS_gr_signal *GR_sig = new WS_gr_signal();

                ma_gr_signals.push_back(GR_sig);
                break;
            }
        }
}

void PLC_connector::errorMessage(QString msg)
{
    //debug
    if (m_params == DEBUG || m_params == DEBUG_FULL)
    {
        printLogs("PLC_connector::errorMessage:",QTime::currentTime().toString() + " " +msg, log_debug);
    }
}

void PLC_connector::EmitTime()
{
            //test    std::cout <<"PLC_connector::EmitTime:" <<std::endl;
    //
   prepareRead(m_start_reg, m_num_hold_regs);
   //testSignals();
   int placeIn = placeIn_PPD(client);
    //int placeIn = placeIn_PPD_2_manual(client);
    if (m_params == DEBUG_FULL && m_level == 0)
    {
        printSingnals();
    }
}

void PLC_connector::EmitTimeDiag()
{
            //test    std::cout <<"PLC_connector::EmitTime:" <<std::endl;
    //
   // prepareRead(m_start_reg, m_num_hold_regs);
        // int placeIn = placeIn_PPD_2_manual(client);
    //int placeIn = placeIn_PPD(client);
    if (m_params == DEBUG)
    {
        printSingnals();
    }
}

//if modbus connection state changed
void PLC_connector::onStateChanged(int state)
{
    if(state == QModbusDevice::UnconnectedState)
    {
        //debug
        if (m_params == DEBUG || m_params == DEBUG_FULL)
        {
            printLogs("PLC_connector::onStateChanged:",QTime::currentTime().toString() + " " + "Port down", log_info);
        }
    }
    else if (state == QModbusDevice::ConnectedState)
    {
        //debug
        if (m_params == DEBUG || m_params == DEBUG_FULL)
        {
            printLogs("PLC_connector::onStateChanged:",QTime::currentTime().toString() + " " + "Port up ", log_info);
        }

    }
    else
    {
        return; // exit from function
    }
}

void PLC_connector::connectOn()
{
    if (m_modbusClient)
     {
         m_modbusClient->disconnectDevice();
         delete m_modbusClient;
         m_modbusClient = nullptr;
     }

    // Create a Modbus client instance
    m_modbusClient = new QModbusTcpClient(this);

    m_modbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter, m_plc_port);
    m_modbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter,m_plc_host);
    m_modbusClient->setTimeout(m_response_time);
    m_modbusClient->setNumberOfRetries(m_num_of_retries);

    //test
//    printLogs("PLC_connector::connectOn:", " m_plc_port:" + QString::number(m_plc_port) + " m_plc_host:" + m_plc_host+
//              " m_response_time:" + QString::number(m_modbusClient->timeout()) +
//              " num_retries:" + QString::number(m_modbusClient->numberOfRetries()),log_info);

    connect(m_modbusClient, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
              printLogs("PLC_connector::connectOn:",QTime::currentTime().toString() + " Error:" + m_modbusClient->errorString(), log_critical);
        });

    if(m_modbusClient) {
        connect(m_modbusClient, &QModbusClient::stateChanged, this, &PLC_connector::onStateChanged);
        }

    if (!m_modbusClient)
        return;
    //debug
    if (m_params == DEBUG_FULL || m_params == DEBUG)
         printLogs("PLC_connector::connectOn:",QTime::currentTime().toString() + " " + " modbusClient run in Connect_on  ", log_info);

     // Try connecting to the server
     if(m_modbusClient->state() != QModbusDevice::ConnectedState)
     {
         if (!m_modbusClient->connectDevice())
         {
             printLogs("PLC_connector::connectOn:",QTime::currentTime().toString() + " " + "Failed to connect to the server:" + m_modbusClient->errorString(), log_critical);
         }
         else
         {
             printLogs("PLC_connector::connectOn:",QTime::currentTime().toString() + " " + "State: " + QString::number(m_modbusClient->state()), log_info);
         }
     }
     else
     {
         //debug
         if (m_params == DEBUG || m_params == DEBUG_FULL)
             printLogs("PLC_connector::connectOn:",QTime::currentTime().toString() + " " + "Successfully connected to the server.", log_info);
         //m_modbusClient->disconnectDevice(); надо ли??
     }
     //
     prepareRead(m_start_reg, m_num_hold_regs);

}

//
QModbusDataUnit PLC_connector::readRequest(quint16 startAddress, quint16 count) const
{
    //
    //debug
    if (m_params == DEBUG_FULL)
    {
                 printLogs("PLC_connector::readRequest:",QTime::currentTime().toString() + " startA:" + QString::number(startAddress) + " count:" + QString::number(count), log_info);
    }
     return QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, count);
}

//
void PLC_connector::prepareRead(quint16 startAddress, quint16 count)
{
    if(!m_modbusClient) return;
// version 1
    if(auto *lastRequest = m_modbusClient->sendReadRequest(readRequest(startAddress, count), m_server_adr))
    {
        //debug
        if (m_params == DEBUG_FULL)
            qInfo(logInfo()) << " PLC_connector Prepare_Read lastRequest ";

        if(!lastRequest->isFinished())
        {
            //
            if (m_params == DEBUG_FULL)
            {
                printLogs("PLC_connector::prepareRead:"," PLC_connector Prepare_Read isFinished ", log_debug);
            }
             connect(lastRequest, &QModbusReply::finished, this, &PLC_connector::readReady);
        }
        else
        {
            delete lastRequest;
        }
    }
    else
    {
        printLogs("PLC_connector::prepareRead:",QTime::currentTime().toString() + " " + "Read error: " + m_modbusClient->errorString(), log_critical);
    }

}


void PLC_connector::readReady()
{
    //    if (m_params == DEBUG_FULL)    {        printLogs("PLC_connector::readReady", " Start Read_Ready ",log_debug);    }
    auto reply = qobject_cast<QModbusReply *>(sender());
     if (!reply)
         return;
     if (reply->error() == QModbusDevice::NoError)
     {
         const QModbusDataUnit unit = reply->result();
         QStringList resultsList;
         // Формируем список результатов в виде строк entry И складываем их в контейнер resultsList
         for (uint i = 0; i < unit.valueCount(); i++)
         {
             QString entry = tr("Address: %1, Value: %2").arg(unit.startAddress()+static_cast<int>(i)).arg(QString::number(unit.value(i)));
            resultsList += entry;
             //debug
            if (m_params == DEBUG_FULL && m_level == 1)
                printLogs("PLC_connector::readReady", " entry: " + entry ,log_debug);
            readMessage(i, unit.value(i));
         }
     }
     else if (reply->error() == QModbusDevice::ProtocolError)
     {
         // Выводим сообщение об ошибке протокола используя объект reply->errorString();
     }
     else
     {
         // Выводим сообщение об остальных типах ошибки используя объект reply->errorString();
         // В принципе, можно убрать столь подробное деление обработчика ошибок, оставив только лишь этот блок.
     }

     reply->deleteLater();
}

void PLC_connector::readMessage(uint unit_num, int unit_value)
{
       // номер элемента вектора аналоговых значений
       uint t_a_num = unit_num / 2;
       //для объединения 2х регистров int16 в int32
       int32_t t_a_int32_val;
        if (unit_num < m_num_hold_regs)
        {
             if(unit_num <= 2 * ma_a_signals.size())
             {
                   ma_a_int_value.append(unit_value);

                  // ma_a_signals[unit_num]->sigAValue = unit_value;
                 if (( unit_num % 2 != 0) && (unit_num != 0))
                 {
                       t_a_int32_val =(ma_a_int_value[unit_num]<<16)|(ma_a_int_value[unit_num-1]&0xffff);
                       memcpy(&ma_a_signals[t_a_num]->sigAValue, &t_a_int32_val, 4);
                       //test                       std::cout <<"PLC_connector::readMessage int hi:" << ma_a_int_value[unit_num-1]<<" lo:"<< ma_a_int_value[unit_num]<<std::hex<< std::endl;
                       //test                       std::cout <<"PLC_connector::readMessage a:" << ma_a_signals[t_a_num]->sigAValue <<" num:"<<unit_num<< " size:"<<2* ma_a_signals.size()<<std::endl;
                 }
             }

           //целочисленные значения заполнение
            if ((unit_num>=(m_sum_a_reg)) && (unit_num<(m_sum_a_reg + m_sum_int_reg)))
            {
               if((static_cast<int>(unit_num) - m_sum_a_reg) < ma_int_signals.size())
               {
                   ma_int_signals[static_cast<int>(unit_num) - m_sum_a_reg]->sigINTValue = unit_value;
                   //test                   std::cout <<"PLC_connector::readMessage int:" << ma_int_signals[static_cast<int>(unit_num) - m_sum_a_reg]->sigINTValue <<" t_int_num:"                            <<( static_cast<int>(unit_num) - m_sum_a_reg)<< " size:"<< ma_int_signals.size()<<std::endl;
               }
            }
            //считываение бинарных значений -
            if ((unit_num>=(m_sum_a_reg + m_sum_int_reg))&&(unit_num<(m_sum_a_reg + m_sum_d_reg + m_sum_int_reg)))
            {
                //test                std::cout <<"PLC_connector::readMessage  unit num:"<<unit_num<< " d size:"<< ma_d_signals.size()<<std::endl;
                // чтобы функция вызывалась по количеству используемых слов, а не в диапазоне от 0 до m_sum_d_reg
                if (unit_num <= static_cast<uint>((m_sum_a_reg + m_sum_int_reg + ma_d_signals.size()/16)))
                {
                   readBit(unit_value);
                }

            }
            //todo считываение качества и симуляции
            if ((unit_num >= (m_start_qual_reg - m_start_reg)) && (unit_num < ( (m_start_qual_reg - m_start_reg) + (m_sum_a_reg/8) + (m_sum_d_reg/8) + (m_sum_int_reg/8) )))
            {
                                   //test                std::cout <<"PLC_connector::readMessage unit_num:" <<unit_num<<" unit_value:" << unit_value <<std::endl;
                readQualityReg(unit_num, unit_value);
            }
          }
        //test        std::cout <<"PLC_connector::readMessage m_start_qual_reg:" <<(m_start_qual_reg)<<" m_num_hold_regs:" << (m_num_hold_regs )<<std::endl;
}

void PLC_connector::readBit( int unit_value)
{
    int t_bit_num;
    //номер элемента дискретнобинарного вектора
    int t_disword_num = ma_d_signals.size()/16;

    //test    std::cout <<"PLC_connector::readBit  unit num:"<<unit_num<< " t_disword_num:"<< t_disword_num<<std::endl;
    //проход по словам
    for (int i = 0; i < t_disword_num + 1; i++)
    {
       // проход по 16 битам в слове
     for (int ii=0; ii<16; ii++)
      {
         //номер бита - положение в слове + количество слов на емкость регистра в 16 бит
        t_bit_num = ii +t_disword_num * 16;
        //ограничиваем количество бит размером дискретного вектора из файла
        if(t_bit_num< ma_d_signals.size())
        {
            //stackowerflow здесь мы создаём маску ( 1 << ii), применяем её к unit_value, а затем сдвигаем замаскированное значение вправо, чтобы получить нужный нам бит.
            ma_d_signals[t_bit_num]->sigDValue = (unit_value & ( 1 << ii )) >> ii ;
            //test            std::cout <<"PLC_connector::readBit bit value:" << ma_d_signals[t_bit_num]->sigDValue <<" t_b_num:" << t_bit_num << std::endl;
        }
      }
    }
}
//todo
void PLC_connector::readQualityReg(uint unit_num, int unit_value)
{
    //номер сигнала
   int t_a_num;
   int t_d_num;
   int t_int_num;

   //проход по словам
       for (int i = 0; i < 8; i++)
       {
           // analog signals
            //m_sum_a_reg/(2*8) = 2, а правильно 3 регистра, поэтому std::ceil (float)
           if ((unit_num >= (m_start_qual_reg - m_start_reg)) && (unit_num < (m_start_qual_reg - m_start_reg + std::ceil(static_cast<float>(ma_a_signals.size())/8))))
           {
               t_a_num = static_cast<int>(unit_num) - (m_start_qual_reg - m_start_reg);
               // 0 bit - отказ устройства, 8 bit - имитация
               if ((8 * t_a_num +i) < ma_a_signals.size())
               {
                   ma_a_signals[8*t_a_num +i]->sigQualValue = 1 * ((unit_value & ( 1 << i )) >> i) + 256 * ((unit_value & ( 1 << (i+8) )) >> (i + 8));
                   if (m_params == DEBUG_FULL && m_level == 1)
                   {
                       std::cout <<"PLC_connector::readQualityReg qual a value:" << ma_a_signals[8*t_a_num +i]->sigQualValue <<" t_a_num:" << 8*t_a_num +i << std::endl;
                   }
               }
           }

           //test unit_num  = reg_num + 1          std::cout <<"PLC_connector::readQualityReg  unit_num:" << unit_num<<" reg num:" <<unit_num + m_start_reg << std::endl;

           // integer signals
                 //m_sum_a_reg/(2*8) = 2, а правильно 3 регистра, поэтому std::ceil

           if ((unit_num >= (m_start_qual_reg - m_start_reg + std::ceil(static_cast<float>(m_sum_a_reg)/(2*8))) &&
                            (unit_num < (m_start_qual_reg - m_start_reg + std::ceil(static_cast<float>(m_sum_a_reg)/(2*8))
                                          + std::ceil(static_cast<float>(ma_int_signals.size())/8)) )))
           {
               t_int_num = static_cast<int>(unit_num) - (m_start_qual_reg - m_start_reg + std::ceil(static_cast<float>(m_sum_a_reg)/(2*8)) );
                // 0 bit - отказ устройства, 8 bit - имитация
               if ((8 * t_int_num +i) < ma_int_signals.size())
               {
                   ma_int_signals[8*t_int_num +i]->sigQualValue = 1 * ((unit_value & ( 1 << i )) >> i) + 256 * ((unit_value & ( 1 << (i+8) )) >> (i + 8));
                   if (m_params == DEBUG_FULL && m_level == 1)
                   {
                       std::cout <<"PLC_connector::readQualityReg qual int value:" << ma_int_signals[8*t_int_num +i]->sigQualValue <<" t_int_num:" << 8*t_int_num +i << std::endl;
                   }
               }
           }

           // discreate signals
                 //m_sum_a_reg/(2*8) = 2, а правильно 3 регистра, поэтому std::ceil
           if ((unit_num >= (m_start_qual_reg - m_start_reg + std::ceil(static_cast<float>(m_sum_a_reg)/(2*8)) +
                                  std::ceil(static_cast<float>(m_sum_int_reg)/8) )) &&
                            (unit_num < (m_start_qual_reg - m_start_reg + std::ceil(static_cast<float>(m_sum_a_reg)/(2*8)) +
                                  std::ceil(static_cast<float>(m_sum_int_reg)/8) + std::ceil(static_cast<float>(ma_d_signals.size())/8) )))
           {
               t_d_num = static_cast<int>(unit_num) - (m_start_qual_reg - m_start_reg + std::ceil(static_cast<float>(m_sum_int_reg)/8) +
                                     std::ceil(static_cast<float>(m_sum_a_reg)/(2*8)) );
               //test               std::cout <<"PLC_connector::readQualityReg  unit_num:" << unit_num<<" t_d_num:" << t_d_num<<" 1q>:" <<                           (m_start_qual_reg - m_start_reg + (m_sum_a_reg/(2*8)) )<<" 1h<=:" <<(m_start_qual_reg - m_start_reg +  (m_sum_a_reg/(2*8)) + (ma_int_signals.size()/8))<< std::endl;
                // 0 bit - отказ устройства, 8 bit - имитация
               if ((8 * t_d_num +i) < ma_d_signals.size())
               {
                   ma_d_signals[8*t_d_num +i]->sigQualValue = 1 * ((unit_value & ( 1 << i )) >> i) + 256 * ((unit_value & ( 1 << (i+8) )) >> (i + 8));
                    if (m_params == DEBUG_FULL && m_level == 1)
                    {
                        std::cout <<"PLC_connector::readQualityReg qual d value:" << ma_d_signals[8*t_d_num +i]->sigQualValue <<" t_d_num:" << 8*t_d_num +i << std::endl;
                    }
               }
           }
       }
}

void PLC_connector::printSingnals()
{
    for(int i =0; i<ma_a_signals.size();i++)
        printLogs("PLC_connector::printSingnals:", QTime::currentTime().toString() + " ma: " +QString::number(ma_a_signals[i]->sigAValue) +
                  " Q:" + QString::number(ma_a_signals[i]->sigQualValue), log_debug);
    for(int i =0; i<ma_d_signals.size();i++)
        printLogs("PLC_connector::printSingnals:", QTime::currentTime().toString() +  " md: " +QString::number(ma_d_signals[i]->sigDValue) +
                  " Q:" + QString::number(ma_d_signals[i]->sigQualValue), log_debug);
    for(int i =0; i<ma_int_signals.size();i++)
        printLogs("PLC_connector::printSingnals:",QTime::currentTime().toString() +  " mint: " +QString::number(ma_int_signals[i]->sigINTValue) +
                  " Q:" + QString::number(ma_int_signals[i]->sigQualValue), log_debug);
}

void PLC_connector::testSignals()
{
    for(int i =0; i<ma_a_signals.size();i++)
    {
        ma_a_signals[i]->sigAValue = 133.3;
        ma_a_signals[i]->sigQualValue = 3;
    }

    for(int i =0; i<ma_d_signals.size();i++)
    {
        ma_d_signals[i]->sigDValue = (int)(i%2 == 0);
        ma_d_signals[i]->sigQualValue = 3;
    }
     for(int i =0; i<ma_int_signals.size();i++)
     {
         ma_int_signals[i]->sigINTValue = 123;
         ma_int_signals[i]->sigQualValue = 3;
     }
}

// Пример функции регистрации принятых данных
 int PLC_connector::rcvFunc( void * argPtr, Value & value, int32_t chnlId )
{
    // Обработка сигналов
    switch( value.type )
    {
    case Ana_VT :
    {
        AData * dataPtr = NULL; value.getData( dataPtr );
    }
    break;
    case Bin_VT :
    {
        BData * dataPtr = NULL; value.getData( dataPtr );
    }
    break;
    case Int_VT :
    {
        IData * dataPtr = NULL; value.getData( dataPtr );
    }
    break;
    case Grp_VT :
    {
        GData * dataPtr = NULL; value.getData( dataPtr );

    }
    break;
    default : break;
    }
    return 0;
}

 // test version 2
 // размещение данных в канале ППД
 int PLC_connector::placeIn_PPD(DTSClient client)
 {
     if( ma_a_signals.size()>0)
      for( int i = 0; i <  ma_a_signals.size(); i++/*,sleep( 1 )*/)
       {
         if (m_params == DEBUG_FULL && m_level == 1)
             printLogs("PLC_connector::placeIn_PPD","Send...AValue:" + QString::number( ma_a_signals[i]->sigAValue)+
                       " Q:"+QString::number( ma_a_signals[i]->sigQualValue),log_debug);
         // Сформировать аналоговое значение
         //test                    makeBValue(&value,0,1,3.14);
         makeAValue(&value, i, ma_a_signals[i]->sigQualValue, ma_a_signals[i]->sigAValue);
         //test         std::cout <<"PLC_connector::placeIn_PPD ana value:" <<  ma_a_signals[i]->sigAValue <<" i:" << i << std::endl;
         // Поместить в канал ППД аналоговые данные с индексом 0
         switch( client.put( value ) )
         {
             case ERROR_RET :
             {
                  printLogs("PLC_connector::placeIn_PPD", "Analog value put error: " +QString::number( client.getError()),log_critical);
                 //std::cout <<"Analog value put error: "<<client.getError()<<std::endl;
             }
             return -1;
             case WARNING_RET :
             {
                printLogs("PLC_connector::placeIn_PPD", "Analog value put warning: " +QString::number( client.getError()),log_warning);
             }
             break;
             default : break;
         }
       }
     // Сформировать двоичное значение
     if( ma_d_signals.size()>0)
       for( int i = 0; i <  ma_d_signals.size(); i++/*,sleep( 1 )*/)
         {
             //test            makeBValue(&value,i,2,(i % 2) == 0);
              makeBValue(&value,i, ma_d_signals[i]->sigQualValue, ma_d_signals[i]->sigDValue);
                         //test             std::cout <<"PLC_connector:: placeIn_PPD bit value:" <<  ma_d_signals[i]->sigDValue <<" i:" << i << std::endl;
           // Поместить в канал ППД двоичные данные с индексом 0
            switch( client.put( value ) )
             {
             case ERROR_RET :
             {
                  printLogs("PLC_connector::placeIn_PPD", "Binary value put error: " + QString::number(client.getError()),log_critical);
             }
             return -1;
             case WARNING_RET :
             {
                 printLogs("PLC_connector::placeIn_PPD", "Binary value put warning: " + QString::number(client.getError()),log_warning);
             }
             break;
             default : break;
             }
         }
     // Сформировать целочисленное значение
     if( ma_int_signals.size() > 0)
          for( int i = 0; i <  ma_int_signals.size(); i++/*,sleep( 1 )*/)
             {
                 //test                makeIValue(&value,i,1,i*12);
                 makeIValue(&value,i, ma_int_signals[i]->sigQualValue, ma_int_signals[i]->sigINTValue);
                 //test                    std::cout <<"PLC_connector:: placeIn_PPD bit value int value:" <<  ma_int_signals[i]->sigINTValue <<" i:" << i << std::endl;
                 // Поместить в канал ППД целочисленные данные с индексом 0
                 switch( client.put( value ) )
                 {
                 case ERROR_RET :
                 {
                     printLogs("PLC_connector::placeIn_PPD", "Integer value put error: " + QString::number(client.getError()),log_critical);
                 }
                 return -1;
                 case WARNING_RET :
                 {
                      printLogs("PLC_connector::placeIn_PPD", "Integer value put error: " + QString::number(client.getError()),log_warning);
                 }
                 break;
                 default : break;
                 }
             }
     if( ma_gr_signals.size() > 0)
           for( int i = 0; i <  ma_gr_signals.size(); i++/*,sleep( 1 )*/)
             {
                 char groupData[128]; memset( groupData,i,sizeof( groupData ) );
                 // Сформировать групповое значение для абонента 0
                 makeGValue( &value,0,13,groupData,sizeof( groupData ) );
                 // Поместить группу для потребителя 0 в канал ППД
                 if( client.put(value) != 0 )
                 {
                     printLogs("placeIn_PPD", "Group value put error: " + QString::number(client.getError()),log_critical);
                     return -1;
                 }
             }
          printLogs("PLC_connector::placeIn_PPD", "Ok!",log_info);


     // Задержка для принятия последнего значения
     sleep( 2 );//5
     if (m_params == DEBUG_FULL || m_params == DEBUG)
                  printLogs("PLC_connector::placeIn_PPD", "Done!",log_debug);

     return 0;
 }

 // test version 2
int PLC_connector::placeIn_PPD_2_manual(DTSClient client)
{
        // Сформировать аналоговое значение
    //    std::cout <<"PLC_connector::placeIn_PPD_2_manual: "<<"0"<<std::endl;
        makeAValue(&value,0, ma_a_signals[0]->sigQualValue, ma_a_signals[0]->sigAValue);
        makeAValue(&value,1, ma_a_signals[1]->sigQualValue, ma_a_signals[1]->sigAValue);
        makeAValue(&value,2, ma_a_signals[2]->sigQualValue, ma_a_signals[2]->sigAValue);
        makeAValue(&value,3, ma_a_signals[3]->sigQualValue, ma_a_signals[3]->sigAValue);
        makeAValue(&value,4, ma_a_signals[4]->sigQualValue, ma_a_signals[4]->sigAValue);
        makeAValue(&value,5, ma_a_signals[5]->sigQualValue, ma_a_signals[5]->sigAValue);
        makeAValue(&value,6, ma_a_signals[6]->sigQualValue, ma_a_signals[6]->sigAValue);
        makeAValue(&value,7, ma_a_signals[7]->sigQualValue, ma_a_signals[7]->sigAValue);
        makeAValue(&value,8, ma_a_signals[8]->sigQualValue, ma_a_signals[8]->sigAValue);
        makeAValue(&value,9, ma_a_signals[9]->sigQualValue, ma_a_signals[9]->sigAValue);
        makeAValue(&value,10, ma_a_signals[10]->sigQualValue, ma_a_signals[10]->sigAValue);
        makeAValue(&value,11, ma_a_signals[11]->sigQualValue, ma_a_signals[11]->sigAValue);
        makeAValue(&value,12, ma_a_signals[12]->sigQualValue, ma_a_signals[12]->sigAValue);
        makeAValue(&value,13, ma_a_signals[13]->sigQualValue, ma_a_signals[13]->sigAValue);
        makeAValue(&value,14, ma_a_signals[14]->sigQualValue, ma_a_signals[14]->sigAValue);
        makeAValue(&value,15, ma_a_signals[15]->sigQualValue, ma_a_signals[15]->sigAValue);
        makeAValue(&value,16, ma_a_signals[16]->sigQualValue, ma_a_signals[16]->sigAValue);

          //      std::cout <<"PLC_connector::placeIn_PPD_2_manual: "<<&value <<std::endl;

        switch( client.put( value ) )
        {
            case ERROR_RET :
            {
                 printLogs("PLC_connector::placeIn_PPD", "Analog value put error: " +QString::number( client.getError()),log_critical);
                //std::cout <<"Analog value put error: "<<client.getError()<<std::endl;
            }
            return -1;
            case WARNING_RET :
            {
               printLogs("PLC_connector::placeIn_PPD", "Analog value put warning: " +QString::number( client.getError()),log_warning);
            }
            break;
            default : break;
        }
        //    std::cout <<"PLC_connector::placeIn_PPD_2_manual "<<"1"<<std::endl;
    // Сформировать двоичное значение
             makeBValue(&value,0, ma_d_signals[0]->sigQualValue, ma_d_signals[0]->sigDValue);
             makeBValue(&value,1, ma_d_signals[1]->sigQualValue, ma_d_signals[1]->sigDValue);
             makeBValue(&value,2, ma_d_signals[2]->sigQualValue, ma_d_signals[2]->sigDValue);
             makeBValue(&value,3, ma_d_signals[3]->sigQualValue, ma_d_signals[3]->sigDValue);
             makeBValue(&value,4, ma_d_signals[4]->sigQualValue, ma_d_signals[4]->sigDValue);
          // Поместить в канал ППД двоичные данные с индексом 0
           switch( client.put( value ) )
            {
            case ERROR_RET :
            {
                 printLogs("PLC_connector::placeIn_PPD", "Binary value put error: " + QString::number(client.getError()),log_critical);
            }
            return -1;
            case WARNING_RET :
            {
                printLogs("PLC_connector::placeIn_PPD", "Binary value put warning: " + QString::number(client.getError()),log_warning);
            }
            break;
            default : break;
            }

    // Сформировать целочисленное значение
                makeIValue(&value,0, ma_int_signals[0]->sigQualValue, ma_int_signals[0]->sigINTValue);
                makeIValue(&value,1, ma_int_signals[1]->sigQualValue, ma_int_signals[1]->sigINTValue);
                makeIValue(&value,2, ma_int_signals[2]->sigQualValue, ma_int_signals[2]->sigINTValue);
                makeIValue(&value,3, ma_int_signals[3]->sigQualValue, ma_int_signals[3]->sigINTValue);
                makeIValue(&value,4, ma_int_signals[4]->sigQualValue, ma_int_signals[4]->sigINTValue);
                makeIValue(&value,5, ma_int_signals[5]->sigQualValue, ma_int_signals[5]->sigINTValue);
                makeIValue(&value,6, ma_int_signals[6]->sigQualValue, ma_int_signals[6]->sigINTValue);
                makeIValue(&value,7, ma_int_signals[7]->sigQualValue, ma_int_signals[7]->sigINTValue);

                // Поместить в канал ППД целочисленные данные с индексом 0
                switch( client.put( value ) )
                {
                case ERROR_RET :
                {
                    printLogs("PLC_connector::placeIn_PPD", "Integer value put error: " + QString::number(client.getError()),log_critical);
                }
                return -1;
                case WARNING_RET :
                {
                     printLogs("PLC_connector::placeIn_PPD", "Integer value put error: " + QString::number(client.getError()),log_warning);
                }
                break;
                default : break;
                }

         printLogs("PLC_connector::placeIn_PPD", "Ok!",log_info);

    // Задержка для принятия последнего значения
    //sleep( 2 );//5

    return 0;
}
