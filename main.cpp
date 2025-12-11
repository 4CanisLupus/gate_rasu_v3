
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "dtsclient.h"

#include <QCoreApplication>

#define ERROR_RET -1
#define OK_RET 0
#define WARNING_RET 1

static int rcvFunc( void * argPtr, Value & value, int32_t chnlId );

// Буферное значение для формирования текущих данных
static Value value;

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
    while(true)
    // Рабочий цикл
    for( int i = 0; i < 5; i++,sleep( 1 ) )
    {
        printf( "[%i] Send...",i );

        // Сформировать аналоговое значение
        for( int ii = 0; ii < 5; ii++ )
        makeAValue(&value,ii,i % 3,3.1415926 + i);
        // Поместить в канал ППД аналоговые данные с индексом 0
        switch( client.put( value ) )
        {
            case ERROR_RET :
            {
                printf( "Analog value put error:%i\n", client.getError() );
            }
            return -1;
            case WARNING_RET :
            {
                printf( "Analog value put warning:%i\n", client.getError() );
            }
            break;
            default : break;
        }
    // Сформировать двоичное значение
    makeBValue(&value,0,i % 3,i % 2);
    // Поместить в канал ППД двоичные данные с индексом 0
    switch( client.put( value ) )
        {
            case ERROR_RET :
            {
                printf( "Binary value put error:%i\n", client.getError() );
            }
            return -1;
            case WARNING_RET :
            {
                printf( "Binary value put warning:%i\n", client.getError() );
            }
            break;
            default : break;
        }

    // Сформировать целочисленное значение
    makeIValue(&value,0,i % 3,i);
    // Поместить в канал ППД целочисленные данные с индексом 0
    switch( client.put( value ) )
        {
            case ERROR_RET :
            {
                printf( "Integer value put error:%i\n", client.getError() );
            }
            return -1;
            case WARNING_RET :
            {
                printf( "Integer value put warning:%i\n", client.getError() );
            }
            break;
            default : break;
        }

    char groupData[128]; memset( groupData,i,sizeof( groupData ) );
    // Сформировать групповое значение для абонента 0
    makeGValue( &value,0,13,groupData,sizeof( groupData ) );
    // Поместить группу для потребителя 0 в канал ППД
    if( client.put(value) != 0 )
        {
            printf( "Group value put error:%i\n", client.getError() ); return -1;
        }
    printf( "Ok!\n" );
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
        printf( "Receive CH[%i] ANA[%i]:%.5f(%i)\n",
        chnlId,value.idx,dataPtr->value, dataPtr->quality );
    }
    break;
    case Bin_VT :
    {
        BData * dataPtr = NULL; value.getData( dataPtr );
        printf( "Receive CH[%i] BIN[%i]:%i(%i)\n",
        chnlId,value.idx,dataPtr->value,dataPtr->quality );
    }
    break;
    case Int_VT :
    {
        IData * dataPtr = NULL; value.getData( dataPtr );
        printf( "Receive CH[%i] INT[%i]:%i(%i)\n",
        chnlId,value.idx,dataPtr->value,dataPtr->quality );
    }
    break;
    case Grp_VT :
    {
        GData * dataPtr = NULL; value.getData( dataPtr );
        printf( "Receive CH[%i] GRP[%i] (GT:%i GL:%u)\n",
        chnlId,value.idx,dataPtr->groupType,dataPtr->size );
    }
    break;
    default : break;
    }
    return 0;
}
