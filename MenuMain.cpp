#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <stdexcept> 
#include <iomanip>
#include <vector>
#include <map>
#include <chrono>
#include <sstream> 
#include <typeinfo>
#include "ReadLoadData.h"
#include "Stocks.h"
#include "Group.h"
#include "bootstrap.h"
#include "Matrix.h"
#include <thread>

using namespace std;
using namespace std::chrono;
using namespace fre;

//map and vector of vector prints?? are they included in header files ?
String SetN(int N, map<string, Stocks> &stock_map)
{

    auto itr2 = stock_map.begin();
    
    String skipped_tickers;
    
    for(;itr2 != stock_map.end(); itr2++)
    {
        int flag = (itr2->second).SetN(N);
        if(flag == -1)
        {
            skipped_tickers.push_back(itr2->first);
            
        }
        
    }
    
    cout<<"\nWarning: No. of stocks with no data for given n (will be skipped): "<<skipped_tickers.size()<<endl;
    cout<<"Tickers for these stocks: "<<endl;
    for(int i = 0; i < (int)skipped_tickers.size(); i++)
    {
        cout<<setw(8)<<setfill(' ')<<skipped_tickers[i]<<" ";
        if((i+1)%10 == 0)
            cout<<endl;
        
    }
    return skipped_tickers;
}

void CalAbnormalReturns(map<string, Stocks> &stock_map)
{
    auto itr = stock_map.begin();
    for(; itr != stock_map.end(); itr++)
    {
        (itr->second).CalculateAbnormalReturns(stock_map);
    }
}


int main() 
{
    
    int group,test =0;  // menu option and number of days 
    string x;
    string tick;
    int N=0;
    int integer1;

    map<string, Stocks> GlobalStockMap;
    Stocks Russel;
    
    LoadEarnings(GlobalStockMap);
    
    String skipped_tickers;
    
    GlobalStockMap["IWV"] = Russel;
    
    Groupby_Surprise gobj(&GlobalStockMap);
    gobj.CreateGroups(skipped_tickers);
    
    Table groupTable = gobj.GetGroup_Mapping();
    
    Bootstrap GlobalBoot;
    // create object of bootstrap or our stock object with our data Bootstrap test;
    Matrix AAR;
    
    int fetched = 0;
    
    while(true)
    {
        cout<<"\n\nMenu: "<<endl;
        cout<<"1) Enter N to retrieve 2N+1 historical prices for all stocks (Please allow around 2 mins to fetch)"<<endl;
        cout<<"2) Pull information for one stock from one group. "<<endl;
        cout<<"3) Display AAR, AAR-std, CAAR, CAAR-std for one group. "<<endl;
        cout<<"4) Display gnuplot plot for CAAR of all 3 groups.  "<<endl;
        cout<<"5) Exit program "<<endl<<endl;
        cout<<"Please select appropriate option number:  "<<endl;;
        cin>>x;
        
        
            if(x == "1")
            {

                while(test==0)
                {
                    
                    cout<<"Enter N value between 60 and 80: "<<endl;
                    cin>>N;
                    integer1 = (int) N;
                    
                    
                    if (integer1 != N)
                    {
                        cout<<"Error, please enter an integer value error"<<endl;
                        cin.clear();
                    }
                    else if(N>=60 && N<=80 && !cin.fail())  // n is an int 
                    {
                        
                        if (fetched == 0)
                        {
                            cout<<"Loading data, please allow around 2 mins to load...\n";
                            auto start = high_resolution_clock::now();
                            
                            thread t1(FetchData,std::ref(GlobalStockMap),groupTable[0]);
                            thread t2(FetchData,std::ref(GlobalStockMap),groupTable[1]);
                            thread t3(FetchData,std::ref(GlobalStockMap),groupTable[2]);
                            t1.join();
                            t2.join();
                            t3.join();
                            
                            String Index;
                            
                            Index.push_back("IWV");
                            FetchData(GlobalStockMap,Index);
                            
                            auto stop = high_resolution_clock::now();
                            auto duration = duration_cast<seconds>(stop - start);
                            cout<<"Fetched data in seconds: "<<duration.count()<<endl;
                            fetched  = 1;
                            CalAbnormalReturns(GlobalStockMap);
                        }
                        
                        AAR = GlobalBoot.GetAAR();
                        if(AAR.size() != 0)
                            AAR.clear();
                            
                        skipped_tickers = SetN(N, GlobalStockMap);
                        gobj.CreateGroups(skipped_tickers);
                        test = 1;
                    }
                    else
                    {
                        cout<<"Error, Enough data points not found. Please select N above 60 and below 80 "<<endl;
                        cin.clear();
                    }
                }
                
            }
            else if(x == "2")
            {   
                if (N>=60 && N<=80)
                {
                    while(test==0)
                    {
                        cout<<"Please provide ticker of stock: "<<endl;
                        cin>> tick;
                        
                        if(GlobalStockMap.find(tick) != GlobalStockMap.end())
                        {
                            GlobalStockMap[tick].DisplayDetails();
                            test = 1;
                        }
                        else
                        {
                            cout<<"Error, Please enter a valid ticker. Example: Try 'GOOG'\n";
                            //break;
                        }
                        
                        
                        if(!cin.fail())
                        {
                            test = 1;
                        }
                        else 
                        { 
                            cout<<"Error, Invalid Input field. Example: Try GOOG ";   // try and error c++ 
                        }
                    }
                }
                else
                {
                    cout<<"Data has not yet been retrieved! Try retrieving data first using Option 1 in Main Menu!"<<endl;
                }
                
            }
            else if(x == "3")
            {
                if(N>=60 && N<=80)
                {   
                    Bootstrap boot(&gobj, &GlobalStockMap, N);
                    boot.RunBootstrap();
                    cout << "\nBootstrap object created" << endl;
                    while(test==0)
                    {
                        cout<<"1) Beat \t 2) Meet \t 3) Miss"<<endl;
                        cout<< "Please select appropriate group number: "<<endl; 
                        cin>>group;
                        int integer2 = (int) group;
                        
                        if(integer2 != group)
                        {
                            cout<<"Invalid Input field"<<endl;
                            cin.clear();
                            
                        }
                        else if(group == 1)   //switch?
                        {
                            cout<<" Beat Estimate Group summary "<<endl;
                            boot.DisplayResults(group-1);

                            test = 1;
                        }
                        else if(group == 2)
                        {
                            cout<<"\n Meet Estimate Group summary "<<endl;
                            boot.DisplayResults(group-1);
                            test = 1;
                        }
                        else if(group == 3)
                        {
                            
                            cout<<" Miss Estimate Group summary "<<endl;
                            boot.DisplayResults(group-1);
                            test = 1;
                        }
                        else 
                        {
                            cout<<"Error, Please select valid option number"<<endl;
                        }
                    
                    }
                    GlobalBoot = boot;
                    AAR = GlobalBoot.GetAAR();
                    
                }
                
                else
                {
                    cout<<"Data has not yet been retrieved! Try retrieving data first using Option 1 in Main Menu!"<<endl;
                }
                
            }
            else if (x == "4")
            {   
                if(N>=60 && N<=80)
                {   
                    
                    if(AAR.size() == 0)
                    {
                        cout<<"Error, Please run Bootstrap in Option 3 first";
                
                    }
                    else
                    {
                        Vector g1 = GlobalBoot.GetCAAR(0);
                        Vector g2 = GlobalBoot.GetCAAR(1);
                        Vector g3 = GlobalBoot.GetCAAR(2);
                        
                        plotResults(100*g1, 100*g2, 100*g3, N);
                        
                        GlobalBoot.plotResults(100*g1, 100*g2, 100*g3, N);    
                    }
                    
                    
                }
                else
                {
                    cout<<"Data has not yet been retrieved! Try retrieving data first using Option 1 in Main Menu!"<<endl;
                }
                
            }
            else if(x == "5")
            {
                exit(0);
            }
            else 
            {
                cout<<"Error, Please select valid option "<< endl;
                cin.clear();
                test = 1;
                //break;
            }
        
         
        test = 0;
        
    }
    
    return 0;
     
}
