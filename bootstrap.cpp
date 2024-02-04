#include <stdio.h>
#include <cstring>
#include <string> 
#include <iostream>
#include <vector>
#include <locale>
#include <iomanip>
#include <cassert>
#include <map>
#include <chrono>
#include <time.h>
#include <random>
#include<algorithm>
#include<iterator>
#include "Stocks.h"
#include "bootstrap.h"
#include "Matrix.h"
#include "Group.h"

using namespace std;

namespace fre
{
    Bootstrap::Bootstrap(Group* GroupPtr_, map<string, Stocks>* MapPtr_, int n): GroupPtr(GroupPtr_),MapPtr(MapPtr_), T(2*n)
    {
    }

    // Random sample generator
    String Bootstrap :: generateSample(int gr_n)
    {
        int max_size = (GroupPtr->GetGroup_Mapping())[gr_n].size();
        String grouptickers(max_size);
        grouptickers = (GroupPtr->GetGroup_Mapping())[gr_n];
        String W(M);
        vector<int> numbers;
        vector<int> sample;
        for (int i = 0; i < max_size; i++) {numbers.push_back(i);}
	    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	    std::shuffle(numbers.begin(), numbers.end(), std::default_random_engine(seed));
    	for (int i = 0; i < M; i++)
    	{
    		sample.push_back(numbers[i]);
    	}
    	for (int i = 0; i < (int)sample.size(); i++)
    	{
    		W[i] = grouptickers[sample[i]];  
    	}
	    return W;
    }
    
    // Return cumulative sum of input vectors
    // Used for CAAR calculation across sample stocks (of 1 sample) for 2N timesteps (2N x 1)
    Vector Bootstrap :: cumsum(const Vector& V)
    {
        int d = (int)V.size();
        Vector U(d);
        U[0] = V[0];
        for(int j = 1; j < d; j++)
        {
            U[j] = U[j-1] + V[j];
        }
        return U; 
    }
    
    //Abnormal return calculation
    Vector Bootstrap :: AbnormRet(string ticker)  
    {
        Vector AbnormReturn = ConstVector(0,T);
        int start, end;
        start = (*MapPtr)[ticker].GetStartIndex();
        end = (*MapPtr)[ticker].GetEndIndex();
        
        if ((end - start) != T){
            
            
            return AbnormReturn;
        }

        Vector R = (*MapPtr)[ticker].GetReturns();
        Vector Benchmark = (*MapPtr)["IWV"].GetReturns();
        for(int i=0; i<T; i++)
        {
         AbnormReturn[i] = R[start+i] - Benchmark[start+i];
        }
        return AbnormReturn;
    }
 
    // return AAR calculation across sample stocks (of 1 sample) for 2N timesteps (2N x 1)
    Vector Bootstrap :: Cal_AAR(int gr_n) 
    {
        
        String sample(M);
       
        Matrix Ave = ConstMatrix(0,M,T);
        sample = generateSample(gr_n);
        
        for(int i = 0; i< M; i++)
        {
            Vector tmp = AbnormRet(sample[i]);
            for (int k = 0; k< T; k++){
                Ave[i][k] = tmp[k];
            }
        }
        Vector average_AAR(T);
        for (int i = 0; i < T; i++){
            for (int k = 0; k < M; k++ ){
                average_AAR[i] += Ave[k][i];
            }
            average_AAR[i] = average_AAR[i] / M;
        }
        return average_AAR;
        
    }
    
     // Run bootstrap and populate Avg_AAR, AAR-STD, Avg_CAAR, CAAR_STD
    void Bootstrap :: RunBootstrap()  
    {
        int N_Group = GroupPtr->GetN(); // number of groups. In this case 3
        //initialize result matrices to 0s
        Avg_AAR = ConstMatrix(0,N_Group,T); //group x time 
        Avg_CAAR = ConstMatrix(0,N_Group,T);//group x time 
        AAR_STD = ConstMatrix(0,N_Group,T); //group x time 
        CAAR_STD = ConstMatrix(0,N_Group,T);//group x time 
        Results.resize(N_Group); //initialize Results Matrix

        for(int n = 0; n < N_Group; n++) //iterate through each group
        {
            Vector Sum_AAR_tmp(T), Sum_CAAR_tmp(T);
            for(int i = 0;i < MCN; i++) //iterate through each monte carlo iteration (i.e. Bootstrap iteration 1-40) 
            {
                Vector AAR_tmp(T), CAAR_tmp(T);
        
                AAR_tmp = Cal_AAR(n);
                Sum_AAR_tmp += AAR_tmp; //operator overload
                AAR_STD[n] += AAR_tmp*AAR_tmp;
                CAAR_tmp = cumsum(AAR_tmp);
                Sum_CAAR_tmp += CAAR_tmp; //operator overload
                CAAR_STD[n] += CAAR_tmp*CAAR_tmp; //operator overload
            }
            double one_by_MCN = (double)(1.0/MCN);
        
            for(int j=0;j<T;j++)
            {
                Avg_AAR[n][j] = one_by_MCN*Sum_AAR_tmp[j]; 
                Avg_CAAR[n][j] = one_by_MCN*Sum_CAAR_tmp[j];
                AAR_STD[n][j] = sqrt((one_by_MCN*AAR_STD[n][j] - Avg_AAR[n][j]*Avg_AAR[n][j]));
                CAAR_STD[n][j] = sqrt(one_by_MCN*CAAR_STD[n][j] - Avg_CAAR[n][j]*Avg_CAAR[n][j]);
            }
            
            Results[n].push_back(Avg_AAR[n]);
            Results[n].push_back(AAR_STD[n]);
            Results[n].push_back(Avg_CAAR[n]);
            Results[n].push_back(CAAR_STD[n]);
        }
    }
    
    void Bootstrap::DisplayResults(int n)
    {
        string group_name;
        if(n == 0)
            group_name = "Beat";
        else if (n == 1)
            group_name = "Meet";
        else
            group_name = "Miss";
            

        cout<<"\n\nAverage Abnormal Returns (AAR) "<<endl;

        cout << "AAR - Group "<<group_name<< endl;
        for(int i = 0; i < (int)Avg_AAR[n].size(); i++)
        {
            if(i%10 == 0) cout << endl;
            cout<<setw(10)<<setfill(' ')<<std::fixed << std::setprecision(3)<<100*Avg_AAR[n][i]<<" ";

        }
        

        cout<<"\n\nAverage Abnormal Returns standard deviation (AAR-std) "<<endl;

        cout << "AAR_STD - Group "<<group_name<< endl;
        for(int i = 0; i < (int)AAR_STD[n].size(); i++)
        {
            if(i%10 == 0) cout << endl;
            cout<<std::setw(10)<<std::setfill(' ')<<std::fixed << std::setprecision(3)<<100*AAR_STD[n][i]<<" ";
        }
        cout<<"\n\nCumulative Average Abnormal Returns (CAAR) "<<endl;

        cout << "CAAR - Group "<<group_name<< endl;
        
        for(int i = 0; i < (int)Avg_CAAR[n].size(); i++)
        {
        
            if(i%10 == 0) cout << endl;
            cout<<std::setw(10)<<std::setfill(' ')<<std::fixed << std::setprecision(3)<<100*Avg_CAAR[n][i]<<" ";

        }
        

        cout<<"\n\nCumulative Average Abnormal Returns (CAAR-std) "<<endl;
        cout << "CAAR_STD - Group "<<group_name<< endl;
        for(int i = 0; i < (int)CAAR_STD[n].size(); i++)
        {
            if(i%10 == 0) cout << endl;
            cout<<setw(10)<<setfill(' ')<<std::fixed << std::setprecision(3)<<100*CAAR_STD[n][i]<<" ";
        }
    }
    
    
    // Getter functions
    // Return the vectors of Avg_AAR, AAR-STD, Avg_CAAR, CAAR_STD for each group from respective matrices
    Vector Bootstrap::GetAAR(int gr_index) const
    {
        return Avg_AAR[gr_index];
    }
    Vector Bootstrap::GetAAR_STD(int gr_index) const
    {
        return AAR_STD[gr_index];
    }
    Vector Bootstrap::GetCAAR(int gr_index) const
    {
        return Avg_CAAR[gr_index];
    }
    Vector Bootstrap::GetCAAR_STD(int gr_index) const
    {
        return CAAR_STD[gr_index];
    }
    
    //Plot GNU Plot for Avg_CAAR for each group (Beat, Meet, Miss)
    void Bootstrap :: plotResults(Vector r1, Vector r2, Vector r3, int N) 
    {
        
        FILE *gnuplotPipe,*tempDataFile;
        
        const char *tempDataFileName1 = "Beat";
        const char *tempDataFileName2 = "Meet";
        const char *tempDataFileName3 = "Miss";
        
        //set up the gnu plot
        gnuplotPipe = popen("/usr/bin/gnuplot", "w");
        fprintf(gnuplotPipe,"set terminal png size 800,600; set output 'Results.png'\n");
        fprintf(gnuplotPipe, "set grid\n");
        fprintf(gnuplotPipe, "set title 'Avg CAAR for 3 groups'\n");
        fprintf(gnuplotPipe, "set arrow from 0,graph(0,0) to 0,graph(1,1) nohead lc rgb 'red'\n");
        fprintf(gnuplotPipe, "set xlabel 'Announcement Date'\nset ylabel 'Avg CAAR (%)'\n");
        
        if (gnuplotPipe) 
        {
            fprintf(gnuplotPipe,"plot \"%s\" with lines lw 3, \"%s\" with lines lw 3, \"%s\" with lines lw 3 \n",tempDataFileName1, tempDataFileName2, tempDataFileName3);
            fflush(gnuplotPipe);
            
            //plot Avg_CAAR for Beat
            tempDataFile = fopen(tempDataFileName1,"w");
            for (int j = -N; j < N; j++)
            {
                fprintf(tempDataFile, "%i %lf\n", j, r1[j + N]);
            }
            fclose(tempDataFile);
            
            //plot Avg_CAAR for Meet
            tempDataFile = fopen(tempDataFileName2,"w");
            for (int j = -N; j < N; j++)
            {
                fprintf(tempDataFile, "%i %lf\n", j, r2[j + N]);
            }
            fclose(tempDataFile);
            
            //plot Avg_CAAR for Miss
            tempDataFile = fopen(tempDataFileName3,"w");
            for (int j = -N; j < N; j++)
            {
                fprintf(tempDataFile, "%i %lf\n", j, r3[j + N]);
            }
            fclose(tempDataFile);
        } 
        else 
        {        
            printf("gnuplot not found...");    
        }
    }
}
