#include <crow.h>
#include <pqxx/pqxx>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <iostream>

using namespace std;

// Global Middleware to handle CORS for all routes and preflight requests
struct CORSMiddleware {
    struct context {};
    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        // Handle preflight OPTIONS requests automatically
        if (req.method == "OPTIONS"_method) {
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type");
            res.code = 204;
            res.end(); 
        }
    }
    void after_handle(crow::request& req, crow::response& res, context& ctx) {
        // Add CORS headers to every response (Success or Error)
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");

    }
};

int countWords(string s) {
    stringstream ss(s);
    string word;
    int count = 0;
    while (ss >> word) count++;
    return count;
}

int main() {
    // Use the CORSMiddleware in the App definition
    crow::App<CORSMiddleware> app;

    const char* db_url_env = getenv("DATABASE_URL");
    string CONN_STR = db_url_env ? db_url_env : "";
    
    char* port_env = getenv("PORT");
    uint16_t port = static_cast<uint16_t>(port_env ? stoi(port_env) : 10000);

    cout << "[INIT] Server starting on port: " << port << endl;
    if (CONN_STR.empty()) {
        cerr << "[CRITICAL] DATABASE_URL is not set!" << endl;
    }

    CROW_ROUTE(app, "/add").methods("POST"_method)([&](const crow::request& req) {
        auto body = crow::json::load(req.body);
        cout << "[INFO] /add endpoint hit" << endl;
        if (!body) return crow::response(400, "Invalid JSON");

        string user = body["username"].s();
        string text = body["sentence"].s();
        cout<<"[INFO] Received data from user: " << user << endl;
        try {
            pqxx::connection C(CONN_STR);
            pqxx::work W(C);
            int words = countWords(text);
            
            // Use exec_params to prevent SQL Injection
            W.exec_params("INSERT INTO user_word_count_history (username, word_count) VALUES ($1, $2)", 
                         user, words);
            W.commit();
            cout << "[INFO] Data inserted successfully for user: " << user << endl;
            return crow::response(200, "Success");
        } catch (const std::exception &e) {
            cerr << "[DB ERROR] " << e.what() << endl;
            return crow::response(500, string("Database Error: ") + e.what());
        }
    });

    CROW_ROUTE(app, "/history/<string>")([&](string user) {
        std::vector<crow::json::wvalue> history_vec;
        try {
            pqxx::connection C(CONN_STR);
            pqxx::nontransaction N(C);
            // Use exec_params for safe querying
            pqxx::result R = N.exec_params("SELECT word_count FROM user_word_count_history WHERE username = $1 ORDER BY id DESC", user);
            
            for (auto row : R) {
                history_vec.push_back(row[0].as<int>());
            }
        } catch (const std::exception &e) {
            cerr << "[DB ERROR] " << e.what() << endl;
            return crow::response(500, e.what());
        }

        crow::json::wvalue json_res;
        json_res["history"] = std::move(history_vec);
        return crow::response(200, json_res);
    });

    app.port(port).multithreaded().run();
}