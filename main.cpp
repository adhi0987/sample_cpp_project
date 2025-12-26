#include "crow.h"
#include <pqxx/pqxx>
#include <sstream>
#include <cstdlib>

using namespace std;

// Function to count words
int countWords(string s) {
    stringstream ss(s);
    string word;
    int count = 0;
    while (ss >> word) count++;
    return count;
}

int main() {
    crow::SimpleApp app;

    // 1. Get Environment Variables from the Cloud
    const char* db_url_env = getenv("DATABASE_URL");
    string CONN_STR = db_url_env ? db_url_env : "your_local_db_url_here";
    
    char* port_env = getenv("PORT");
    uint16_t port = static_cast<uint16_t>(port_env ? stoi(port_env) : 8080);

    // 2. ROUTE: Add new entry
    CROW_ROUTE(app, "/add").methods("POST"_method, "OPTIONS"_method)([&](const crow::request& req) {
        // Handle CORS Pre-flight
        if (req.method == "OPTIONS"_method) return crow::response(204);
        
        auto body = crow::json::load(req.body);
        string user = body["username"].s();
        string text = body["sentence"].s();

        pqxx::connection C(CONN_STR);
        pqxx::work W(C);
        int words = countWords(text);
        W.exec0("INSERT INTO user_word_count_history (username, word_count) VALUES (" + 
                W.quote(user) + ", " + to_string(words) + ")");
        W.commit();

        crow::response res(200);
        res.add_header("Access-Control-Allow-Origin", "*"); // Allows your website to connect
        return res;
    });

    // 3. ROUTE: Get user history
    CROW_ROUTE(app, "/history/<string>")([&](string user) {
        pqxx::connection C(CONN_STR);
        pqxx::nontransaction N(C);
        pqxx::result R = N.exec("SELECT word_count FROM user_word_count_history WHERE username = " + N.quote(user) + " ORDER BY id DESC");
        
        crow::json::wlist list;
        for (auto row : R) list.push_back(row[0].as<int>());

        crow::json::wvalue json_res;
        json_res["history"] = std::move(list);
        
        crow::response res;
        res.body = json_res.dump();
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    app.port(port).multithreaded().run();
}