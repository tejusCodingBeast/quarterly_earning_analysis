#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include <string>
#include <set>
#include <map>
#include "Stocks.h"
#include <iostream>
#include <iomanip>

using namespace std;

namespace fre{

    int Stocks::SetDates()
	{
		auto itr = find(Date.begin(), Date.end(), earnings_date);

		int index = std::distance(Date.begin(), itr);
		
		if(index + N >= (int)Adjusted_close.size() and index - N < 0)
		{
			start_index = 0;
			end_index = (int)Adjusted_close.size() -1;
			full_data = 0;
			return -1;
		}
		else if (index + N >= (int)Adjusted_close.size())
		{
			start_index = index - N;
			end_index = (int)Adjusted_close.size() - 1;
			full_data = 0;
			return -1;
		}
		else
		{
			start_date = Date[index - N];
			end_date = Date[(index + N)];
			SetStartIndex(index - N);
			SetEndIndex(index + N);
			full_data = 1;
		}
		return 1;
	}
	
	void Stocks::CalculateReturns()
	{
		float daily_return;
		for (int i = 1; i < (int)Adjusted_close.size(); i++)
		{
			daily_return = Adjusted_close[i]/Adjusted_close[i-1] - (float)1.0;
			pct_returns.push_back(daily_return);
		}
	}
	
	void Stocks::CalculateCumulativeReturns()
	{
		
		Cumulative_returns.push_back(pct_returns[0]);
		for(int i = 1; i < (int)pct_returns.size(); i++)
		{
			Cumulative_returns.push_back(Cumulative_returns.back() + pct_returns[i]);
		}
	}
	
	void Stocks::CalculateAbnormalReturns(map<string, Stocks> &GlobalStockMap)
	{
		Vector Benchmark = GlobalStockMap["IWV"].GetReturns();
		for(int i = 0; i < (int)pct_returns.size(); i++)
		{
			Abnormal_returns.push_back(pct_returns[i] - Benchmark[i]);
		}
	}
	
	int Stocks::SetN(int N_)
	{
		N = N_;
		int check_present_in_list = SetDates();
		
		return check_present_in_list;
	}
	
	void Stocks::DisplayDetails()
	{
		
		cout<<"Ticker: "<<ticker<<endl;
		cout<<"Earnings Announcement Date: "<<earnings_date<<endl;
		cout<<"Earnings Period Ending: "<<period_ending<<endl;
		cout<<"Earning Estimate: $"<<estimated_earnings<<endl;
		cout<<"Reported Earnings per share: $"<<reported_earnings<<endl;
		cout<<"Earnings Surprise: $"<<surprise_earnings<<endl;
		cout<<"Earnings surprise percent: "<<surprise_perecent<<"%"<<endl;
		
		if(full_data == 1)
			cout<<"Sufficient data: True\n";
		else
			cout<<"Sufficient data: False\n";
		
		cout<<"Group: "<<group<<endl<<endl;
		
		if(full_data == 1)
		{
		
			cout<<"Stock Daily Prices Dates: \n";
			for(int i = start_index; i <= end_index; i++)
			{
				cout<<setw(13)<<setfill(' ')<<Date[i]<<" ";
	
				if( (i - start_index + 1) % 10 == 0)
					cout<<endl;
			}
			
			cout<<"\n\nStock Daily Prices: \n";
			for(int i = start_index; i <= end_index; i++)
			{
				cout<<setw(10)<<setfill(' ')<<std::fixed << std::setprecision(3)<<Adjusted_close[i]<<" ";
				if( (i - start_index + 1) % 10 == 0)
					cout<<endl;
			}
			
			cout<<"\n\nStock Daily Returns: \n";
			for(int i = start_index; i < end_index; i++)
			{
				cout<<setw(10)<<setfill(' ')<<std::fixed << std::setprecision(3)<<pct_returns[i]<<" ";
				if( (i - start_index + 1) % 10 == 0)
					cout<<endl;
			}
			
			cout<<"\n\nStock Cumulative Daily Returns: \n";
			for(int i = start_index; i < end_index; i++)
			{
				cout<<setw(10)<<setfill(' ')<<std::fixed << std::setprecision(3)<<(Cumulative_returns[i] - Cumulative_returns[start_index-1])<<" ";
				if( (i - start_index + 1) % 10 == 0)
					cout<<endl;
			}
			
			cout<<"\n\nStock Abnormal Daily Returns: \n";
			for(int i = start_index; i < end_index; i++)
			{
				cout<<setw(10)<<setfill(' ')<<std::fixed << std::setprecision(3)<<Abnormal_returns[i]<<" ";
				if( (i - start_index + 1) % 10 == 0)
					cout<<endl;
			}
		}
	}
	
}
