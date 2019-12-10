#include "./time_evaluation.h"

double timeEvaluate(string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2)
{
	double time_diff = duration_cast<nanoseconds>(t2 - t1).count();
	cout << "\n -------------------------------------------------------------------- \n";
	// cout << "\nTime Evaluation \n";
	cout << task_name << " : " << time_diff / 1000000.0 << " ms";
	cout << "\n -------------------------------------------------------------------- \n";

	return time_diff / 1000000.0;
}

void trackTaskPerformance(TRACK_LIST &time_track_list, string task_name, high_resolution_clock::time_point t1, high_resolution_clock::time_point t2)
{
	double task_time_diff = timeEvaluate(task_name, t1, t2);
	time_track_list.push_back(make_pair(task_name, to_string(task_time_diff)));
}

int computeTimeEvaluation()
{
	std::ifstream data("./data/report_maliciousParty_500K_100K_noDebug.csv");
	if (!data.is_open())
	{
		std::exit(EXIT_FAILURE);
	}

	int i = 0;
	std::string str;
	std::getline(data, str); // skip the first line

	int verify_status_1 = 0;
	int verify_status_0 = 0;

	while (!data.eof())
	{
		getline(data, str);
		if (str.empty())
		{
			continue;
		}
		string id;
		int verify;
		istringstream iss(str);
		getline(iss, id, ',');
		iss >> verify;

		if (verify == 0)
		{
			verify_status_0++;
		}

		// string id_domain = id + " " + to_string(verify);
		// cout << id_domain << endl;
		i++;
	}

	verify_status_1 = i - verify_status_0;

	cout << "Total number of pass verification iteration: " << verify_status_1 << "/" << i << endl;
	cout << "Total number of fail verification iteration: " << verify_status_0 << "/" << i << endl;

	return -1;
}

void storeTimeEvaluation(int argc, char **argv, TRACK_LIST &time_track_list, bool verify_status)
{

	if (argc > 1)
	{
		fstream fout;
		if (strcmp(argv[8], "1") == 0)
		{
			fout.open("./data/report_maliciousParty_500K_100K_noDebug.csv", ios::out | ios::trunc);
			fout << "Iteration, Verification Status";
			for (auto itr = time_track_list.begin(); itr != time_track_list.end(); itr++)
			{
				string column = itr->first;
				fout << ", " << column;
			}
			fout << "\n";
		}
		else
		{
			fout.open("./data/report_maliciousParty_500K_100K_noDebug.csv", ios::out | ios::app);
		}

		// Insert the data to file
		fout << argv[4] << ", " << verify_status;
		for (auto itr = time_track_list.begin(); itr != time_track_list.end(); itr++)
		{
			string time_diff = itr->second;
			fout << ", " << time_diff;
		}
		fout << "\n";
		fout.close();
	}
}