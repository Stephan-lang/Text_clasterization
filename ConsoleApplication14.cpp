#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <filesystem>
#include <experimental/filesystem>
#include <sstream>

using namespace std;
using namespace std::experimental::filesystem::v1;

struct Message
{
	int day;
	int hour;
	int min;
	string name;
	vector<string> text;
};

vector <Message> read(string path)
{
	vector<Message>messages;
	for (const auto & entry : directory_iterator(path))
	{
		fstream Logfile(entry.path());
		if (!Logfile.is_open())
		{
			cout << "some file is creating error" << endl;
			continue;
		};
		bool isHeader = true;
		Message mes;
		string str;
		while (getline(Logfile, str))
		{
			if (isHeader) {
				stringstream stream = stringstream(str);
				stream >> mes.day;
				stream >> mes.hour;
				stream.ignore(1);
				stream >> mes.min;
				stream.ignore(1);
				getline(stream, mes.name, ':');
				stream.ignore(1);
				isHeader = false;
			}
			else {
				if (str.empty())
				{
					if (mes.text.size() > 1 || mes.text[0] != "yes" && mes.text[0] != "no" && mes.text[0] != "hello")
						messages.push_back(mes);
					isHeader = true;
					mes.text.clear();
				}
				else
					mes.text.push_back(str);
			}
		}
		Logfile.close();
	}
	return messages;
}

map<string, vector<Message>> clusterise(vector<Message> log)
{
	map<string, vector<Message>> catalog, sortedCatalog;
	for (Message mes : log)
		catalog[mes.name].push_back(mes);
	for (pair<string, vector<Message>> entry : catalog) {
		for (int i = 0; i < entry.second.size(); i++) {
			for (int j = 0; j < entry.second.size() - 1 - i; j++) {
				if ((entry.second[j].day > entry.second[j + 1].day) || (entry.second[j].day == entry.second[j + 1].day &&
					entry.second[j].hour > entry.second[j + 1].hour) || (entry.second[j].day == entry.second[j + 1].day &&
						entry.second[j].hour == entry.second[j + 1].hour && entry.second[j].min > entry.second[j + 1].min)) {
					Message buf = entry.second[j];
					entry.second[j] = entry.second[j + 1];
					entry.second[j + 1] = buf;
				}
			}
		}
		sortedCatalog.emplace(string(entry.first), vector<Message>(entry.second));
	}
	return sortedCatalog;
}

void createCatalog(map<string, vector<Message>> catalog)
{
	fstream file;
	cout << "enter path" << endl;
	string path;
	cin >> path;
	for (pair<string, vector<Message>> entry : catalog)
	{
		file.open(path + "/" + entry.first + ".txt", ios::out);
		for (Message mes : entry.second)
		{
			file << mes.day;
			file << " ";
			file << mes.hour;
			file << ":";
			file << mes.min;
			file << " ";
			file << mes.name;
			file << ":" << endl;
			for (string str : mes.text)
				file << str;
			file << endl << endl;
		}
		file.close();
	}
}

map<int, map<string, bool>> analyzePresence(string path, vector<string> users) {
	map<int, map<string, bool>> presenceTable;
	for (const auto & entry : directory_iterator(path)) {
		fstream usersList(entry.path());
		string filename = entry.path().filename().string();
		int date = stoi(filename.substr(0, filename.find_last_of(".")));
		for (string user : users)
			presenceTable[date][user] = false;
		string presentUser;
		while (usersList >> presentUser)
			presenceTable[date][presentUser] = true;
		usersList.close();
	}
	return presenceTable;
}

void createTable(map<int, map<string, bool>> table, vector<string> users) {
	fstream tableFile("PresenceTable.txt", ios::out);
	for (pair<int, map<string, bool>> entry : table)
		tableFile << "\t" << entry.first;
	tableFile << endl;
	for (int i = 0; i < users.size(); i++) {
		tableFile << users[i];
		auto it = table.begin();
		for (int j = 0; j < table.size(); j++, it++)
			tableFile << "\t" << (table[it->first][users[i]] ? "+" : "-");
		tableFile << endl;
	}
	tableFile.close();
}

int main()
{
	map<string, vector<Message>> catalog = clusterise(read("MessageLogs"));
	createCatalog(catalog);
	vector<string> users;
	for (pair<string, vector<Message>> entry : catalog)
		users.push_back(entry.first);
	createTable(analyzePresence("UsersLists", users), users);
}
