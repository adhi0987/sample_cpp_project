#include "crow.h"
#include <pqxx/pqxx>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <iostream>

using namespace std;

// Updated countWords logic for better accuracy
int countWords(string s) {
    stringstream ss(s);
    string word;
    int count = 0;
    while (ss >> word) count++;
    return count;
}

int main() {
    crow::SimpleApp app;

    const char* db_url_env = getenv("DATABASE_URL");
    string CONN_STR = db_url_env ? db_url_env : "";
    
    char* port_env = getenv("PORT");
    uint16_t port = static_cast<uint16_t>(port_env ? stoi(port_env) : 10000);

    // Initial Debug Logs
    cout << "[PROD-INIT] Starting server on port " << port << endl;
    if (CONN_STR.empty()) cerr << "[CRITICAL] DATABASE_URL is missing!" << endl;

    // GLOBAL CORS MIDDLEWARE
    // This ensures every response from the server has the required headers
    CROW_CATCHALL_ROUTE(app)
    ([](crow::response& res) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
    });

    CROW_ROUTE(app, "/add").methods("POST"_method, "OPTIONS"_method)([&](const crow::request& req) {
        if (req.method == "OPTIONS"_method) return crow::response(204);
        
        cout << "[DEBUG] Request: /add" << endl;
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        string user = body["username"].s();
        string text = body["sentence"].s();

        try {
            pqxx::connection C(CONN_STR);
            pqxx::work W(C);
            W.exec("INSERT INTO user_word_count_history (username, word_count) VALUES (" + 
                   W.quote(user) + ", " + to_string(countWords(text)) + ")");
            W.commit();
            cout << "[PROD-LOG] Record added for " << user << endl;
            return crow::response(200, "Success");
        } catch (const std::exception &e) {
            cerr << "[PROD-ERR] Database: " << e.what() << endl;
            return crow::response(500, string("DB Error: ") + e.what());
        }
    });

    CROW_ROUTE(app, "/history/<string>")([&](string user) {
        cout << "[DEBUG] Request: /history for " << user << endl;
        try {
            pqxx::connection C(CONN_STR);
            pqxx::nontransaction N(C);
            pqxx::result R = N.exec("SELECT word_count FROM user_word_count_history WHERE username = " + N.quote(user) + " ORDER BY id DESC");
            
            vector<crow::json::wvalue> history_vec;
            for (auto row : R) history_vec.push_back(row[0].as<int>());

            crow::json::wvalue json_res;
            json_res["history"] = std::move(history_vec);
            return crow::response(json_res);
        } catch (const std::exception &e) {
            cerr << "[PROD-ERR] History: " << e.what() << endl;
            return crow::response(500, e.what());
        }
    });

    app.port(port).multithreaded().run();
}