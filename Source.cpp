#include<iostream>
#include<fstream>
#include<queue>
#include<vector>
#include<algorithm>
#include<iomanip>
using namespace std;

struct Process{
	string pid;
	int AT; // AT: Arrival Time
	int BT; // BT: Burst Time
	int FT = 0; // FT: Finish Time
	int ST;// ST: Start Time
	int TAT = 0; // TAT: Turnaround Time
	int WT = 0; // WT: Waiting Time
	int RT; // RT: Remaining Time	
	int lastExc;//lastExc: the last time process was excuted 
};

void print(const vector<Process>& processes, const vector<pair<string, int>>& ganttChart, int totalTime,int idleTime, const string& algo) {
	cout << "\n---------------------------------------------------\nThe Results for " << algo << " Scheduling:\n";
	cout << "\nGantt Chart:\n";

	for (const auto& g : ganttChart) {
		if(g.first== "Idle")
			cout << "| " << "Idle" << " ";
		else
            cout << "|  " << g.first << "  ";
	}
	cout << " |\n";

	for (const auto& g :ganttChart) {
		cout << g.second << "      ";
	}
	cout << totalTime << "\n\n";
	
	cout << "\nProcess\tAT\tBT\tStart\tFinish\tWT\tTAT\n";
	for (const auto& p : processes) {
		cout << p.pid << "\t" << p.AT << "\t" << p.BT<< "\t"
			<< p.ST << "\t" << p.FT << "\t" << p.WT << "\t" << p.TAT << "\n";
	}

	double avgWait = 0, avgTurnaround = 0;
	for (const auto& p : processes) {
		avgWait += p.WT;
		avgTurnaround += p.TAT;
	}
	avgWait /= processes.size();
	avgTurnaround /= processes.size();
	double CPU_Utilization = ((totalTime-idleTime) / (double)totalTime ) * 100;

	cout << "\nAverage Waiting Time: " << avgWait << "\n";
	cout << "Average Turnaround Time: " << avgTurnaround << "\n";
	cout << "CPU Utilization: " << fixed << setprecision(2) << CPU_Utilization << "%\n";
	cout << "---------------------------------------------------\n\n";
}

void FCFS(vector<Process> processes){
	vector<pair<string, int>> GanttChart;
	int CT = 0, idle = 0; // CT: Current Time

		for (auto& p : processes) {
			//Check if the process arrival time less than the current time(if CPU is idle in this time)
		    if (CT < p.AT){
			    GanttChart.emplace_back("Idle" , CT);
			    idle += (p.AT - CT);
			    CT = p.AT;
		    }
		    // if not idle the process will printed in the Gantt Chart and calculate the TAT and WT
		    GanttChart.emplace_back(p.pid, CT);
		    p.ST = CT;
		    CT += p.BT;
		    p.FT = CT;
		    p.TAT = p.FT - p.AT;
		    p.WT = p.ST - p.AT;
     	}
        print(processes, GanttChart, CT, idle, "FCFS");
}

void SRT(vector<Process> processes){
	vector<pair<string, int>> ganttChart;
	int CT = 0, completed = 0, idle = 0;
	int n = processes.size(); 
    //initialization
	for (auto& p : processes){
		p.RT = p.BT;
		p.ST = -1;
		p.lastExc = -1;
	}
	while (completed < n) { 
		int idx = -1, min_time = INT_MAX;

		//this loop to find the process with shortest remaining time 
		for (int i = 0; i < n; i++) {
			if (processes[i].AT <= CT && processes[i].RT > 0 && processes[i].RT < min_time) {
				idx = i; 
				min_time = processes[i].RT;
			} 
		} 
		// to check if cpu idle
		if (idx == -1) { 
			ganttChart.emplace_back("Idle", CT);
			idle++; 
			CT++; 
			continue; 
		} 
		// print in Gantt Chart
		if (ganttChart.empty() || ganttChart.back().first != processes[idx].pid){ 
			ganttChart.emplace_back(processes[idx].pid, CT);
		} 

		//initialization starting time
		if (processes[idx].ST == -1) {
			processes[idx].ST = CT;
		}

		// for checking the excution for process that leave CPU and enter again
		if (processes[idx].lastExc != -1 && processes[idx].lastExc < CT) {
			processes[idx].WT += (CT - processes[idx].lastExc);
		}
		//update values
		processes[idx].lastExc = CT;
		processes[idx].RT--;
		CT++;
		//calculate TAT and WT after remaining time finished
		if (processes[idx].RT == 0) {
			processes[idx].FT = CT;
			processes[idx].TAT = processes[idx].FT - processes[idx].AT; 
			processes[idx].WT += (processes[idx].ST - processes[idx].AT);
			completed++; 
		} 
	}
	print(processes, ganttChart, CT, idle, "SRT");
}

void RR(vector<Process> processes, int quantum){
	vector<pair<string, int>> ganttChart;
	queue<int> readyQueue;
	int CT = 0, completed = 0, idle = 0;
	int n = processes.size();
	vector<bool> isInQueue(n, false);

	//initialization
	for (auto& p : processes) {
		p.RT = p.BT;
		p.ST = -1;
		p.lastExc = 0;
	}

	readyQueue.push(0);
	isInQueue[0] = true;

	//push the process in the queue
	while (!readyQueue.empty()){
		int c = readyQueue.front();
		readyQueue.pop();
		isInQueue[c] = false;
		//check if the CPU idle
		if (CT < processes[c].AT) {
			ganttChart.emplace_back("Idle", CT);
			idle += (processes[c].AT - CT);
			CT = processes[c].AT;
		}
		//print in the Gantt Chart
		if (ganttChart.empty() || ganttChart.back().first != processes[c].pid){
			ganttChart.emplace_back(processes[c].pid, CT);
		}
		//initialization
		if (processes[c].ST == -1) {
			processes[c].ST = CT;
		}
		// for checking the excution for process that leave CPU and enter again
		if (processes[c].lastExc != -1 && processes[c].lastExc < CT) {
			processes[c].WT += (CT - processes[c].lastExc);
		} 

		int exc = min(processes[c].RT, quantum);
		processes[c].RT -= exc; 
		CT += exc;
		processes[c].lastExc = CT ;

        for (int i = 0; i < n; i++) {
			if (i != c && processes[i].AT <= CT && processes[i].RT > 0 && !isInQueue[i]) {
				readyQueue.push(i);
				isInQueue[i] = true;
				processes[i].WT += (CT - processes[i].AT);
			}
		}

		if (processes[c].RT > 0) {
			readyQueue.push(c);
			isInQueue[c] = true;
		}
		else {
			processes[c].FT = CT; 
			processes[c].TAT = processes[c].FT - processes[c].AT; 
			processes[c].WT = processes[c].TAT - processes[c].BT;
			completed++;
		}	
	}
	print(processes, ganttChart, CT, idle, "Round Robin");
}

int main()
{
	cout << "Enter the path of processes's details file pleaes: ";
	string FilePath;
	cin >> FilePath;
	ifstream fin(FilePath);
	if (!fin.is_open()) {
		cout << "Sorry, unable to open file:(\n";
		return 1;
	}

	vector<Process> p;
	int q;
	fin >> q;
	string id;
	int arrival, burst;
	while (fin >> id >> arrival >> burst) {
		p.push_back({id,arrival,burst,0,0,0,0});
	}
	fin.close();

	sort(p.begin(), p.end(), [](const Process& a, const Process& b) {
		return a.AT < b.AT;
	});

    int choic;

	do {
		string Menu = "1.FCFS Algorithm.\n2.SRT Algorithm.\n3.Round Robin Algorithm.\n4.Exit.\n";
		cout << Menu;
		cout << "Choose the number of the algorithm:";
		
		cin >> choic;
		switch (choic)
		{
		case 1:
			FCFS(p);
			break;
		case 2:
			SRT(p);
			break;
		case 3:
			RR(p, q);
			break;
		case 4:
			exit(0);
		default:
			break;
		}
	} while (choic != 4);
	
	return 0;
}
