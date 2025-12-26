#include "crow.h"
#include <pqxx/pqxx>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <iostream>

using namespace std;

// Helper to ensure all responses have CORS headers to prevent "(failed)" in browser
crow::response cors_response(int status, string body = "") {
    crow::response res(status, body);
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
    return res;
}

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

    // Initial Debug Log
    cout << "[INIT] Server starting on port: " << port << endl;
    if (CONN_STR.empty()) {
        cerr << "[CRITICAL] DATABASE_URL is not set!" << endl;
    } else {
        cout << "[INIT] Database URL found (Connection will be attempted on request)" << endl;
    }

    CROW_ROUTE(app, "/add").methods("POST"_method, "OPTIONS"_method)([&](const crow::request& req) {
        if (req.method == "OPTIONS"_method) return cors_response(204);
        
        cout << "[DEBUG] Received /add request" << endl;
        
        auto body = crow::json::load(req.body);
        if (!body) {
            cerr << "[ERROR] Failed to parse JSON body" << endl;
            return cors_response(400, "Invalid JSON");
        }

        string user = body["username"].s();
        string text = body["sentence"].s();
        cout << "[DEBUG] User: " << user << " | Sentence length: " << text.length() << endl;

        try {
            cout << "[DB] Connecting to Supabase..." << endl;
            pqxx::connection C(CONN_STR);
            pqxx::work W(C);
            int words = countWords(text);
            
            W.exec("INSERT INTO user_word_count_history (username, word_count) VALUES (" + 
                   W.quote(user) + ", " + to_string(words) + ")");
            W.commit();
            cout << "[DB] Successfully inserted record for " << user << endl;
            return cors_response(200, "Success");
        } catch (const std::exception &e) {
            cerr << "[DB ERROR] " << e.what() << endl;
            // Returning 500 with CORS header so the browser shows the error instead of "(failed)"
            return cors_response(500, string("Database Error: ") + e.what());
        }
    });

    CROW_ROUTE(app, "/history/<string>")([&](string user) {
        cout << "[DEBUG] Fetching history for user: " << user << endl;
        std::vector<crow::json::wvalue> history_vec;

        try {
            pqxx::connection C(CONN_STR);
            pqxx::nontransaction N(C);
            pqxx::result R = N.exec("SELECT word_count FROM user_word_count_history WHERE username = " + N.quote(user) + " ORDER BY id DESC");
            
            for (auto row : R) {
                history_vec.push_back(row[0].as<int>());
            }
            cout << "[DB] Fetched " << history_vec.size() << " records" << endl;
        } catch (const std::exception &e) {
            cerr << "[DB ERROR] " << e.what() << endl;
            return cors_response(500, e.what());
        }

        crow::json::wvalue json_res;
        json_res["history"] = std::move(history_vec);
        auto res = cors_response(200, json_res.dump());
        res.add_header("Content-Type", "application/json");
        return res;
    });

    app.port(port).multithreaded().run();
}