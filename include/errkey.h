#ifndef __ERRKEY_H__
#define __ERRKEY_H__

#include <limits.h>
#include <stdint.h>

// Максимальный размер строки сообщения об ошибке
#define ERR_STR_LEN		256

enum ErrFlag
	{
	Ok = 0,
	NotInit = SHRT_MIN,
	BadInit,
	AlreadyInUse,
	UseBeforeInit,
	NewerUsed,
	
	//Группа блокировок
	BlockBySystem,
	BlockByExtern,
	BlockByUser,
	
	// Группа "плохих"
	BadCommand,
	BadArg,
	BadSize,
	BadType,
	BadInput,
	BadOutput,
	BadString,
	BadOffs,
	BadHardware,
	BadCfg,
	BadVersion,
	
	// Группа "вне диапазона"
	Overflow,
	Underflow,
	NotInRange,
	Overtime,
	NoMem,
	
	// Группа "не сделано потому, что..."
	NotFind,
	NotAllowed,
	NotReadyBut,
	NotStored,
	NotAccess
	};
//
// Базовый класс ошибок
//
class ErrKey
	{
private :
	int32_t error;				// Ошибка последней операции

protected :
	ErrFlag state;			// Cостояние объекта
	char errstr[ERR_STR_LEN];	// Строка ошибки последней операции

	// Установка ошибки
	inline void _setError( int16_t _error, uint16_t _moduleId = 0, uint16_t _moduleNum = 0 )
		{
		error = ( ( (int32_t)_moduleId & 0xFF ) << 24 ) + ( ( (int32_t)_moduleNum & 0xFF ) << 16 ) + ( _error & 0xFFFF );
		}
	ErrKey(){ state = BadInit; error = 0; *errstr = 0; }
	virtual ~ErrKey(){}

public :
	// Получить состояние
    // RMV-DtsClient-01-16022022
	virtual ErrFlag getState(){ return state; }

	// Получить строку ошибки
	inline const char * getErrStr(){ return errstr; }

	// Получить расширенный код ошибки
	inline int16_t getError(){ return (int16_t)error; }
	inline int32_t getModuleId(){ return error >> 24; }
	inline int32_t getFuncNum(){ return ( error >> 16 ) & 0xFF; }

	// Операторы сравнений
    // RMV-DtsClient-01-16022022
	inline int operator == ( ErrFlag _state ){ return ( getState() == _state )? 1 : 0; }
	inline int operator != ( ErrFlag _state ){ return ( getState() != _state )? 1 : 0; }
	};
#endif
