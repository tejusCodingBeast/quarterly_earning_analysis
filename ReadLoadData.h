#pragma once

#include <stdio.h>
#include <cstring>
#include <string> 
#include <iostream>
#include <sstream>  
#include <vector>
#include <locale>
#include <iomanip>
#include <fstream>
#include <curl/curl.h>
#include <map>
#include "Stocks.h"

namespace fre
{
    struct MemoryStruct 
    {
    	char* memory;
    	size_t size;
    };
    
    string FormatDate(string Date);
    
    void plotResults(Vector Beat, Vector Meet, Vector Miss, int N);
    
    void LoadEarnings(map<string, Stocks> &data);
    
    int write_data(void* ptr, int size, int nmemb, void* data);
    
    void FetchData(map<string, Stocks> &stock_map, String group_tickers);
}