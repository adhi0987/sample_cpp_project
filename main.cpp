#include "crow.h"
#include <pqxx/pqxx>
#include <sstream>
#include <cstdlib>
#include <vector>

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

    // 1. Get Environment Variables
    const char* db_url_env = getenv("DATABASE_URL");
    string CONN_STR = db_url_env ? db_url_env : "";
    
    char* port_env = getenv("PORT");
    uint16_t port = static_cast<uint16_t>(port_env ? stoi(port_env) : 10000);

    // 2. ROUTE: Add new entry
    CROW_ROUTE(app, "/add").methods("POST"_method, "OPTIONS"_method)([&](const crow::request& req) {
        // Handle CORS Pre-flight for browsers
        if (req.method == "OPTIONS"_method) {
            crow::response res(204);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type");
            return res;
        }
        
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        string user = body["username"].s();
        string text = body["sentence"].s();

        try {
            pqxx::connection C(CONN_STR);
            pqxx::work W(C);
            int words = countWords(text);
            
            // Fixed: Used 'exec' instead of deprecated 'exec0'
            W.exec("INSERT INTO user_word_count_history (username, word_count) VALUES (" + 
                   W.quote(user) + ", " + to_string(words) + ")");
            W.commit();
        } catch (const std::exception &e) {
            cerr << "DB Error: " << e.what() << endl;
            return crow::response(500, "Database Error");
        }

        crow::response res(200, "Success");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // 3. ROUTE: Get user history
    CROW_ROUTE(app, "/history/<string>")([&](string user) {
        crow::json::wvalue json_res;
        std::vector<crow::json::wvalue> history_vec; // Fixed: Use std::vector instead of wlist

        try {
            pqxx::connection C(CONN_STR);
            pqxx::nontransaction N(C);
            pqxx::result R = N.exec("SELECT word_count FROM user_word_count_history WHERE username = " + N.quote(user) + " ORDER BY id DESC");
            
            for (auto row : R) {
                history_vec.push_back(row[0].as<int>());
            }
        } catch (const std::exception &e) {
            cerr << "DB Error: " << e.what() << endl;
            return crow::response(500, "Database Error");
        }

        json_res["history"] = std::move(history_vec);
        
        crow::response res;
        res.body = json_res.dump();
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Content-Type", "application/json");
        return res;
    });

    app.port(port).multithreaded().run();
}