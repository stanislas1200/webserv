#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;

string get_query_string() {
    char* query_string = getenv("QUERY_STRING");
    return query_string ? query_string : "";
}

string get_param_value(const string& query_string, const string& param_name) {
    size_t start = query_string.find(param_name + "=");
    if(start == string::npos) {
        return "";
    }
    start += param_name.size() + 1;
    size_t end = query_string.find("&", start);
    if(end == string::npos) {
        end = query_string.size();
    }
    return query_string.substr(start, end - start);
}

int main() {
    string query_string = get_query_string();
    string filename = get_param_value(query_string, "name");

    if(!filename.empty()) {
        ifstream file("./upload/" + filename, ios::binary);

        if(file) {
            cout << "Content-Type: application/octet-stream\r\n\r\n";

            char buffer[512];
            while(file.read(buffer, sizeof(buffer))) {
                cout.write(buffer, file.gcount());
            }
        } else {
            cout << "Content-Type: text/html\r\n\r\n";
            cout << "<br>Exception : Unable to open file";
        }
    }

    return 0;
}