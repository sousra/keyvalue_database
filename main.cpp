#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

/* 
* СУБД типа ключ-значение. Данные хранятся в файле, который указан в argv[1]
* 
* Поддерживаемые команды:
* KEYS *key*		-- Поиск по шаблону. Возвращает список ключей или "null", если подходящих нет.
					-- Шаблоны: *, key*, *key* (* - любое количество, в том числе 0, любых символов)
* SET key value		-- Создание пары ключ-значение или изменение значения, если ключ уже существует
* GET key			-- Получение значения по ключу. Возвращает строку со значением или строку "null", если ключа не существует
* DEL key			-- Удаление пары ключ-значение по ключу. Возвращает 1, если пара удалена и 0, если пара не найдена
* FLUSHALL			-- Удаляет все пары ключ-значение из базы данных
* SAVE				-- Сохраняет измененные в программе данные в файл
* EXIT				-- Завершает выполнение СУБД
*/
struct Pair {
	Pair() {};
	Pair(const std::string& key, const std::string& value)
		: key(key), value(value) {};
	std::string key;
	std::string value;
};

std::string get_keys(const std::vector<Pair>& data, std::string pattern);
// Создает пару ключ-значение по ключу или перезаписывает значение, если ключ уже существовал
void set_pair(std::vector<Pair>& data, const std::string& key, const std::string& value);
// Возвращает строку, которая содержит значение по ключу или строку "null"
std::string get_value(const std::vector<Pair>& data, const std::string& key);
// Удаляет пару ключ-значение по ключу. Возвращает true, если пара удалена и false, если пара не найдена
bool del(std::vector<Pair>& data, const std::string& key);
// Удаляет все пары ключ-значение из базы данных
void flushall(std::vector<Pair>& data);
// Сохраняет измененные в программе данные в файл
void save_data(const std::vector<Pair>& data, const std::string& path);
// Синхронизирует текущие данные программы с данными из файла
void recover_data(std::vector<Pair>& data, const std::string& path);

std::string str_to_lower(std::string str);

int main(int argc, char ** argv)
{
	std::vector<Pair> data{};
	std::string data_file_path = argv[1];
	recover_data(data, data_file_path);
	std::string query{}, command{}, arg1{}, arg2{};
	unsigned long long int ticks = 0;
	while (true) {
		query.clear(); command.clear(); arg1.clear(); arg2.clear();
		std::getline(std::cin, query);
		std::stringstream ss;
		ss << query;
		ss >> command >> arg1 >> arg2;
		command = str_to_lower(command);
		if (command == "keys" && !arg1.empty()) {
			std::cout << get_keys(data, arg1) << std::endl;
		}
		else if (command == "set" && !arg1.empty() && !arg2.empty()) {
			set_pair(data, arg1, arg2);
			std::cout << "OK" << std::endl;
		}
		else if (command == "get" && !arg1.empty()) {
			std::cout << get_value(data, arg1) << std::endl;
		}
		else if (command == "del" && !arg1.empty()) {
			if (del(data, arg1)) { std::cout << 1 << std::endl; }
			else { std::cout << 0 << std::endl; };
		}
		else if (command == "flushall") {
			flushall(data);
			std::cout << "OK" << std::endl;
		}
		else if (command == "exit") {
			break;
		}
		else if (command == "save") {
			save_data(data, data_file_path);
			std::cout << "OK" << std::endl;
		}
		else if (!query.empty()) {
			std::cout << "Incorrect query" << std::endl;
		}
		if (ticks++ % 5 == 0) {
			save_data(data, data_file_path);
		}
	}
	return 0;
}

std::string get_keys(const std::vector<Pair>& data, std::string pattern) {
	std::string res{};
	if (pattern == "*") {
		for (int i = 0; i < data.size(); ++i) {
			res.push_back(char(int('0') + i + 1));
			res += +") " + data[i].key;
			res.push_back('\n');
		}
	}
	else if (pattern.front() == '*' && pattern.back() == '*') {
		pattern = pattern.substr(1, pattern.size() - 2);
		for (int i = 0; i < data.size(); ++i) {
			std::string cur_key = data[i].key;
			auto it = std::search(cur_key.begin(), cur_key.end(), pattern.begin(), pattern.end());
			if (it != cur_key.end()) {
				res.push_back(char(int('0') + i + 1));
				res += +") " + data[i].key;
				res.push_back('\n');
			}
		}
	}
	else if (pattern.back() == '*') {
		pattern.pop_back();
		for (int i = 0; i < data.size(); ++i) {
			std::string cur_key = data[i].key;
			auto it = std::search(cur_key.begin(), cur_key.end(), pattern.begin(), pattern.end());
			if (it == cur_key.begin()) {
				res.push_back(char(int('0') + i + 1));
				res += +") " + data[i].key;
				res.push_back('\n');
			}
		}
	}
	if (res.empty()) { res = "null"; }
	return res;
}

void set_pair(std::vector<Pair>& data, const std::string& key, const std::string& value) {
	for (int i = 0; i < data.size(); ++i) {
		if (data[i].key == key) {
			data[i].value = value;
			return;
		}
	}
	data.push_back(Pair(key, value));
}

std::string get_value(const std::vector<Pair>& data, const std::string& key) {
	std::string res = "null";
	for (int i = 0; i < data.size(); ++i) {
		if (data[i].key == key) {
			res = data[i].value;
		}
	}
	return res;
}

bool del(std::vector<Pair>& data, const std::string& key) {
	for (auto it = data.begin(); it != data.end(); ++it) {
		if (it->key == key) {
			data.erase(it);
			return true;
		}
	}
	return false;
}

void flushall(std::vector<Pair>& data) {
	data.clear();
}

void save_data(const std::vector<Pair>& data, const std::string& path) {
	std::ofstream file(path, std::ios::trunc);
	if (!file) {
		std::cerr << "Save data error!\n";
	}
	for (int i = 0; i < data.size(); ++i) {
		file << data[i].key << ' ' << data[i].value << '\n';
	}
	file.close();
}

void recover_data(std::vector<Pair>& data, const std::string& path) {
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Recover data error!\n";
	}
	std::string key, value;
	while (file >> key) {
		file >> value;
		set_pair(data, key, value);
	}
}

std::string str_to_lower(std::string str) {
	for (int i = 0; i < str.size(); ++i) {
		str[i] = tolower(str[i]);
	}
	return str;
}