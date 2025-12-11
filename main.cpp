
#include <iterator>
#include <iostream>
#include <cstring>
#include <random>
#include <string>
#include <stdio.h>
#include <time.h>
#include <thread>
#include <unistd.h>

#include "include/loggingcategories.h"
#include "include/dtsclient.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QRegularExpression>
#include <QTime>

//dts
#define ERROR_RET -1
#define OK_RET 0
#define WARNING_RET 1

//args commond line
#define WORK "--WORK"
#define DEBUG "--DEBUG"
#define DEBUG_FULL "--DEBUG_FULL"
#define SIMUL "--SIMUL"
#define HELP "--HELP"


static int rcvFunc( void * argPtr, Value & value, int32_t chnlId );
void placeIn_PPD_4_manual(DTSClient client);
//  обработчик сообщений
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
//функция логгирования сообщений
extern void printLogs(QString func_name, QString log_word, Logs_Type type_log);

// Буферное значение для формирования текущих данных
static Value value;

//test vector
static QVector<int> t_test_int;
int testVector();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Породить клиента доступа к ППД
      DTSClient client( rcvFunc,NULL );
    // Проверить порождение клиента
    if( client.getState() != Ok )
    {
        printf( "DTS Client error:%i\n",client.getError() ); return -1;
    }


    //добавим бесконечный цикл
    for(;;sleep(1))
    {
        //test
        for (int i{0}; i<17;i++)
        {
            t_test_int.push_back(testVector());
        }
        for (int i{0}; i<17;i++)
        {
             printLogs("placeIn_PPD number: ", QString::number(t_test_int[i]),log_info);
        }


                printLogs("placeIn_PPD_4", "0!",log_debug);
                // Сформировать аналоговое значение
                makeAValue(&value,0,  0   ,  (float)(0.1+ t_test_int[0]));
                makeAValue(&value,1,  0   ,  (float)(1.2+ t_test_int[1])    );
                makeAValue(&value,2,  0   ,  (float)(2.2+ t_test_int[2])   );
                makeAValue(&value,3,  0   ,  (float)(3.2+ t_test_int[3])    );
                makeAValue(&value,4,  0   ,  (float)(4.2+ t_test_int[4])    );
                makeAValue(&value,5,  0   ,  (float)(5.2+ t_test_int[5])   );
                makeAValue(&value,6,  0   ,  (float)(0.2+ t_test_int[6])   );
                makeAValue(&value,7,  0   ,  (float)(1.3+ t_test_int[7])    );
                makeAValue(&value,8,  0   ,  (float)(2.3+ t_test_int[8])    );
                makeAValue(&value,9,  0   ,  (float)(3.3+ t_test_int[9])    );
                makeAValue(&value,10,  0   , (float)(4.3+ t_test_int[10])    );
                makeAValue(&value,11,  0   , (float)(5.3+ t_test_int[11])    );
                makeAValue(&value,12,  0   , (float)(0.3+ t_test_int[12])    );
                makeAValue(&value,13,  0   , (float)(1.3+ t_test_int[13])    );
                makeAValue(&value,14,  0   , (float)(2.3+ t_test_int[14])    );
                makeAValue(&value,15,  0   , (float)(3.3+ t_test_int[15])    );
                makeAValue(&value,16,  0   , (float)(4.3 + t_test_int[16])   );

                //
                printLogs("placeIn_PPD_4", "1!",log_debug);
                switch( client.put( value ) )
                {
                    case ERROR_RET :
                    {
                         printLogs("placeIn_PPD", "Analog value put error: " +QString::number( client.getError()),log_critical);
                        //std::cout <<"Analog value put error: "<<client.getError()<<std::endl;
                    }
                    return  a.exec();
                    case WARNING_RET :
                    {
                       printLogs("placeIn_PPD", "Analog value put warning: " +QString::number( client.getError()),log_warning);
                    }
                    break;
                    default : break;
                }
                //
                printLogs("placeIn_PPD_4", "2!",log_debug);
            // Сформировать двоичное значение
                     makeBValue(&value,0,     0   ,     t_test_int[0] % 2  );
                     makeBValue(&value,1,     0   ,     t_test_int[1] % 2  );
                     makeBValue(&value,2,     0   ,     t_test_int[2] % 2  );
                     makeBValue(&value,3,     0   ,     t_test_int[3] % 2  );
                     makeBValue(&value,4,     0   ,     t_test_int[4] % 2  );
                  // Поместить в канал ППД двоичные данные с индексом 0
                   switch( client.put( value ) )
                    {
                    case ERROR_RET :
                    {
                         printLogs("placeIn_PPD", "Binary value put error: " + QString::number(client.getError()),log_critical);
                    }
                    return a.exec();
                    case WARNING_RET :
                    {
                        printLogs("placeIn_PPD", "Binary value put warning: " + QString::number(client.getError()),log_warning);
                    }
                    break;
                    default : break;
                    }

            // Сформировать целочисленное значение
                        makeIValue(&value,0,    1   ,    t_test_int[0]  );
                        makeIValue(&value,1,    1   ,    t_test_int[1]+1  );
                        makeIValue(&value,2,    1   ,    t_test_int[2]+2  );
                        makeIValue(&value,3,    1   ,    t_test_int[3]+3  );
                        makeIValue(&value,4,    1   ,    t_test_int[4]+4  );
                        makeIValue(&value,5,    1   ,    t_test_int[5]+5  );
                        makeIValue(&value,6,    1   ,    t_test_int[6]+6  );
                        makeIValue(&value,7,    1   ,    t_test_int[7]+7  );

                        // Поместить в канал ППД целочисленные данные с индексом 0
                        switch( client.put( value ) )
                        {
                        case ERROR_RET :
                        {
                            printLogs("placeIn_PPD", "Integer value put error: " + QString::number(client.getError()),log_critical);
                        }
                        return a.exec();
                        case WARNING_RET :
                        {
                             printLogs("placeIn_PPD", "Integer value put error: " + QString::number(client.getError()),log_warning);
                        }
                        break;
                        default : break;
                        }

                // printLogs("placeIn_PPD", "Ok!",log_info);

    }

    // Задержка для принятия последнего значения
    sleep( 5 );
    printf( "Done!\n" );
    return a.exec();
}


// Пример функции регистрации принятых данных
static int rcvFunc( void * argPtr, Value & value, int32_t chnlId )
{
    // Обработка сигналов
    switch( value.type )
    {
    case Ana_VT :
    {
        AData * dataPtr = NULL; value.getData( dataPtr );
//        printf( "Receive CH[%i] ANA[%i]:%.5f(%i)\n",
//        chnlId,value.idx,dataPtr->value, dataPtr->quality );
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

void placeIn_PPD_4_manual(DTSClient client)
{
        printLogs("placeIn_PPD_4", "0!",log_debug);
        // Сформировать аналоговое значение
        makeAValue(&value,0,  0   ,  (float)0.1    );
        makeAValue(&value,1,  0   ,  (float)1.2    );
        makeAValue(&value,2,  0   ,  (float)2.2    );
        makeAValue(&value,3,  0   ,  (float)3.2    );
        makeAValue(&value,4,  0   ,  (float)4.2    );
        makeAValue(&value,5,  0   ,  (float)5.2   );
        makeAValue(&value,6,  0   ,  (float)0.2   );
        makeAValue(&value,7,  0   ,  (float)1.3    );
        makeAValue(&value,8,  0   ,  (float)2.3    );
        makeAValue(&value,9,  0   ,  (float)3.3    );
        makeAValue(&value,10,  0   , (float)4.3    );
        makeAValue(&value,11,  0   , (float)5.3    );
        makeAValue(&value,12,  0   , (float)0.3    );
        makeAValue(&value,13,  0   , (float)1.3    );
        makeAValue(&value,14,  0   , (float)2.3    );
        makeAValue(&value,15,  0   , (float)3.3    );
        makeAValue(&value,16,  0   , (float)4.3    );

        //
        printLogs("placeIn_PPD_4", "1!",log_debug);
        switch( client.put( value ) )
        {
            case ERROR_RET :
            {
                 printLogs("placeIn_PPD", "Analog value put error: " +QString::number( client.getError()),log_critical);
                //std::cout <<"Analog value put error: "<<client.getError()<<std::endl;
            }
            return;
            case WARNING_RET :
            {
               printLogs("placeIn_PPD", "Analog value put warning: " +QString::number( client.getError()),log_warning);
            }
            break;
            default : break;
        }
        //
        printLogs("placeIn_PPD_4", "2!",log_debug);
    // Сформировать двоичное значение
             makeBValue(&value,0,     0   ,     0  );
             makeBValue(&value,1,     0   ,     1  );
             makeBValue(&value,2,     0   ,     0  );
             makeBValue(&value,3,     0   ,     1  );
             makeBValue(&value,4,     0   ,     1  );
          // Поместить в канал ППД двоичные данные с индексом 0
           switch( client.put( value ) )
            {
            case ERROR_RET :
            {
                 printLogs("placeIn_PPD", "Binary value put error: " + QString::number(client.getError()),log_critical);
            }
            return;
            case WARNING_RET :
            {
                printLogs("placeIn_PPD", "Binary value put warning: " + QString::number(client.getError()),log_warning);
            }
            break;
            default : break;
            }

    // Сформировать целочисленное значение
                makeIValue(&value,0,    1   ,    0  );
                makeIValue(&value,1,    1   ,    1  );
                makeIValue(&value,2,    1   ,    2  );
                makeIValue(&value,3,    1   ,    3  );
                makeIValue(&value,4,    1   ,    4  );
                makeIValue(&value,5,    1   ,    5  );
                makeIValue(&value,6,    1   ,    6  );
                makeIValue(&value,7,    1   ,    7  );

                // Поместить в канал ППД целочисленные данные с индексом 0
                switch( client.put( value ) )
                {
                case ERROR_RET :
                {
                    printLogs("placeIn_PPD", "Integer value put error: " + QString::number(client.getError()),log_critical);
                }
                return ;
                case WARNING_RET :
                {
                     printLogs("placeIn_PPD", "Integer value put error: " + QString::number(client.getError()),log_warning);
                }
                break;
                default : break;
                }

        // printLogs("placeIn_PPD", "Ok!",log_info);

    return;
}


int testVector()
{
    int lower_bound = 1;
    int upper_bound = 100;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(lower_bound, upper_bound);
    return distr(gen);
}
