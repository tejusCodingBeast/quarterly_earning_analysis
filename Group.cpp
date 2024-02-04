#include "Group.h"

using namespace std;
//base Group class
//default no. of groups = 3
//constructor
Group::Group(Map* data_):stockMapPtr(data_), N_group(3){
    vector<string> Group_Names({"Beat", "Meet", "Miss"});
}

//destructor
Group::~Group(){
}

//set no. of groups
void Group::SetN(int n){
    N_group = n;
}

//get no. of groups
int Group::GetN() const{
    return N_group;
}

//set group names
void Group::SetGroupNames(vector<string>& names){
    Group_Names = names;
}

//get group names
vector<string> Group::GetGroupNames() const{
    return Group_Names;
}

//update Stock Map
void Group::UpdateStockMap(Map* stockMapPtr_){
    stockMapPtr = stockMapPtr_;
}

//get Stock Map
Map Group::GetStockMap() const{
    return *stockMapPtr;
}

//return pointer to Group_Mapping Table
Table Group::GetGroup_Mapping(){ return Group_Mapping; }

//overloaded subscript operator to return by groupname
vector<string> Group::operator[] (const string& groupname){
    if (groupname == Group_Names[0]) return Group_Mapping[0];
    else if (groupname == Group_Names[1]) return Group_Mapping[1];
    else if (groupname == Group_Names[2]) return Group_Mapping[2];
    return Group_Mapping[0];
}

//Groupby_Surprise derived class
//constructor
Groupby_Surprise::Groupby_Surprise(Map* stockMapPtr_): Group(stockMapPtr_){}

//destructor
Groupby_Surprise::~Groupby_Surprise(){}

//comparator 
bool compare(Stocks& a, Stocks& b){
    return (a.GetSurprisePerecent() > b.GetSurprisePerecent());
}

//Create groups by suprise percent
void Groupby_Surprise::CreateGroups(String skipped_tickers) 
{
    Group_Mapping.resize(3);
    vector<Stocks> stockVector;
    for (auto it = stockMapPtr->begin(); it != stockMapPtr->end(); ++it) 
    {   
        if(std::find(skipped_tickers.begin(), skipped_tickers.end(), it->first) != skipped_tickers.end()) //skipping stocks which do not have (2N+1) data points
            continue;
        else if (it->first == "IWV") //skipping benchmark
            continue;
        stockVector.push_back(it->second);
    }
    sort(stockVector.begin(), stockVector.end(), compare);
    int NoOfStocks = (int)stockVector.size()/N_group;
    for (int i = 0; i < N_group; ++i)
    {   
        for (int j = i * NoOfStocks; j < min((i + 1)*NoOfStocks, int(stockVector.size())); ++j)
        {
            Group_Mapping[i].push_back(stockVector[j].GetTicker());
            if (i==0) (*stockMapPtr)[stockVector[j].GetTicker()].SetGroup("Beat");
            else if (i == 1) (*stockMapPtr)[stockVector[j].GetTicker()].SetGroup("Meet");
            else if (i == 2) (*stockMapPtr)[stockVector[j].GetTicker()].SetGroup("Miss");
        }
        
    }
}

//create groups, no. of groups as parameter
void Groupby_Surprise::CreateGroups(int n, String skipped_tickers){
    this->SetN(n);
    this->CreateGroups(skipped_tickers);
}

//create groups, stockMap and no. of groups as parameter
void Groupby_Surprise::CreateGroups(Map* stockMapPtr_, int n, String skipped_tickers){
    this->UpdateStockMap(stockMapPtr_);
    this->CreateGroups(n, skipped_tickers);
}




