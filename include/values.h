//
// Значения
//
// Составил Рябов М.В.

#ifndef __VALUES_H__
#define __VALUES_H__

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>

// Количество используемых битов в качестве значений
const unsigned DTSQualityBitNum = 12;

// Качества значений DTS
#define GOOD_QUAL					0
#define DEV_FAIL_QUAL				0x0001
#define CHNL_FAIL_QUAL				0x0002
#define NOT_INIT_QUAL				0x0004
#define INVALID_TIME_QUAL			0x0008
#define REQ_DATA_QUAL				0x0010
#define CYCLIC_DATA_QUAL			0x0020
#define CHANGE_FLTR_QUAL			0x0040
#define RECOVER_DATA_QUAL			0x0080
#define IMIT_DATA_QUAL				0x0100
#define OUT_OF_RANGE_QUAL			0x0200
#define DTS_CHNL_FAIL_QUAL			0x0400
#define DTS_CYCLIC_DATA_QUAL		0x0800

// Размер данных группового значения
#define GRP_DATA_SIZ				4096
//
// Типы значений
//
enum ValueType
	{
	NoType_VT = -1,
	Ana_VT,
	Bin_VT,
	Int_VT,
	Grp_VT,
	Total_VT
	};
//
// Аналоговые значения
//
class AData
	{
public:
	int32_t quality;		// Битовое качество
	float value;			// Вещественное значение,IEEE
	};
//
// Дискретные значения
//
class BData
	{
public:
	int32_t quality;		// Битовое качество
	int32_t value;		// Значение
	};
//
// Целочисленное значение
//
class IData
	{
public:
	int32_t quality;		// Битовое качество
	int32_t value;			// Значение
	};
//
// Групповое значение
//
class GData
	{
public:
	int32_t groupType;					// Тип группы
	uint32_t size;						// Размер массива value,байты
	uint8_t value[GRP_DATA_SIZ];			// Данные группы
	};
//
// Общее значение
//	
class Value
	{

public:
	int32_t type;				// Тип значение (enum ValueType)
	int32_t idx;				// Индекс значения
	uint32_t sec;				// Время создания, секунды UTC
	uint32_t msec;			// Миллисекунды последней секунды создания
private:
	//Тело значения
	union
		{
		AData adata;
		BData bdata;
		IData idata;
		GData gdata;
		} data;
public:
	// Получить данные	
	inline AData * getData( AData * & _ptr ){ return _ptr = ( type == Ana_VT )? (AData *)&data : NULL; }
	inline BData * getData( BData * & _ptr ){ return _ptr = ( type == Bin_VT )? (BData *)&data : NULL; }
	inline IData * getData( IData * & _ptr ){ return _ptr = ( type == Int_VT )? (IData *)&data : NULL; }
	inline GData * getData( GData * & _ptr ){ return _ptr = ( type == Grp_VT )? (GData *)&data : NULL; }

	// Присвоение
	Value & operator = ( Value &  );
	inline Value & operator << ( Value & _src ){ *this = _src; return *this; }
		
	Value()
		{ 
		idx = -1; type = NoType_VT; sec = 0; msec = 0; data.gdata.size = 0; data.gdata.groupType = -1;
		}
	Value( ValueType _type, int32_t _idx = -1 )
		{
		type = _type; idx = _idx; data.gdata.size = 0; data.gdata.groupType = -1;
		}
	Value( ValueType _type, int32_t _idx, uint32_t _sec, uint32_t _msec )
		{ 
		type = _type; idx = _idx; data.gdata.size = 0; data.gdata.groupType = -1;
		if( _sec != 0 || _msec != 0 ){ sec = _sec; msec = _msec % 1000; }
		else 
			{ 
			timeval tv; gettimeofday( &tv,NULL ); sec = tv.tv_sec; msec = tv.tv_usec / 1000;
			} 
		}
	};	
#define HDR_VAL_SIZ	( sizeof( Value ) - sizeof( GData ) )
#define ANA_VAL_SIZ	( HDR_VAL_SIZ + sizeof( AData ) )
#define BIN_VAL_SIZ	( HDR_VAL_SIZ + sizeof( BData ) )
#define INT_VAL_SIZ	( HDR_VAL_SIZ + sizeof( IData ) )
#define GRP_VAL_SIZ	( HDR_VAL_SIZ + sizeof( GData ) )
//
// Служебные функции работы со значениями	
//
// Получить размер значения
uint32_t getLeng( Value * );
inline uint32_t getLeng( Value & _value ){ return getLeng( &_value ); }
size_t getMaxValueLeng( int32_t );
inline size_t getMaxValueLeng( Value * _valuePtr ){ return ( _valuePtr == NULL )? 0 : getMaxValueLeng( _valuePtr->type ); }
inline size_t getMaxValueLeng( Value & _value ){ return getMaxValueLeng( _value.type ); }

// Проверка типа значения
int isValue( Value * _valuePtr );
inline int isValue( Value & _value ){ return isValue( &_value ); }

// Сравнение значений
inline int operator == ( Value & _value1,Value & _value2 )
	{ 
	uint32_t len_1 = getLeng( _value1 ); return( len_1 == 0 || len_1 != getLeng( _value2 ) || memcmp( &_value1,&_value2,len_1 ) != 0 )? 0 : 1;
	}
inline int operator != ( Value & _value1,Value & _value2 ){ return ( _value1 == _value2 )? 0 : 1; }

// Функции формирования значений
int makeAValue( Value *,int32_t,int32_t,float,uint32_t sec = 0, uint32_t msec = 0 );
int makeBValue( Value *,int32_t,int32_t,int32_t,uint32_t sec = 0, uint32_t msec = 0 );
int makeIValue( Value *,int32_t,int32_t,int32_t,uint32_t sec = 0, uint32_t msec = 0 );
int makeGValue( Value *,int32_t,int32_t,void *,uint32_t,uint32_t sec = 0, uint32_t msec = 0 );

inline int makeAValue( Value & _value,int32_t _idx,int32_t _quality,float _result,uint32_t _sec = 0, uint32_t _msec = 0 )
	{
	return makeAValue( &_value,_idx,_quality,_result,_sec,_msec );
	}
inline int makeBValue( Value & _value,int32_t _idx,int32_t _quality,int32_t _result,uint32_t _sec = 0, uint32_t _msec = 0 )
	{
	return makeBValue( &_value,_idx,_quality,_result,_sec,_msec );
	}
inline int makeIValue( Value & _value,int32_t _idx,int32_t _quality,int32_t _result,uint32_t _sec = 0, uint32_t _msec = 0 )
	{
	return makeIValue( &_value,_idx,_quality,_result,_sec,_msec );
	}
inline int makeGValue( Value & _value,int32_t _idx,int32_t _groupType,void * _dataPtr,uint32_t _size,uint32_t _sec = 0, uint32_t _msec = 0 )
	{
	return makeGValue( &_value,_idx,_groupType,_dataPtr,_size,_sec,_msec );
	}
#endif
