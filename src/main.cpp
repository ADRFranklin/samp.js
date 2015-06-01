#pragma comment(lib, "winmm.lib")


#include "sdk.h"

#include "samp/SAMP_Callbacks.h"
#include "SAMP_JS.h"
#include "utils/SAMP_Utils.h"
#include "io/SAMP_FileSystem.h"
#include "samp/SAMP_Players.h"

#define NOMINMAX


#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <string> 
#include <map>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_BUGFIX 4

typedef void(*logprintf_t)(char* format, ...);
logprintf_t logprintf;

extern void *pAMXFunctions;

std::vector<std::string> js_scripts;


std::vector<std::string> split(std::string str){
	std::regex re("[\\s,]+");
	std::sregex_token_iterator it(str.begin(), str.end(), re, -1);
	std::sregex_token_iterator reg_end;
	std::vector<std::string> arr;
	for (; it != reg_end; ++it) {
		arr.push_back(it->str());
	}
	return arr;
}

void ReadConfig(){
	std::ifstream config("server.cfg");
	if (!config){
		std::cout << "Config File does not exist" << std::endl;
	}
	else {
		std::string line;
		while (std::getline(config, line)){
			std::vector<std::string> args = split(line);
			if (args.size() > 1){
				if (args[0] == "jsfiles"){
					for (unsigned int i = 1; i < args.size(); i++){
						js_scripts.push_back(args[i]);
					}
				}
			}
		}
	}
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports(){
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData){
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	std::cout << std::endl;
	std::cout << std::string(30, '-') + " samp.js " + std::string(30, '-') << std::endl;
	std::cout << "*** Loaded samp.js v" << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_BUGFIX << " by !damo!spiderman ***" << std::endl;
	std::cout << std::string(30, '-') + std::string(9,'-') + std::string(30, '-') << std::endl;
	std::cout << std::endl;

	ReadConfig();
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload(){
	std::cout << std::endl;
	std::cout << std::string(30, '-') + " samp.js unloaded " + std::string(30, '-') << std::endl;
	std::cout << std::endl;
}
#include "samp/SAMP_Natives.h"

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx){
	
	amx_Register(amx, PluginNatives, -1);

	int idx;
	if (!amx_FindPublic(amx, "SAMPJS_Init", &idx)){
		if (js_scripts.size() > 0){
			for (unsigned int i = 0; i < js_scripts.size(); i++){
				SAMP_JS* jsfile = new SAMP_JS();
				jsfile->SetAMX(amx);

				jsfile->AddModule("utils", new SAMP_Utils(jsfile));
				jsfile->AddModule("$fs",new SAMP_FileSystem(jsfile));
				jsfile->AddModule("players", new SAMP_Players(jsfile));
				
				JS_SCOPE(jsfile->GetIsolate())
				JS_CONTEXT(jsfile->GetIsolate(), jsfile->_context)
				Local<Value> ret = jsfile->LoadScript("js/"+js_scripts[i]);
				String::Utf8Value jsStr(ret);
				char* str = *jsStr;
				SAMP_JS::_scripts[js_scripts[i]] = jsfile;

			}
			std::cout << std::endl;
		}
		else {
			std::cout << "[samp.js] No JS Scripts configured. Add jsfiles to your server.cfg" << std::endl;
		}
	}
	return 1;
}


PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx){
	std::cout << "AMX UNLOADED" << std::endl;
	for (auto it = SAMP_JS::_scripts.begin(); it != SAMP_JS::_scripts.end();){
		if (it->second->GetAMX() == amx){
			it->second->Shutdown();
			SAMP_JS::_scripts.erase(it++);
		}
		else {
			++it;
		}
	}
	return AMX_ERR_NONE;
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick(){
	for (auto it = SAMP_JS::_scripts.begin(); it != SAMP_JS::_scripts.end();++it){
		it->second->ProcessTick();
	
	}
}


