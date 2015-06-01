#ifndef __SAMP_EVENTS__
#define __SAMP_EVENTS__


#include "sdk.h"

#include <vector>
#include <list> 
#include <include/v8.h>
#include <include/libplatform/libplatform.h>

class SAMP_JS;

class SAMP_Events {
public:
	static void Bind(const v8::FunctionCallbackInfo<v8::Value>& args);

	SAMP_Events(SAMP_JS* sampjs);
	int FireNative(std::string name, std::string param_types, std::vector<std::string> param_names, AMX* amx, cell* params);
private:
	SAMP_JS* _sampjs;
	
};

#endif